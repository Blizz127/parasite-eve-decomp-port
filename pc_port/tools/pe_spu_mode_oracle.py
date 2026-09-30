#!/usr/bin/env python3
"""Original mode graph, with explicit DMA/event providers and RAM registers."""
import hashlib,struct,sys
from pe_battle_hud_oracle import ROOT,execute
RANGES=((0x9B384,4),(0x9B390,0x30),(0x9B3FC,4),(0x9B414,0x44),(0x9B464,0x30),(0x9C4C0,0x400),(0x9C8C0,0x2A8),(0x151000,0x200),(0x152000,0x10),(0x9D24C,4))
def fnv(data):
 h=14695981039346656037
 for b in data:h=((h^b)*1099511628211)&0xffffffffffffffff
 return h

def main():
 ex=(ROOT/'build/disc1.candidate.exe').read_bytes();assert hashlib.sha1(ex).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
 common=None;patches=[];cases=[]
 for mode in range(10):
  for same in (0,1):
   for cb in (0,0x80085098):
    r=bytearray(0x200000);r[0x10000:0x10000+len(ex)-2048]=ex[2048:];spu=bytearray([0xa5])*0x80000
    def sw(a,v):struct.pack_into('<I',r,a&0x1fffff,v&0xffffffff)
    def sh(a,v):struct.pack_into('<H',r,a&0x1fffff,v&0xffff)
    def lw(a):return struct.unpack_from('<I',r,a&0x1fffff)[0]
    def lh(a):return struct.unpack_from('<H',r,a&0x1fffff)[0]
    sw(0x9B384,0x100);sw(0x9B3FC,0x80151000);sw(0x9B3A0,mode if same else 0xffffffff)
    sw(0x9B464,0x80152000);sw(0x152000,0x40001010);sw(0x9B424,3);sw(0x9B434,cb);sw(0x9B418,int(bool(cb)))
    sh(0x1511aa,0x80 if same else 0);r[0x9C4C0:0x9C8C0]=bytes(0x400)
    seed={a+i:lw(a+i) for a,n in RANGES for i in range(0,n,4)}
    if common is None:common={a:v for a,v in seed.items() if v}
    first=len(patches);patches.extend((a,v) for a,v in seed.items() if v!=common.get(a,0))
    regs=execute(r,0x8008CB54,(mode,),stop_at=(0x8007D778,0x8008D7B0));pending=None;events=0
    for _ in range(5000):
     if regs[31]==0:break
     if regs[31] in (0x8008D718,0x8008D720,0x8008D734):
      op=regs[4]
      if op==2:sh(0x9B414,regs[5]>>(lw(0x9B424)&31));sh(0x1511a6,lh(0x9B414))
      elif op==1:sw(0x9B44C,0);sh(0x1511aa,(lh(0x1511aa)&0xffcf)|0x20)
      else:
       assert op==3 and pending is None
       size=(regs[6]+63)&~63;pending=(regs[5]&0x1fffff,lh(0x9B414)<<3,size)
       sw(0x9B450,regs[5]);sw(0x9B454,size//64)
      regs[2]=0
     else:
      assert regs[31]==0x8008D748 and regs[4]==0x100 and pending is not None
      src,dst,size=pending;first_size=min(size,len(spu)-dst)
      spu[dst:dst+first_size]=r[src:src+first_size]
      if first_size<size:spu[:size-first_size]=r[src+first_size:src+size]
      pending=None
      sh(0x1511aa,lh(0x1511aa)&0xffcf);events+=1;regs[2]=1
     regs=execute(r,regs[31],initial_regs=dict(enumerate(regs)),stop_at=(0x8007D778,0x8008D7B0))
    else:raise AssertionError('mode did not complete')
    assert pending is None and lw(0x9B3A0)==mode and lw(0x9B434)==cb
    cases.append((mode,first,len(patches),fnv(b''.join(r[a:a+n] for a,n in RANGES)),fnv(spu),events))
 lines=['/* Original 8CB54 graph; DMA/WaitEvent provider contracts explicit. */']
 for name,rows in (('ranges',RANGES),('common',sorted(common.items())),('patches',patches)):
  lines.append(f'static const uint32_t SM_{name}[][2]={{');lines.extend(f'{{0x{a:X}u,0x{v:X}u}},' for a,v in rows);lines.append('};')
 lines.append('static const struct { unsigned mode,first,end; uint64_t ram,spu; unsigned events; } SM_cases[]={');lines.extend('{%d,%d,%d,UINT64_C(0x%016X),UINT64_C(0x%016X),%d},'%c for c in cases);lines.append('};')
 p=ROOT/'pc_port/tests/retail_spu_mode_cases.h';out='\n'.join(lines)+'\n'
 if '--check' in sys.argv:assert p.read_text()==out
 else:p.write_text(out)
 print(f'PASS {len(cases)} original mode graphs with explicit DMA/event providers')
if __name__=='__main__':main()
