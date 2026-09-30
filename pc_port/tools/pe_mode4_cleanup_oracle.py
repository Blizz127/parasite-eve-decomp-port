#!/usr/bin/env python3
"""Original full mode4 cleanup, shared sparse fixtures with native tests."""
import hashlib,struct,sys
from pe_scripted_exit_oracle import fixture,RANGES,words,fingerprint,ROOT
from pe_battle_hud_oracle import execute

def make(ex,phase,v):
 r,s=fixture(ex,dict(phase=phase,flags=0x82,empty_list=v==7,media=64 if v==8 else 0,busy=4 if v==8 else 0,timer=1 if v==8 else 0))
 def put(a,b):r[a:a+len(b)]=b;s[a:a+len(b)]=b
 def sw(a,x):put(a,struct.pack('<I',x))
 def sb(a,x):put(a,bytes((x,)))
 sw(0x9D28C,4);sw(0x141098,(0x40000000 if v&1 else 0)|(0x40 if v&2 else 0))
 sb(0xA5D5C+0xAF,1 if v&4 else 0)
 sb(0x140252,1 if v==1 else 0);sb(0xB0D8A,1 if v==2 else 0)
 sb(0x141252,1 if v&4 else 0);sb(0x142252,1 if v&4 else 0)
 if v==3:sw(0x141000,0)
 if v==5:sw(0x142000,0)
 return r,s

def main():
 ex=(ROOT/'build/disc1.candidate.exe').read_bytes();assert hashlib.sha1(ex).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
 _,s=make(ex,0,0);base=words(s);common=[(i*4,v) for i,v in enumerate(base) if v];patches=[];cases=[]
 for phase in (0,1,2,3,255):
  for v in range(9 if phase==3 else 8):
   r,s=make(ex,phase,v);start=len(patches);patches.extend((i*4,x) for i,(x,y) in enumerate(zip(words(s),base)) if x!=y)
   execute(r,0x8002B94C,scratchpad=bytearray(0x400),instruction_budget=1000000)
   cases.append((phase,v,start,len(patches),fingerprint(r)))
 out=[]
 for name,rows in (('ranges',RANGES),('common',common),('patches',patches)):
  out.append(f'static const uint32_t M4_{name}[][2]={{');out.extend(f'{{0x{a:X}u,0x{b:X}u}},' for a,b in rows);out.append('};')
 out.append('static const struct { unsigned phase,variant,first,end; uint64_t hash; } M4_cases[]={')
 out.extend('{%d,%d,%d,%d,UINT64_C(0x%016X)},'%c for c in cases);out.append('};')
 header='\n'.join(out)+'\n';path=ROOT/'pc_port/tests/retail_mode4_cleanup_cases.h'
 if '--check' in sys.argv:assert path.read_text()==header
 else:path.write_text(header)
 print(f'PASS {len(cases)} original full mode4 cleanup cases')
if __name__=='__main__':main()
