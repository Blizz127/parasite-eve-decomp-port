#!/usr/bin/env python3
"""Retail 6D60C exit music states with explicit loader/SPU provider contracts.

Checks state transitions, timer arithmetic, return values and provider calls;
provider internals and audible output are outside this comparison.
"""
import hashlib,itertools,struct,sys
from pe_battle_hud_oracle import ROOT,execute
STOPS=(0x80086FF8,0x8006CDA4,0x8006D2B8,0x80086464,0x80086C5C,0x8006DB48)
RANGES=((0x9D188,12),(0xB0CD8,0x198))
def cases():
 for state,flags,timer,busy,handle in itertools.product((0x40,0x3E,0x33),(0,4,0x40,0x44,0xC4),(-1,0,1),(0,1,2),(-1,0,7)):
  yield (state,flags,timer,busy,handle,30)
 for flags,elapsed in itertools.product((0,4,0x44),(-5,0,51,52,59,60,70)):
  yield (0x32,flags,99,0,7,elapsed)

def fixture(exe,c):
 r=bytearray(0x200000);r[0x10000:0x10000+len(exe)-0x800]=exe[0x800:]
 for a,n in RANGES:r[a:a+n]=bytes(n)
 state,flags,timer,busy,handle,elapsed=c
 def w(a,v):struct.pack_into('<I',r,a,v&0xFFFFFFFF)
 def h(a,v):struct.pack_into('<H',r,a,v&65535)
 w(0xB0CD8,flags|0x12340000);r[0xB0DCA]=state
 r[0xB0DB2]=0x83;r[0xB0DB4]=0xF9;r[0xB0DD6]=101
 w(0xB0E00,0x80142000);w(0xB0E6C,0x80140000)
 h(0xB0DC0,-1);w(0x9D190,timer);w(0x9D18C,100);w(0x9CDA4,100+elapsed)
 return r

def fingerprint(r):
 h=14695981039346656037
 for a,n in RANGES:
  for b in r[a:a+n]:h=((h^b)*1099511628211)&0xFFFFFFFFFFFFFFFF
 return h

def original(r,c):
 regs=execute(r,0x8006D60C,(0,),stop_at=STOPS);calls=[]
 while regs[31]:
  pc=struct.unpack_from('<I',r,((regs[31]-8)&0x1FFFFF))[0]
  fn=0x80000000|((pc&0x3FFFFFF)<<2);assert fn in STOPS,hex(fn)
  args=list(regs[4:8]);sp=regs[29]&0x1FFFFF
  if fn==0x80086FF8:args=[];value=0
  elif fn==0x8006CDA4:args+=list(struct.unpack_from('<2I',r,sp+16));value=int(c[3]==1)
  elif fn==0x8006D2B8:
   args+=[struct.unpack_from('<I',r,sp+16)[0]];struct.pack_into('<I',r,args[3]&0x1FFFFF,1)
   args[3]=0;value=int(c[3]==2)
  elif fn==0x80086464:args=args[:1];value=c[4]
  elif fn==0x80086C5C:args=args[:3];value=0
  else:value=0
  calls.append((fn,*args,*([0]*(6-len(args)))))
  regs[2]=value&0xFFFFFFFF
  regs=execute(r,regs[31],stop_at=STOPS,initial_regs=dict(enumerate(regs)))
  assert len(calls)<15
 return regs[2],calls

def main():
 exe=(ROOT/'build/disc1.candidate.exe').read_bytes()
 assert hashlib.sha1(exe).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
 rows=[];trace=[]
 for c in cases():
  r=fixture(exe,c);v,t=original(r,c);first=len(trace);trace.extend(t)
  rows.append((*c,v,first,len(trace),fingerprint(r)))
 out=['/* Generated original6D60C ambient reload; controlled providers. */','static const uint32_t ambient_calls[][7]={']
 out.extend(' {'+','.join(f'0x{v:X}u' for v in t)+'},' for t in trace);out.append('};')
 out.append('static const struct { int state,flags,timer,busy,handle,elapsed; uint32_t result,first,end; uint64_t hash; } ambient_cases[]={')
 out.extend(' {'+','.join(str(x) for x in r[:-1])+f',UINT64_C(0x{r[-1]:X})'+'},' for r in rows);out.append('};')
 if '--write-header' in sys.argv:(ROOT/'pc_port/tests/retail_ambient_reload_cases.h').write_text('\n'.join(out)+'\n')
 print('PASS:',len(rows),'original ambient reload cases;',len(trace),'provider calls')
if __name__=='__main__':main()
