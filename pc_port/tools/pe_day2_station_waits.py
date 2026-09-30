#!/usr/bin/env python3
"""Original VM dispatch/binding slices around station animation wait signals."""
import json,struct,sys
from pe_day2_station_paths import ROOT,BASE,load
from pe_battle_hud_oracle import execute
SITES=((0,0x801C55A0,0x801C55AC,0x801C55B4,20),(0,0x801C5670,0x801C567C,0x801C5684,20),(4,0x801C7AF0,0x801C7AFC,0x801C7B04,18),(4,0x801C7BE8,0x801C7BF4,0x801C7BFC,18),(4,0x801C7C4C,0x801C7C58,0x801C7C60,18))
def main():
 ex,raw,script=load();cases=[];seen=set();advance="--advance" in sys.argv
 for module,start,wait,signal,slot in SITES:
  commands={BASE+c['offset']:c for c in script['modules'][module]['commands']}
  assert commands[start]['opcode']==0x2F and commands[wait]['opcode']==0x30
  assert commands[signal]['opcode']==10 and commands[signal]['modes']==[4,0] and commands[signal]['args']==[slot,1]
  requested=commands[start]['args'][0]
  for limit in (0,1,8,59,255):
   for current in (0,1,8,59,255,65535):
    for fraction in (0,65535):
     r=bytearray(0x200000);r[0x10000:0x10000+len(ex)-0x800]=ex[0x800:]
     r[BASE&0x1FFFFF:(BASE&0x1FFFFF)+len(raw)]=raw
     def sw(a,v):struct.pack_into('<I',r,a&0x1FFFFF,v&0xFFFFFFFF)
     def sh(a,v):struct.pack_into('<H',r,a&0x1FFFFF,v&65535)
     def lw(a):return struct.unpack_from('<I',r,a&0x1FFFFF)[0]
     def lh(a):return struct.unpack_from('<H',r,a&0x1FFFFF)[0]
     actor=0x80150000;task=0x80151000
     r[0x150000:0x151100]=bytes(0x1100)
     sw(0x8009D2F0,actor);sw(0x8009D1A0,0);sw(actor+0x9C,BASE+script['modules'][module]['start'])
     sw(task,start);sw(task+16,1);sw(task+20,7 if module==0 else 6)
     r[0x15000F]=limit;sw(actor+0x14,(current<<16)|fraction);sw(actor+0x1C,0x30000)
     sw(0x8009D254,0);r[0xB0CE9]=0
     sw(0x800B6A80+slot*4,0);snapshots=[]
     def tick(stops=()):
      sw(0x8009D300,task)
      regs=execute(r,0x80017018,stop_at=stops,visited_pcs=seen)
      result=dict(pc=f'{lw(task):08X}',dispatch_pc=f'{lw(0x8009CE00):08X}',delay=lw(task+16),flags=lh(task+8),counter=lw(0x800B6A80+slot*4),target=lh(actor+0x12),frame=lw(actor+0x14))
      snapshots.append(result);return regs
     target=min(requested,limit);tick()
     assert lh(actor+0x12)==target and lw(actor+0x98)&0x200
     assert lw(task)==(signal if current==target else wait) and lw(task+16)==1
     assert lw(0x800B6A80+slot*4)==0
     if current!=target:
      if advance:
       for step in range(258):
        execute(r,0x8001A4AC,(actor,),visited_pcs=seen);tick()
        assert lw(0x800B6A80+slot*4)==0
        if lw(task)==signal:break
        assert lw(task)==wait and lw(task+16)==1
       else:raise AssertionError(('animation did not reach wait target',module,start,limit,current,fraction))
      else:
       # Supplied animation state; --advance executes the original ticker instead.
       sw(actor+0x14,(target<<16)|fraction);tick()
      assert lw(task)==signal and lw(task+16)==1 and lw(0x800B6A80+slot*4)==0
     tick(() if signal==0x801C7C60 else (0x80012850,))
     assert lw(0x800B6A80+slot*4)==1
     if signal==0x801C7C60:assert lh(task+8)&0x10 and lw(task)==0x801C7C78
     else:assert lw(0x8009CE00)==signal+40 and lw(task)==signal
     cases.append(dict(module=module,start=f'{start:08X}',limit=limit,current=current,fraction=fraction,snapshots=snapshots))
 assert {0x800171DC,0x80017B34,0x80017B74,0x800173F4,0x800172FC}<=seen
 path=ROOT/('local/live/day2-station-animation-waits.json' if advance else 'local/live/day2-station-waits.json');path.write_text(json.dumps(dict(script_sha256=script['sha256'],scope=('original VM and original animation ticks, speed three frames/pass, no eligible sound records; explicit interleaving, no outer scheduler/render-clock claim' if advance else 'original VM slices and real argument binding; animation progression supplied between slices, no render-clock claim'),cases=cases),indent=2)+'\n')
 print(f'PASS {len(cases)} original station VM wait-to-signal graphs; {path.relative_to(ROOT)}')
if __name__=='__main__':main()
