#!/usr/bin/env python3
"""Original EA305/314 audio control through complete command FIFO."""
import hashlib,struct,sys
from pe_script_sound_oracle import ROOT,execute,fixture as sound_fixture,RANGES as SOUND_RANGES
RANGES=SOUND_RANGES
def fingerprint(r):
 h=14695981039346656037
 for a,n in RANGES:
  for b in r[a:a+n]:h=((h^b)*1099511628211)&0xFFFFFFFFFFFFFFFF
 return h

def main():
 ex=(ROOT/'build/disc1.candidate.exe').read_bytes()
 assert hashlib.sha1(ex).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
 common=None;patches=[];cases=[]
 for key in (305,314):
  for duration in (0,1,60,127,128,255,0x80000000,0xFFFFFFFF):
   for volume in (0,127,128,0xFFFFFFFF):
    for queue in (0,1,17):
     r=sound_fixture(ex,dict(key=300,sound=1,x=-300,queue=queue))
     for i,value in enumerate((key,duration,volume)):
      struct.pack_into('<I',r,0x140240+i*4,value)
     seed={a+i:struct.unpack_from('<I',r,a+i)[0] for a,n in RANGES for i in range(0,n,4)}
     if common is None:common={a:v for a,v in seed.items() if v}
     first=len(patches);patches.extend((a,v) for a,v in seed.items() if v!=common.get(a,0))
     regs=execute(r,0x80015DAC,(0x80140200,),instruction_budget=100000);assert regs[2]==1
     assert struct.unpack_from('<I',r,0xBCD80)[0]==(0xA9 if key==305 else 0xF1)
     if key==305:
      assert struct.unpack_from('<2I',r,0xBCD84)==((duration*2)&255,volume&127)
     cases.append((first,len(patches),fingerprint(r)))
 lines=['/* Original EA305/314 control and audio FIFO. */']
 for name,rows in (('ranges',RANGES),('common',sorted(common.items())),('patches',patches)):
  lines.append(f'static const uint32_t SAC_{name}[][2]={{')
  lines.extend(f'{{0x{a:X}u,0x{b:X}u}},' for a,b in rows);lines.append('};')
 lines.append('static const struct { unsigned first,end; uint64_t hash; } SAC_cases[]={')
 lines.extend(f'{{{a},{b},UINT64_C(0x{h:016X})}},' for a,b,h in cases);lines.append('};')
 header='\n'.join(lines)+'\n';path=ROOT/'pc_port/tests/retail_script_audio_control_cases.h'
 if '--check' in sys.argv:assert path.read_text()==header
 else:path.write_text(header)
 print(f'PASS {len(cases)} original script audio-control/FIFO graphs')
if __name__=='__main__':main()
