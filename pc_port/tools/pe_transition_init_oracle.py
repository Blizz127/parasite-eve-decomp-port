#!/usr/bin/env python3
"""Original91854 selection logic; display/query/byte setter/CD are providers."""
import hashlib,itertools,struct,sys
from pe_battle_hud_oracle import ROOT,execute
from pe_m0000i_leaves_oracle import load_overlay
from pe_transition_loader_oracle import fnv
SCENES=(0x179,0x18,0x2E,0x75,0x67,0xBF,0xC0,0x5D,0x60,0x3A,0x176,0x7D,0x104,0xA1,0xB7,0x122,0,0xFFFF0179)
ENCOUNTERS=(0x80,0x208,0xB8,0xC0,0xD0,0xD8,0xE0,0xE4,0x148,0x160,0x178,0x180,0x1C0,0x1C8,0,0xFFFF0080)
RANGES=((0x19BFF0,0x20),(0x19C1F0,4),(0x19CC50,4))
STOPS=(0x8019BD78,0x8005BCB0,0x800371A4,0x80191C94)
def state(r):return fnv(b''.join(r[a:a+n] for a,n in RANGES))
def main():
 exe,overlay,base=load_overlay();code=overlay[0x80191854-base:0x80191C94-base]
 assert hashlib.sha256(code).hexdigest()=='0cf01123a041bee57cb1aa131dd7ba72d07cce3d1377d234ca5f331b9cba4da8'
 rows=[]
 for scene,encounter,profile in itertools.product(SCENES,ENCOUNTERS,range(8)):
  r=bytearray(0x200000);r[0x10000:0x10000+len(exe)-0x800]=exe[0x800:];r[base&0x1FFFFF:(base&0x1FFFFF)+len(overlay)]=overlay
  for a,n in RANGES:r[a:a+n]=bytes((scene+encounter+profile+i*17)&255 for i in range(n))
  for a,v in ((0xA77F4,scene),(0xA7918,encounter),(0xB0CD8,0x40000000 if profile&1 else 0),(0xA77FC,0x2000 if profile&2 else 0)):struct.pack_into('<I',r,a,v)
  trace=14695981039346656037;calls=0;regs=execute(r,0x80191854,stop_at=STOPS)
  while regs[31]:
   ret=regs[31];w=struct.unpack_from('<I',r,(ret-8)&0x1FFFFF)[0];fn=0x80000000|((w&0x3FFFFFF)<<2)
   assert fn==STOPS[calls];calls+=1
   trace=fnv(struct.pack('<IIQ',fn,regs[4] if fn==0x800371A4 else 0,state(r)),trace)
   regs[2]=0xFFFFFFFF if fn==0x8005BCB0 and profile&4 else 0
   regs=execute(r,ret,initial_regs=dict(enumerate(regs)),stop_at=STOPS)
  assert calls==4;rows.append((scene,encounter,profile,trace,state(r)))
 out=['/* Original91854 with explicit display/query/setter/package providers. */','static const struct { uint32_t scene,encounter,profile; uint64_t trace,state; } DAY1_init_cases[]={']
 for row in rows:out.append(' {'+','.join(f'0x{x:X}u' for x in row[:3])+','+','.join(f'UINT64_C(0x{x:016X})' for x in row[3:])+'},')
 out.append('};')
 if '--write-header' in sys.argv:(ROOT/'pc_port/tests/retail_transition_init_cases.h').write_text('\n'.join(out)+'\n')
 print('PASS:',len(rows),'original initializer cases')
if __name__=='__main__':main()
