#!/usr/bin/env python3
"""Whole original transition exit, including command issuers and latch clear.
Display/audio dispatch providers are contracts; state is hashed at each call.
"""
import hashlib,itertools,struct,sys
from pe_battle_hud_oracle import ROOT,execute
from pe_m0000i_leaves_oracle import load_overlay
from pe_transition_loader_oracle import fnv
PROVIDERS={0x80074D28:1,0x80074A44:1,0x80074F44:4,0x80074DC0:1,
           0x800755F0:1,0x80086C5C:3,0x8008CBA8:0,0x80073A44:1,
           0x80086FF8:0,0x80087024:0}
RANGES=((0x91A24,4),(0x9D280,4),(0xA77F4,4),(0xB0CD8,4),(0xB0DB0,8),(0xBCD80,20))
STORIES=(0,119,120,121,127,128,129,183,184,185,207,208,209,223,224,225,
         295,296,297,327,328,329,375,376,377,383,384,385,447,448,449,
         519,520,521,527,528,529,0x7FFFFFFF,0x80000000,0xFFFFFFFF)
def put(r,a,v):struct.pack_into('<I',r,a&0x1FFFFF,v&0xFFFFFFFF)
def word(r,a):return struct.unpack_from('<I',r,a&0x1FFFFF)[0]
def state(r):return fnv(b''.join(r[a:a+n] for a,n in RANGES))
def fixture(e,o,b,story,selector,flag,seed):
 r=bytearray(0x200000);r[0x10000:0x10000+len(e)-0x800]=e[0x800:];r[b&0x1FFFFF:(b&0x1FFFFF)+len(o)]=o
 for a,n in RANGES:r[a:a+n]=bytes((seed*71+i*29)&255 for i in range(n))
 for a,v in ((0xA7918,story),(0x19CA68,selector),(0xA77FC,flag),(0x91A24,(0,1,0x80000000,0xFFFFFFFF)[seed])):put(r,a,v)
 r[0xB0DB5]=(0,127,128,255)[seed]
 return r
def original(r,seed,stop):
 before=bytes(r);calls=0;trace=14695981039346656037
 regs=execute(r,0x80192030,stop_at=PROVIDERS)
 while regs[31]:
  ret=regs[31];fn=0x80000000|((word(r,ret-8)&0x3FFFFFF)<<2);n=PROVIDERS[fn]
  args=list(regs[4:4+n]);rect=[0,0]
  if fn==0x80074F44:
   rect=[word(r,args[0]),word(r,args[0]+4)];args=args[1:]
  args+= [0]*(3-len(args));h=state(r)
  trace=fnv(struct.pack('<8I',fn,*args,*rect,h&0xFFFFFFFF,h>>32),trace);calls+=1
  if seed&2 and calls==8:put(r,0xB0CD8,word(r,0xB0CD8)^0xA5A54040)
  if calls==stop:break
  regs[2]=0
  regs=execute(r,ret,stop_at=PROVIDERS,initial_regs=dict(enumerate(regs)))
 outside=bytearray(r[:0x1F0000])
 for a,n in RANGES:outside[a:a+n]=before[a:a+n]
 assert outside==before[:0x1F0000]
 return calls,trace,state(r)
def main():
 e,o,b=load_overlay()
 assert hashlib.sha256(o[0x80192030-b:0x8019234C-b]).hexdigest()=='940c9406cf13145b82ea57eaa96e44b1ad5707b4944c4985dd861c75359b3011'
 for a,z,digest in ((0x800868AC,0x800868F0,'9abbe97787517502ce89f7571a2c71f1f0ca4be1a47804d41fd4179755acfee3'),(0x80038D48,0x80038D74,'ac4ca2cd0bde13e68f56a273946c2302c5585f7a1c7176b5968a33cda1071378')):
  assert hashlib.sha256(e[a-0x80010000+0x800:z-0x80010000+0x800]).hexdigest()==digest
 rows=[]
 def run(story,selector,flag,seed,stop):
  result=original(fixture(e,o,b,story,selector,flag,seed),seed,stop)
  rows.append((story,selector,flag,seed,stop,*result));return result[0]
 for story,selector,flag,seed in itertools.product(STORIES,(*range(11),0xFFFFFFFF),(0,0x2000),range(4)):run(story,selector,flag,seed,0)
 normal=len(rows)
 for story in (0,120):
  for seed in range(4):
   n=original(fixture(e,o,b,story,0,0,seed),seed,0)[0]
   for stop in range(1,n+1):run(story,0,0,seed,stop)
 out=['/* Original92030: story routing, provider state traces and stop prefixes. */',
      'static const struct { uint32_t story,selector,flag,seed,stop,calls; uint64_t trace,state; } DAY1_exit_cases[]={']
 for row in rows:out.append(' {'+','.join(f'0x{x:X}u' for x in row[:6])+f',UINT64_C(0x{row[6]:016X}),UINT64_C(0x{row[7]:016X})'+'},')
 out.append('};')
 if '--write-header' in sys.argv:(ROOT/'pc_port/tests/retail_transition_exit_cases.h').write_text('\n'.join(out)+'\n')
 print('PASS:',normal,'whole original exit cases;',len(rows)-normal,'stop prefixes',flush=True)
if __name__=='__main__':main()
