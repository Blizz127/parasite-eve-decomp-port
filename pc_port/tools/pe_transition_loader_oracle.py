#!/usr/bin/env python3
"""Original 6ECEC control flow with explicit CD/TIM/SDK provider contracts.

Providers do not perform real I/O here. Traces include arguments and field
flags at every call. The changing image base checks retained versus reread
globals. This does not validate the loaded transition overlay or presentation.
"""
import hashlib,itertools,struct,sys
from pe_battle_hud_oracle import ROOT,execute
STOPS=(0x80073A44,0x80074D28,0x8006CDA4,0x8006E6D4,0x800811E4,
       0x800718D0,0x80074DC0,0x80072714,0x800726C4,0x80072724)
COUNTS=(1,1,6,4,0,1,1,0,0,0)
RANGES=((0xB0CD8,0x198),)

def fixture(exe,flag):
 r=bytearray(0x200000);r[0x10000:0x10000+len(exe)-0x800]=exe[0x800:]
 r[0xB0CD8:0xB0E70]=bytes(0x198)
 for a,v in ((0xB0DD8,1013),(0xB0E34,0x80140000),(0xB0E44,0x80141000),
             (0x11614,0x80150000),(0xB0E6C,0x80160000),(0xA77FC,flag),
             (0xB0CD8,0xA5A54042)):
  struct.pack_into('<I',r,a,v)
 for i,v in enumerate((1276,1302,1547,1792,1975)):struct.pack_into('<H',r,0x93168+i*2,v)
 for base,count in ((0x140000,3),(0x141000,262)):
  for i in range(count):struct.pack_into('<I',r,base+i*4,0x800+i*16)
 return r

def fnv(data,h=14695981039346656037):
 for b in data:h=((h^b)*1099511628211)&0xFFFFFFFFFFFFFFFF
 return h

def original(r,profile,mutate,xa):
 issues=[0]*3;polls=[0]*3;phase=0;tim=0;calls=0;trace=14695981039346656037
 regs=execute(r,0x8006ECEC,stop_at=STOPS)
 while regs[31]:
  ret=regs[31]
  word=struct.unpack_from('<I',r,(ret-8)&0x1FFFFF)[0]
  fn=0x80000000|((word&0x3FFFFFF)<<2)
  index=STOPS.index(fn);n=COUNTS[index]
  args=list(regs[4:8])+list(struct.unpack_from('<2I',r,(regs[29]&0x1FFFFF)+16))
  args=args[:n]+[0]*(6-n)
  flags=struct.unpack_from('<I',r,0xB0CD8)[0]
  trace=fnv(struct.pack('<8I',fn,*args,flags),trace);calls+=1
  assert calls<2000
  result=0
  if fn==0x8006CDA4:result=xa
  elif fn==0x8006E6D4:
   phase={0x80140000:0,0x80141000:1,0x80150000:2}[args[2]]
   result=-1 if profile&1 and issues[phase]<2 else 1
   issues[phase]+=1
   struct.pack_into('<I',r,0xB0CD8,flags|0x1004000)
  elif fn==0x800811E4:
   sequence=(2,1,-1,2,1,0) if profile&2 else (2,1,0)
   result=sequence[polls[phase]];polls[phase]+=1
  elif fn==0x800718D0:
   tim+=1
   if mutate and tim==1:struct.pack_into('<I',r,0xB0DD8,2013)
  regs[2]=result&0xFFFFFFFF
  regs=execute(r,ret,stop_at=STOPS,initial_regs=dict(enumerate(regs)))
 assert tim==265 and regs[2]==0
 state=fnv(b''.join(r[a:a+n] for a,n in RANGES))
 return calls,trace,state

def main():
 exe=(ROOT/'build/disc1.candidate.exe').read_bytes()
 assert hashlib.sha1(exe).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
 assert hashlib.sha256(exe[0x5F4EC:0x5F844]).hexdigest()=='1a6ce372cdb4c5b6b25adffefd8e1ab5725059fec34d6e180d5cfd41ed90896c'
 rows=[]
 for flag,profile,mutate,xa in itertools.product((0,0x2000),range(4),range(2),(-1,0,1)):
  calls,trace,state=original(fixture(exe,flag),profile,mutate,xa)
  rows.append((flag,profile,mutate,xa,calls,trace,state))
 out=['/* Generated original6ECEC provider-contract cases. */',
      'static const struct { int flag,profile,mutate,xa; unsigned calls; uint64_t trace,state; } DAY1_transition_cases[]={']
 out.extend(' {'+','.join(str(x) for x in row[:5])+','+
            ','.join(f'UINT64_C(0x{x:016X})' for x in row[5:])+'},' for row in rows)
 out.append('};')
 if '--write-header' in sys.argv:(ROOT/'pc_port/tests/retail_transition_loader_cases.h').write_text('\n'.join(out)+'\n')
 print('PASS:',len(rows),'original transition-loader cases;',sum(row[4] for row in rows),'provider calls')
if __name__=='__main__':main()
