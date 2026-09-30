#!/usr/bin/env python3
"""Execute original95BC8 with overlapping target and state records."""
import hashlib,itertools,struct,sys
from pe_battle_hud_oracle import ROOT,execute
from pe_m0000i_leaves_oracle import load_overlay
from pe_transition_loader_oracle import fnv
RANGES=((0x19C020,0xA0),(0x19C330,16),(0x19C810,16),(0x150000,32))
ADDRS=(0x150000,0x150001,0x150002,0x19C050,0x19C054,0x19C056,0x19C05C,0x19C060,0x19C06C,0x19C07C,0x19C084,0x19C08C,0x19C09C,0x19C0AC,0x19C0B4,0x19C330,0x19C810)
SHIFTS=(0,1,15,16,31,32,255,0x10001)
def main():
 exe,o,b=load_overlay();body=o[0x80195BC8-b:0x80195D3C-b];assert hashlib.sha256(body).hexdigest()=='54414529b327f805ed554a27b70d8554e90762dacb7f348973b471cd6a1afbcd';rows=[]
 for seed,a,c in itertools.product(range(8),ADDRS,ADDRS):
  r=bytearray(0x200000);r[b&0x1FFFFF:(b&0x1FFFFF)+len(o)]=o
  for x,n in RANGES:r[x:x+n]=bytes((seed*31+i*29)&255 for i in range(n))
  for step in range(2):
   execute(r,0x80195BC8,(a+0x80000000,c+0x80000000,SHIFTS[seed],SHIFTS[7-seed]))
   h=fnv(b''.join(r[x:x+n] for x,n in RANGES));rows.append((seed,a,c,step,h))
 out=['/* Original95BC8 history: aliases, unaligned inputs and shift limits. */','static const struct { uint32_t seed,target,eye,step; uint64_t hash; } DAY1_target_cases[]={']
 for seed,a,c,step,h in rows:out.append(f' {{{seed},0x{a+0x80000000:X}u,0x{c+0x80000000:X}u,{step},UINT64_C(0x{h:016X})}},')
 out.append('};')
 if '--write-header' in sys.argv:(ROOT/'pc_port/tests/retail_camera_target_cases.h').write_text('\n'.join(out)+'\n')
 print('PASS:',len(rows),'original target setup history steps')
if __name__=='__main__':main()
