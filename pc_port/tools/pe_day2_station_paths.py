#!/usr/bin/env python3
"""Closed M0042I station predicates through original handlers; no async skips."""
import hashlib,json,struct
from pe_day2_route_audit import ROOT,SCRIPTS,parse_field_table,extract_script_from_package,decode_script,find_disc,read_form1
from pe_battle_hud_oracle import execute
HANDLERS={0:0x80017294,5:0x8001731C,9:0x80012850,10:0x800173F4,0x1F:0x800177AC}
BASE=0x801C5224
COUNTERS=(0,1,2,0x7FFFFFFF,0x80000000,0xFFFFFFFF)
STORIES=(0,0x87,0x88,0x89,0x98,0xA5,0xA6,0xB7,0xB8,0xB9,0x7FFFFFFF,0x80000000,0xFFFFFFFF)
def load():
 ex=(ROOT/'build/disc1.candidate.exe').read_bytes();assert hashlib.sha1(ex).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
 rec=next(r for r in parse_field_table(ex) if r['map_id']==42)
 c0=rec['meta']&255;c1=rec['meta']>>8&4095;c2=rec['meta']>>20
 package=read_form1(find_disc(ROOT),1013+rec['start'],c0+c1+c2)
 off,raw=extract_script_from_package(package,rec['meta']);script=decode_script(raw)
 assert script['sha256']==SCRIPTS[42] and BASE==0x8018EFE8+off-(c0+c1)*2048
 return ex,raw,script

def run(ex,raw,module,start,stops,story=0x88,counter=1,flags44=0,flags0=0,choice=0,slot=22,payload=0):
 r=bytearray(0x200000);r[0x10000:0x10000+len(ex)-0x800]=ex[0x800:]
 r[BASE&0x1FFFFF:(BASE&0x1FFFFF)+len(raw)]=raw
 def sw(a,v):struct.pack_into('<I',r,a&0x1FFFFF,v&0xFFFFFFFF)
 def lw(a):return struct.unpack_from('<I',r,a&0x1FFFFF)[0]
 sw(0x8009D2F0,0x80152000);sw(0x8015209C,BASE+module['start'])
 sw(0x800A7918,story);sw(0x800A78A0,flags44);sw(0x800A77F0,flags0)
 sw(0x800B6A80+slot*4,counter);sw(0x80153014,choice)
 sw(0x8009D300,0x80152400);sw(0x80152414,payload)
 commands={BASE+c['offset']:c for c in module['commands']};pc=start;trace=[]
 for _ in range(40):
  if pc in stops:return dict(stop=f'{pc:08X}',story=lw(0x800A7918),counter=lw(0x800B6A80+slot*4),flags44=lw(0x800A78A0),flags0=lw(0x800A77F0),trace=trace)
  c=commands[pc];assert c['opcode'] in HANDLERS,f'unknown opcode {c["opcode"]:X} at{pc:X}'
  sw(0x8009CE00,pc+8+4*c['argc'])
  for i,(mode,arg) in enumerate(zip(c['modes'],c['args'])):
   if mode==0:address=pc+8+i*4
   elif mode==1:address=0x80153000+arg*4
   elif mode==2:address=0x800A77F0+arg*4
   elif mode==3:address=0x8009DF70+arg*4
   elif mode==4:address=0x800B6A80+arg*4
   else:raise AssertionError(f'unbound mode{mode}')
   sw(0x80150000+i*4,address)
  regs=execute(r,HANDLERS[c['opcode']],(0x80150000,));assert regs[2]==1
  trace.append(f'{pc:08X}');pc=lw(0x8009CE00)
 raise AssertionError('closed region failed to terminate')

def main():
 ex,raw,script=load();module=script['modules'][2];assert BASE+module['start']==0x801C67B0
 results=[]
 producer=script['modules'][1]
 assert BASE+producer['start']==0x801C5FEC
 for story in STORIES:
  signed=story if story<0x80000000 else story-0x100000000
  got=run(ex,raw,producer,0x801C625C,{0x801C6284,0x801C6370},story=story)
  assert got['stop']==('801C6284' if signed<0xA6 else '801C6370')
  results.append(dict(region='producer_story_gate',input=[story],result=got))
  for counter in COUNTERS:
   got=run(ex,raw,producer,0x801C632C,{0x801C6654},story=story,counter=counter)
   assert got['counter']==(1 if story==0x88 else counter)
   results.append(dict(region='producer_counter',input=[story,counter],result=got))
 for story in STORIES:
  signed=story if story<0x80000000 else story-0x100000000
  expected=0x801C68BC if story==0x88 else 0x801C692C if signed<0xA6 else 0x801C699C if story==0xB8 else 0x801C69E4
  got=run(ex,raw,module,0x801C6894,{0x801C68BC,0x801C692C,0x801C699C,0x801C69E4},story=story)
  assert got['stop']==f'{expected:08X}';results.append(dict(region='placement',input=[story],result=got))
 for story in STORIES:
  for counter in COUNTERS:
   for flags44 in (0,8,0xFFFFFFF7,0xFFFFFFFF):
    for flags0 in range(8):
     expected=0x801C7114 if story!=0x88 else 0x801C6A40 if counter==0 else 0x801C7114 if flags44&8 else 0x801C70BC if flags0&2 else 0x801C6B1C if flags0&4 else 0x801C6BB4
     got=run(ex,raw,module,0x801C69F0,{0x801C7114,0x801C6A40,0x801C70BC,0x801C6B1C,0x801C6BB4},story,counter,flags44,flags0)
     assert got['stop']==f'{expected:08X}'
     assert got['counter']==((counter-1)&0xFFFFFFFF if story==0x88 and counter else counter)
     assert (got['story'],got['flags44'],got['flags0'])==(story,flags44,flags0)
     results.append(dict(region='entry_gate',input=[story,counter,flags44,flags0],result=got))
 for choice in COUNTERS:
  for flags0 in range(8):
   got=run(ex,raw,module,0x801C6B40,{0x801C6BB4,0x801C70BC},flags0=flags0,choice=choice)
   assert got['stop']==('801C6BB4' if choice==0 else '801C70BC')
   assert got['flags0']==(flags0 if choice==0 else flags0|2)
   results.append(dict(region='choice_gate',input=[choice,flags0],result=got))
 waits=((0x801C6C50,20,0x801C6C78,0x801C6CB8),(0x801C6CF4,19,0x801C6D1C,0x801C6D5C),(0x801C6D88,19,0x801C6DB0,0x801C6DF0),(0x801C6E30,18,0x801C6E58,0x801C6E98),(0x801C6EC4,18,0x801C6EEC,0x801C6F2C),(0x801C6F40,20,0x801C6F68,0x801C6FA8),(0x801C7004,18,0x801C702C,0x801C706C))
 for start,slot,wait,end in waits:
  for counter in COUNTERS:
   got=run(ex,raw,module,start,{wait,end},counter=counter,slot=slot)
   assert got['stop']==f'{end if counter else wait:08X}' and got['counter']==((counter-1)&0xFFFFFFFF if counter else 0)
   results.append(dict(region='counter_wait',input=[start,slot,counter],result=got))
 for flags44 in (0,1,8,9,0xFFFFFFF7,0xFFFFFFFF):
  got=run(ex,raw,module,0x801C70BC,{0x801C70F4},flags44=flags44)
  assert got['story']==0x90 and got['flags44']==flags44|8
  results.append(dict(region='finalize',input=[flags44],result=got))
 producers={slot:[] for slot in (18,19,20,22)}
 for m in script['modules']:
  for c in m['commands']:
   if c['opcode']==10 and c['modes']==[4,0] and c['args'][0] in producers:
    assert c['args'][1]==1
    producers[c['args'][0]].append(BASE+c['offset'])
 assert producers=={18:[0x801C7B04,0x801C7B98,0x801C7BFC,0x801C7C60],19:[0x801C7600,0x801C7670],20:[0x801C55B4,0x801C5684],22:[0x801C6354]}
 commands={BASE+c['offset']:c for c in module['commands']}
 assert commands[0x801C70F4]['opcode']==0x85 and commands[0x801C7100]['opcode']==0x9C
 assert commands[0x801C7108]['opcode']==0x31 and commands[0x801C7108]['args']==[0xA80040C8]
 out=dict(script_sha256=script['sha256'],module=2,producers=producers,scope='closed predicates and assignments only; asynchronous commands are explicit boundaries',cases=results)
 path=ROOT/'local/live/day2-station-paths.json';path.write_text(json.dumps(out,indent=2)+'\n')
 print(f'PASS {len(results)} original station predicate/assignment paths; {path.relative_to(ROOT)}')
if __name__=='__main__':main()
