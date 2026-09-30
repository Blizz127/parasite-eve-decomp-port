#!/usr/bin/env python3
"""Original animation sound scan through spatial projection and audio FIFO."""
import hashlib, struct, sys
from pe_script_sound_oracle import ROOT, execute, fixture as sound_fixture, RANGES as SOUND_RANGES
RANGES = SOUND_RANGES + ((0x94488,0x840),(0x9D1A0,4))
CASES=[]
for entry in (0x8006A318,0x8001A4AC):
 for mode in range(8):
  for current,previous,speed in ((0,0,0),(3,0,0x10000),(0,3,0xFFFF0000),(0,8,0x10000),(8,0,0xFFFF0000),(3,0,0),(65535,0,0x10000),(0,65535,0xFFFF0000)):
   for variant in range(4):CASES.append((entry,mode,current,previous,speed,variant))

def fixture(ex,c):
 entry,mode,current,previous,speed,variant=c
 r=sound_fixture(ex,dict(key=300,sound=1,x=-300,queue=variant))
 r[0x94488:0x94CC8]=bytes(0x840)
 def sw(a,v):struct.pack_into('<I',r,a,v&0xFFFFFFFF)
 def sh(a,v):struct.pack_into('<H',r,a,v&65535)
 sw(0x9D254,0x80150000 if mode&1 else 0)
 sw(0x9D1A0,2 if mode&2 else 0);sw(0xB0CD8,0x800000 if mode&4 else 0)
 sw(0x150014,current<<16|0xABCD);sw(0x150018,previous<<16|0x1234);sw(0x15001C,speed)
 sw(0x150098,(0,0x100,0x200,0x300)[variant]);sh(0x150012,current)
 r[0x15000E]=5;r[0x15000F]=8
 r[0xB0CE9]=(0,1,8,255)[variant];r[0xB0CEA]=variant%2
 for i in range(259):
  a=0x94488+i*8
  # Four independent mismatches plus valid records at both endpoints.
  r[a:a+4]=bytes((2 if i%5!=0 else 3,7 if i%5!=1 else 8,5 if i%5!=2 else 6,(0,1,2,3,8)[i%5]))
  sh(a+4,0 if i%7==0 else 0x7E2);sh(a+6,0x7E3)
 return r

def fingerprint(r):
 h=14695981039346656037
 for a,n in RANGES:
  for b in r[a:a+n]:h=((h^b)*1099511628211)&0xFFFFFFFFFFFFFFFF
 return h

def main():
 ex=(ROOT/'build/disc1.candidate.exe').read_bytes()
 assert hashlib.sha1(ex).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
 common=None;patches=[];cases=[]
 for k,c in enumerate(CASES):
  r=fixture(ex,c);seed={a+i:struct.unpack_from('<I',r,a+i)[0] for a,n in RANGES for i in range(0,n,4)}
  if common is None:common={a:v for a,v in seed.items() if v}
  first=len(patches);patches.extend((a,v) for a,v in seed.items() if v!=common.get(a,0))
  regs=execute(r,c[0],(0x80150000,),instruction_budget=900000)
  if c[0]==0x8006A318:assert regs[2]==0
  cases.append((first,len(patches),c[0],fingerprint(r)))
 out=['/* Generated original animation sound/FIFO expectations. */']
 for name,rows in (('ranges',RANGES),('common',sorted(common.items())),('patches',patches)):
  out.append(f'static const uint32_t ANS_{name}[][2]={{')
  out.extend(f'{{0x{a:X}u,0x{b:X}u}},' for a,b in rows);out.append('};')
 out.append('static const struct { unsigned first,end; uint32_t entry; uint64_t hash; } ANS_cases[]={')
 out.extend(f'{{{a},{b},0x{e:X}u,UINT64_C(0x{h:016X})}},' for a,b,e,h in cases);out.append('};')
 header='\n'.join(out)+'\n';path=ROOT/'pc_port/tests/retail_animation_sound_cases.h'
 if '--write-header' in sys.argv:path.write_text(header)
 else:assert path.read_text()==header
 print(f'PASS {len(cases)} original animation sound graphs, including ticker integration and audio FIFO')
if __name__=='__main__':main()
