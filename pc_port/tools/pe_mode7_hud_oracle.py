#!/usr/bin/env python3
"""Original full mode7 HUD, shared sparse fixtures with native tests."""
import hashlib,struct,sys
from pe_scripted_exit_oracle import fixture,RANGES,words,fingerprint,ROOT
from pe_battle_hud_oracle import execute

RANGES=RANGES+((0xB01BC,0xAF0),(0x180000,0x100))

def fingerprint(r):
 h=14695981039346656037
 for a,n in RANGES:
  for b in r[a:a+n]:h=((h^b)*1099511628211)&0xFFFFFFFFFFFFFFFF
 return h

def make(ex,phase,v):
 bank=phase&1;tick=(phase>>1)&3;at=(8999,9000,65535)[phase>>3]
 r,s=fixture(ex,dict(bank=bank,tick=tick,at=at,state=(0,0x2000,0x4000,0x6000)[v&3],color=136 if v&4 else 255,kind=2))
 def put(a,b):r[a:a+len(b)]=b;s[a:a+len(b)]=b
 def sw(a,x):put(a,struct.pack('<I',x&0xFFFFFFFF))
 def sh(a,x):put(a,struct.pack('<H',x&65535))
 def sb(a,x):put(a,bytes((x&255,)))
 for a,n in RANGES[-2:]:put(a,bytes([0xA5])*n)
 sw(0xB0E38,0x80180000);sw(0xB0E3C,0x80180080)
 sw(0x9D28C,7);sw(0x9D244,0);sw(0x9D230,v%3)
 for j,a in enumerate((0x144050,0x144058,0xA5D5C+0xD0,0xA5E38+0xD0)):
  sh(a,(0,9,123,32767,-1,-32768,9999,42)[(v+j)%8])
  sh(a+2,100+j*17);sh(a+4,70+j*9)
  sb(a+6,(0,1,29,30,31,255,30,0)[(v+j)%8]);sb(a+7,(v+j)%5)
 for j,a in enumerate((0x140000,0x141000,0x142000)):
  sh(a+0x210,220+j*3);sh(a+0x212,120+j*4)
 if v==8:sw(0x9D20C,0)
 if v==9:sw(0x141000,0)
 return r,s

def main():
 ex=(ROOT/'build/disc1.candidate.exe').read_bytes();assert hashlib.sha1(ex).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
 _,s=make(ex,0,0);base=words(s);common=[(i*4,v) for i,v in enumerate(base) if v];patches=[];cases=[]
 for phase in range(24):
  for v in range(10):
   r,s=make(ex,phase,v);start=len(patches);patches.extend((i*4,x) for i,(x,y) in enumerate(zip(words(s),base)) if x!=y)
   execute(r,0x8002D1F0,scratchpad=bytearray(0x400),instruction_budget=1000000)
   cases.append((phase,v,start,len(patches),fingerprint(r)))
 out=[]
 for name,rows in (('ranges',RANGES),('common',common),('patches',patches)):
  out.append(f'static const uint32_t M7_{name}[][2]={{');out.extend(f'{{0x{a:X}u,0x{b:X}u}},' for a,b in rows);out.append('};')
 out.append('static const struct { unsigned phase,variant,first,end; uint64_t hash; } M7_cases[]={')
 out.extend('{%d,%d,%d,%d,UINT64_C(0x%016X)},'%c for c in cases);out.append('};')
 header='\n'.join(out)+'\n';path=ROOT/'pc_port/tests/retail_mode7_hud_cases.h'
 if '--check' in sys.argv:assert path.read_text()==header
 else:path.write_text(header)
 print(f'PASS {len(cases)} original full mode7 HUD cases')
if __name__=='__main__':main()
