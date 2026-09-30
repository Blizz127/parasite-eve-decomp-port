#!/usr/bin/env python3
"""Station mailbox receiver gates and source continuation/send relationships."""
import json,struct
from pe_battle_hud_oracle import execute
from pe_day2_station_paths import ROOT,BASE,load,run,STORIES

def main():
 ex,raw,script=load();cases=[];sequences=[]
 specs=((0,0x801C5494,{5:0x801C54C8,6:0x801C548C,7:0x801C5588,8:0x801C55EC,2:0x801C5658,3:0x801C56BC,254:0x801C5708,255:0x801C5768,0:0x801C57C0},0x801C58D0),
        (3,0x801C7594,{6:0x801C75C8,2:0x801C7638,10:0x801C76A8},0x801C7830),
        (4,0x801C7A84,{6:0x801C7AB8,7:0x801C7B3C,3:0x801C7BD0,4:0x801C7C34},0x801C7C70))
 for index,start,targets,end in specs:
  for payload in list(range(256))+[0x100,0x106,0x7FFFFFFF,0x80000000,0xFFFFFFFF]:
   got=run(ex,raw,script['modules'][index],start,set(targets.values())|{end},payload=payload)
   assert got['stop']==f'{targets.get(payload,end):08X}'
   cases.append(dict(module=index,payload=payload,result=got))
 # Each producer lies after the receiver's asynchronous sequence, with no
 # intervening jump. Inspect its exact commands; never execute past a boundary.
 producer_specs=((0,7,0x801C5588,0x801C55B4,20,[0x2E,0x4E,0x2F,0x30]),
 (0,2,0x801C5658,0x801C5684,20,[0x2E,0x4E,0x2F,0x30]),
 (3,6,0x801C75C8,0x801C7600,19,[2,0x2E,0x4B,0x2E]),
 (3,2,0x801C7638,0x801C7670,19,[0x2E,0x4E,0x2F,0x30,0x2E]),
 (4,6,0x801C7AB8,0x801C7B04,18,[0x2E,0x4B,0x2E,0x4E,0x2F,0x30]),
 (4,7,0x801C7B3C,0x801C7B98,18,[0x5D,0x2E,0xB8,0x44,0x76,0x2E]),
 (4,3,0x801C7BD0,0x801C7BFC,18,[0x2E,0x4E,0x2F,0x30]),
 (4,4,0x801C7C34,0x801C7C60,18,[0x2E,0x4E,0x2F,0x30]))
 for index,payload,start,producer,slot,ops in producer_specs:
  commands={BASE+c['offset']:c for c in script['modules'][index]['commands']};pc=start;chain=[]
  while pc!=producer:
   c=commands[pc];chain.append(dict(pc=f'{pc:08X}',opcode=c['opcode'],modes=c['modes'],args=c['args']))
   pc+=8+c['argc']*4;assert pc<=producer
  assert [c['opcode'] for c in chain]==ops
  c=commands[producer];assert c['opcode']==10 and c['modes']==[4,0] and c['args']==[slot,1]
  got=run(ex,raw,script['modules'][index],producer,{producer+16},slot=slot,counter=0xFFFFFFFF)
  assert got['counter']==1
  sequences.append(dict(module=index,payload=payload,producer=f'{producer:08X}',scratch=slot,commands=chain,assignment=got))
 sends=[]
 wanted={0x801C6BB4:[0,0,5],0x801C6C24:[0,0,2],0x801C6E04:[0,0,3],0x801C706C:[0,0,8],0x801C6C04:[3,0,6],0x801C6D5C:[3,0,2],0x801C6DF0:[4,0,6],0x801C6E98:[4,0,3],0x801C6F2C:[0,0,7],0x801C6FC0:[4,0,4]}
 commands={BASE+c['offset']:c for c in script['modules'][2]['commands']}
 for pc,args in wanted.items():
  c=commands[pc];assert c['opcode']==0x1C and c['modes']==[0,0,0] and c['args']==args
  sends.append(dict(pc=f'{pc:08X}',destination_type=args[0],destination_id=args[1],payload=args[2]))
 send_cases=[]
 for message in sends:
  for sender in (0,1,0xFFFF,0x10000):
   for count in (0,1,27,255):
    r=bytearray(0x200000);r[0x10000:0x10000+len(ex)-0x800]=ex[0x800:]
    def sw(a,v):struct.pack_into('<I',r,a&0x1FFFFF,v)
    sw(0x8009D2F0,0x80152000);struct.pack_into('<H',r,0x152024,sender&0xFFFF)
    for i,v in enumerate((message['destination_type'],message['destination_id'],message['payload'])):
     sw(0x80153000+i*4,0x80153100+i*4);sw(0x80153100+i*4,v)
    r[0x9CDB4]=count;offset=0xA3180+count*12;r[offset:offset+12]=bytes([0xA5])*12
    regs=execute(r,0x80017764,(0x80153000,))
    expected=struct.pack('<HBBII',message['destination_type'],message['destination_id'],message['payload'],0,sender&0xFFFF)
    assert r[offset:offset+12]==expected and r[0x9CDB4]==(count+1)&255 and regs[2]==1
    send_cases.append(dict(pc=message['pc'],sender=sender,count=count,record=expected.hex()))
 assert not any(c['opcode']==0x1C and c['modes']==[0,0,0] and c['args']==[4,0,7] for m in script['modules'] for c in m['commands'])
 # Shared placement branches retain their later-story alternatives.
 for index,start,first,second,special,third,end in ((3,0x801C7454,0x801C747C,0x801C74E0,0xDA,0x801C7544,0x801C7580),(4,0x801C7944,0x801C796C,0x801C79D0,0xB8,0x801C7A34,0x801C7A70)):
  for story in STORIES+(0xD9,0xDA,0xDB):
   signed=story if story<0x80000000 else story-0x100000000
   target=first if story==0x88 else second if signed<0xA6 else third if story==special else end
   got=run(ex,raw,script['modules'][index],start,{first,second,third,end},story=story)
   assert got['stop']==f'{target:08X}';cases.append(dict(module=index,story=story,result=got))
 out=dict(script_sha256=script['sha256'],scope='original receiver predicates; source-only asynchronous continuations; no mailbox delivery or animation completion claim',cases=cases,sequences=sequences,sends=sends,send_cases=send_cases)
 path=ROOT/'local/live/day2-station-mailboxes.json';path.write_text(json.dumps(out,indent=2)+'\n')
 print(f'PASS {len(cases)} original receiver/placement paths, {len(sequences)} signal assignments/source continuations, {len(send_cases)} original sends; {path.relative_to(ROOT)}')
if __name__=='__main__':main()
