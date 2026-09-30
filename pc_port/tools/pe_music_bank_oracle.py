#!/usr/bin/env python3
"""Execute complete original6D2B8; controlled loader/queue and BIOS memcpy contracts.

This checks bank lookup, channel state, copying and retry control flow. It does
not assert equivalence of the called CD/SPU providers or audible playback.
"""
import hashlib,struct,sys,itertools
from pe_battle_hud_oracle import ROOT,execute
RANGES=((0x9D180,8),(0xB0CD8,0x198),(0x140000,0x40),(0x141000,0x40),(0x142000,4))
STOPS=(0x80086FF8,0x8006CDA4,0x80071A34)

def fixture(exe,kind):
 r=bytearray(0x200000);r[0x10000:0x10000+len(exe)-0x800]=exe[0x800:]
 for a,n in RANGES:r[a:a+n]=bytes(n)
 def w(a,v):struct.pack_into('<I',r,a,v&0xFFFFFFFF)
 def h(a,v):struct.pack_into('<H',r,a,v&0xFFFF)
 w(0xB0E64,0x80130000);w(0x130004,0x40)
 w(0x130070,(2<<22)|0x100)
 w(0x130100,0xAB00001D);w(0x130104,0xCD000200);h(0x130108,5);h(0x13010A,7)
 w(0x13010C,0xAB000011);w(0x130110,0xCD000220);h(0x130114,9);h(0x130116,200)
 r[0x130200:0x130240]=bytes(range(1,65))
 w(0xB0E00,0x80140000);w(0xB0E04,0x80141000);w(0xB0E6C,0x80150000)
 w(0x142000,0xDEADBEEF);w(0xB0CD8,0xA5A50100)
 r[0xB0DB2:0xB0DB8]=bytes((0xFF,0xFF,0xFF,0xA1,0xFF,0xB2))
 if kind in (1,2,3,4,5):
  slot=1 if kind in (3,4) else 0
  r[0xB0DB4+slot*2]=7;r[0xB0DB2+slot]=5
  if kind in (2,4):w(0xB0CD8,0xA5A50100|(0x40<<slot))
  if kind==5:
   r[0xB0DB6]=7;r[0xB0DB3]=6;w(0xB0CD8,0xA5A501C0)
 if kind in (6,7):
  w(0x130070,0x100)
  if kind==7:r[0xB0DB4]=7;r[0xB0DB2]=0xFE
 if kind in (8,9):
  r[0xB0DC9]=7 if kind==8 else 9;w(0x9D180,0x80130100);h(0x9D184,5)
 if kind==11:
  w(0x130070,0x100);r[0xB0DB4]=0x80;r[0xB0DB2]=0x80
 return r

def fingerprint(r):
 h=14695981039346656037
 for a,n in RANGES:
  for b in r[a:a+n]:h=((h^b)*1099511628211)&0xFFFFFFFFFFFFFFFF
 return h

def original(r,id,load,second,blocking,busy):
 regs=execute(r,0x8006D2B8,(id,load,second,0x80142000,blocking),stop_at=STOPS)
 calls=[];loads=0
 while regs[31]:
  ret=regs[31]
  if ret==0x8006D4AC:
   calls.append((0x80086FF8,0,0,0,0,0,0));regs[2]=0
  elif ret==0x8006D544:
   sp=regs[29]&0x1FFFFF
   args=(*regs[4:8],*struct.unpack_from('<2I',r,sp+16))
   calls.append((0x8006CDA4,*args))
   regs[2]=(1 if loads<busy else (-1 if busy==-1 else 0))&0xFFFFFFFF;loads+=1
  elif ret==0x8006D5A4:
   dst,src,n=regs[4:7]
   for i in range(n):r[(dst+i)&0x1FFFFF]=r[(src+i)&0x1FFFFF]
   regs[2]=dst
  else:raise AssertionError(hex(ret))
  assert len(calls)<20
  regs=execute(r,ret,stop_at=STOPS,initial_regs=dict(enumerate(regs)))
 return regs[2],calls

def main():
 exe=(ROOT/'build/disc1.candidate.exe').read_bytes()
 assert hashlib.sha1(exe).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
 assert hashlib.sha256(exe[0x5DAB8:0x5DE0C]).hexdigest()=='ea86f38da35cb0224e6a50f581e8e02044d4e91528a33ac29d95a7c856fe0029'
 cases=[];trace=[]
 for kind,load,second,blocking,busy in itertools.product(range(12),(0,1),(0,1,-1),(0,1,2,3),(-1,0,2)):
  id=200 if kind==10 else (-128 if kind==11 else 7)
  r=fixture(exe,kind);result,calls=original(r,id,load,second,blocking,busy)
  first=len(trace);trace.extend(calls)
  cases.append((kind,id,load,second,blocking,busy,result,first,len(trace),fingerprint(r)))
 out=['/* Generated complete original6D2B8 cases; explicit provider contracts. */','static const uint32_t DAY1_bank_calls[][7]={']
 out.extend(' {'+','.join(f'0x{v:08X}u' for v in row)+'},' for row in trace)
 out+=['};','static const struct { int kind,id,load,second,blocking,busy; uint32_t result,first,end; uint64_t hash; } DAY1_bank_cases[]={']
 out.extend(' {'+','.join(str(v) for v in row[:-1])+f',UINT64_C(0x{row[-1]:016X})'+'},' for row in cases)
 out.append('};')
 if '--write-header' in sys.argv:(ROOT/'pc_port/tests/retail_music_bank_cases.h').write_text('\n'.join(out)+'\n')
 print('PASS:',len(cases),'complete original music-bank cases;',len(trace),'provider calls')
if __name__=='__main__':main()
