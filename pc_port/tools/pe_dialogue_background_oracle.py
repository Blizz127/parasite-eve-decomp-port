#!/usr/bin/env python3
"""Original ED2101/2300 and complete empty-window background packet update."""
import hashlib,struct,sys
from pe_battle_hud_oracle import ROOT,execute
RANGES=((0x9CE90,0x48),(0x9EC70,0x70),(0xBCEA8,224),(0xB0CD8,4),
        (0x150000,0x4800),(0x155000,0x40),(0x156000,0x80),
        (0xB0E38,24),(0x9CDDC,4))
def digest(r):
 h=14695981039346656037
 for a,n in RANGES:
  for b in r[a:a+n]:h=((h^b)*1099511628211)&0xFFFFFFFFFFFFFFFF
 return h

def main():
 ex=(ROOT/'build/disc1.candidate.exe').read_bytes()
 assert hashlib.sha1(ex).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
 common=None;patches=[];cases=[];seen=set()
 for key in (2101,2300):
  for value in (0,1,256,0x80000000,0xFFFFFFFF):
   for bank in (0,1):
    for flags in (0,0x800,0xFFFFF7FF,0xFFFFFFFF):
     r=bytearray(0x200000);r[0x10000:0x10000+len(ex)-2048]=ex[2048:]
     def sw(a,v):struct.pack_into('<I',r,a&0x1FFFFF,v&0xFFFFFFFF)
     def lw(a):return struct.unpack_from('<I',r,a&0x1FFFFF)[0]
     execute(r,0x800371B0,(0x80160000,))
     sw(0x9CDDC,bank);sw(0xB0CD8,flags)
     sw(0x9CED4,1 if key==2101 else 0xDEADBEEF)
     for i in range(2):
      sw(0xB0E38+i*4,0x80155000+i*16);sw(0xB0E44+i*4,0x80150000+i*0x2400)
      for j in range(4):sw(0x155000+i*16+j*4,0xFFFFFF if j==0 else 0x155000+i*16+(j-1)*4)
     for i,v in enumerate((key,value,0,0,0,0,0,0)):
      sw(0x156000+i*4,0x80156040+i*4);sw(0x156040+i*4,v)
     seed={a+i:lw(a+i) for a,n in RANGES for i in range(0,n,4)}
     if common is None:common={a:v for a,v in seed.items() if v}
     first=len(patches);patches.extend((a,v) for a,v in seed.items() if v!=common.get(a,0))
     regs=execute(r,0x80016910,(0x80156000,),visited_pcs=seen);assert regs[2]==1
     assert lw(0x9CED4)==(1 if key==2101 else int(value!=0))
     assert lw(0xB0CD8)==(flags|0x800 if key==2101 else flags)
     before=digest(r)
     execute(r,0x80037870,visited_pcs=seen,instruction_budget=100000)
     cases.append((first,len(patches),before,digest(r)))
 assert {0x800375D0,0x80037870}<=seen
 lines=['/* Original ED scene flags and empty-window background packets. */']
 for name,rows in (('ranges',RANGES),('common',sorted(common.items())),('patches',patches)):
  lines.append(f'static const uint32_t DBG_{name}[][2]={{')
  lines.extend(f'{{0x{a:X}u,0x{b:X}u}},' for a,b in rows);lines.append('};')
 lines.append('static const struct { unsigned first,end; uint64_t flag,draw; } DBG_cases[]={')
 lines.extend(f'{{{a},{b},UINT64_C(0x{h:016X}),UINT64_C(0x{d:016X})}},' for a,b,h,d in cases);lines.append('};')
 header='\n'.join(lines)+'\n';target=ROOT/'pc_port/tests/retail_dialogue_background_cases.h'
 if '--check' in sys.argv:assert target.read_text()==header
 else:target.write_text(header)
 print(f'PASS {len(cases)} original scene flag/background packet graphs')
if __name__=='__main__':main()
