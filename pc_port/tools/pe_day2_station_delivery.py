#!/usr/bin/env python3
"""Original station handler installation/send/drain/poll in an actor-list fixture."""
import json,struct
from pe_day2_station_paths import ROOT,BASE,load
from pe_battle_hud_oracle import execute
ENTRIES={0:0x801C5494,2:0x801C711C,3:0x801C7594,4:0x801C7A84,5:0x801C7F78}
def main():
 ex,raw,script=load();cases=[]
 messages=[(m['index'],BASE+c['offset'],c['args']) for m in script['modules'] for c in m['commands'] if c['opcode']==0x1C and c['modes']==[0,0,0]]
 assert len(messages)==27
 for variant in range(32):
  r=bytearray(0x200000);r[0x10000:0x10000+len(ex)-0x800]=ex[0x800:]
  r[BASE&0x1FFFFF:(BASE&0x1FFFFF)+len(raw)]=raw
  def sw(a,v):struct.pack_into('<I',r,a&0x1FFFFF,v&0xFFFFFFFF)
  def sh(a,v):struct.pack_into('<H',r,a&0x1FFFFF,v&0xFFFF)
  def sb(a,v):r[a&0x1FFFFF]=v&255
  def lw(a):return struct.unpack_from('<I',r,a&0x1FFFFF)[0]
  def lh(a):return struct.unpack_from('<H',r,a&0x1FFFFF)[0]
  actors=[0x80150000+i*0x200 for i in range(8)];installed=[]
  for i,actor in enumerate(actors):
   r[actor&0x1FFFFF:(actor&0x1FFFFF)+0x200]=bytes(0x200)
   sw(actor+4,actors[i+1] if i<7 else 0);sb(actor+12,i);sb(actor+13,0);sh(actor+0x24,0x200+i)
   sw(actor+0x9C,BASE+script['modules'][i]['start']);sw(0x8009D2F0,actor)
   for c in script['modules'][i]['commands']:
    if c['opcode']!=0x14:continue
    assert c['modes']==[0,0]
    for j in range(2):sw(0x80153000+j*4,BASE+c['offset']+8+j*4)
    regs=execute(r,0x80017588,(0x80153000,));assert regs[2]==1
    dest=actor+(0x19C if c['args'][0]==2 else 0x1A0)
    expected=BASE+script['modules'][i]['start']+c['args'][1]*2
    assert lw(dest)==expected
    installed.append(dict(module=i,pc=f'{BASE+c["offset"]:08X}',field=hex(dest-actor),entry=f'{expected:08X}'))
   assert lw(actor+0x19C)==ENTRIES.get(i,0)
  # Explicit variations in recipient identity/enable state; not actor construction.
  if variant&1:sb(actors[0]+13,1)
  if variant&2:sw(actors[3]+0x19C,0)
  if variant&4:sb(actors[7]+12,4);sw(actors[7]+0x19C,ENTRIES[4])
  if variant&8:sb(actors[4]+13,1)
  sw(0x8009D20C,actors[0]);sb(0x8009CDB4,0);sh(0x8009D308,0xFFFE)
  pool=[0x80160000+i*0x40 for i in range(64)]
  for i,node in enumerate(pool):
   r[node&0x1FFFFF:(node&0x1FFFFF)+0x40]=bytes([0xA5])*0x40
   sw(node+0x24,pool[i+1] if i+1<len(pool) else 0)
  sw(0x8009CDFC,pool[0]);expected=[[] for _ in actors];allocated=0
  selected=messages if variant&16 else [m for m in messages if m[0]==2]
  for sender,pc,args in selected:
   sw(0x8009D2F0,actors[sender])
   for i in range(3):sw(0x80153000+i*4,pc+8+i*4)
   regs=execute(r,0x80017764,(0x80153000,));assert regs[2]==1
   for i,actor in enumerate(actors):
    if r[(actor+12)&0x1FFFFF]==args[0] and r[(actor+13)&0x1FFFFF]==args[1] and lw(actor+0x19C):
     expected[i].insert(0,dict(task=pool[allocated],entry=lw(actor+0x19C),sender=0x200+sender,payload=args[2],serial=(0xFFFE+allocated)&0xFFFF));allocated+=1
  queued=bytes(r[0xA3180:0xA3180+len(selected)*12])
  execute(r,0x80065400)
  assert bytes(r[0xA3180:0xA3180+len(selected)*12])==queued
  assert r[0x9CDB4]==0 and lw(0x8009CDFC)==pool[allocated] and lh(0x8009D308)==(0xFFFE+allocated)&0xFFFF
  delivered=[]
  for i,actor in enumerate(actors):
   node=lw(actor+0xA8);previous=0
   for want in expected[i]:
    assert node==want['task'] and lw(node)==want['entry'] and lw(node+4)==0
    assert lh(node+8)==4 and lh(node+10)==want['serial'] and lw(node+12)==want['sender'] and lw(node+16)==1
    assert lw(node+20)==want['payload'] and lw(node+0x28)==previous
    assert lw(node+0x18)==0xA5A5A5A5 and lw(node+0x1C)==0xA5A5A5A5
    sw(0x8009D300,node);sw(0x80153000,0x80153800)
    regs=execute(r,0x800177AC,(0x80153000,));assert regs[2]==1 and lw(0x80153800)==want['payload']
    delivered.append(dict(actor=i,**want));previous=node;node=lw(node+0x24)
   assert node==0
  cases.append(dict(variant=variant,installed=installed,messages=len(selected),delivered=delivered))
 out=dict(script_sha256=script['sha256'],scope='explicit actor type/module/list and free-task-pool fixture; original handler installs/send/drain/poll; no actor construction or scheduler execution',cases=cases)
 path=ROOT/'local/live/day2-station-delivery.json';path.write_text(json.dumps(out,indent=2)+'\n')
 print(f'PASS {len(cases)} original station delivery graphs; {sum(len(c["delivered"]) for c in cases)} delivered tasks; {path.relative_to(ROOT)}')
if __name__=='__main__':main()
