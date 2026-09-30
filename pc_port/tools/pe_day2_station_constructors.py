#!/usr/bin/env python3
"""Original station relocation and constructors through resource boundaries."""
import json,struct
from pe_day2_station_paths import ROOT,BASE,load
from pe_battle_hud_oracle import execute

def main():
 ex,raw,script=load();cases=[]
 for kind in range(8):
  for variant in range(8 if kind==0 else 16):
   r=bytearray(0x200000);r[0x10000:0x10000+len(ex)-0x800]=ex[0x800:]
   r[BASE&0x1FFFFF:(BASE&0x1FFFFF)+len(raw)]=raw
   def sw(a,v):struct.pack_into('<I',r,a&0x1FFFFF,v&0xFFFFFFFF)
   def sh(a,v):struct.pack_into('<H',r,a&0x1FFFFF,v&0xFFFF)
   def lw(a):return struct.unpack_from('<I',r,a&0x1FFFFF)[0]
   def lh(a):return struct.unpack_from('<H',r,a&0x1FFFFF)[0]
   execute(r,0x80012574,(BASE,))
   entries=[lw(BASE+8+i*4) for i in range(8)]
   assert entries==[BASE+m['start'] for m in script['modules']]
   sw(0x800B161C,BASE)
   for i in range(10):sw(0x800B0E70+i*4,0)
   r[0x9D310:0x9DF70]=bytes(0xC60);execute(r,0x8001266C)
   sh(0x8009D308,0xFFFE);sw(0x8009D224,0x1FFFF);sh(0x8009D2A6,0xFFFF)
   actor=0x80150000;parent=0x80151000 if variant&1 else 0;old=0x80151400 if variant&2 else 0
   r[0x150000:0x151800]=bytes(0x1800)
   sw(0x8009D2AC,actor);sw(actor+4,0x80151800)
   sw(0x8009D20C,parent if parent else old)
   if parent:sw(parent+4,old)
   resource=bool(variant&8)
   if resource:
    sw(0x800B0E70+kind*4,0x80153100);sh(0x80153100,3)
    r[0x153102:0x153104]=bytes((2,1))
    for i in range(48):sw(0x800B0E98+kind*192+i*4,0)
   descriptor=0x80153000;r[0x153000:0x153002]=bytes((kind,variant))
   regs=execute(r,0x80035038,(descriptor,parent,1 if variant&4 else 0),stop_at=(0x800362B8,) if resource else ())
   if resource:
    assert regs[17]==actor and regs[4]==(180 if variant&4 else 76)
    assert regs[31]==(0x80035348 if variant&4 else 0x80035378)
   else:assert regs[2]==actor
   assert lw(0x8009D2AC)==0x80151800
   assert lw(actor+4)==old and lw(actor+8)==parent
   assert lw(parent+4 if parent else 0x8009D20C)==actor
   if old:assert lw(old+8)==actor
   assert r[0x15000C:0x15000E]==bytes((kind,variant))
   assert lw(actor+0x9C)==entries[kind] and lw(actor+0x19C)==0 and lw(actor+0x1A0)==0
   task=lw(actor+0xA8);assert task==0x8009D310 and lw(task)==entries[kind]
   assert lh(task+8)==0 and lh(task+10)==0xFFFE and lw(task+16)==1
   assert lw(0x8009CDFC)==task+0x2C and lh(0x8009D308)==0xFFFF
   assert lh(actor+0x24)==0xFFFF and lw(0x8009D224)==0x20000 and lh(0x8009D2A6)==0
   assert lw(actor+0x1AC)==(0x80153100 if resource else 0) and lw(actor+0x98)==(0x10000000 if resource else 0x100000E0)
   installed=[];sw(0x8009D2F0,actor)
   for c in ([] if resource else script['modules'][kind]['commands']):
    if c['opcode']!=0x14:continue
    assert c['modes']==[0,0]
    for i in range(2):sw(0x80153800+i*4,BASE+c['offset']+8+i*4)
    regs=execute(r,0x80017588,(0x80153800,));assert regs[2]==1
    field=0x19C if c['args'][0]==2 else 0x1A0
    expected=entries[kind]+c['args'][1]*2;assert lw(actor+field)==expected
    installed.append(dict(field=hex(field),entry=f'{expected:08X}'))
   cases.append(dict(type=kind,id=variant,parent=parent,old=old,resource=resource,stop=0x800362B8 if resource else 0,entry=f'{entries[kind]:08X}',task=task,installed=installed))
 out=dict(script_sha256=script['sha256'],scope='original relocation, task-pool initialization, types0..7 construction without models, types1..7 through first model allocation boundary; handler installations only after returning constructors; type0 model continuation/preceding scene setup excluded',cases=cases)
 path=ROOT/'local/live/day2-station-constructors.json';path.write_text(json.dumps(out,indent=2)+'\n')
 print(f'PASS {len(cases)} original station relocation/constructor/initial-task paths; {path.relative_to(ROOT)}')
if __name__=='__main__':main()
