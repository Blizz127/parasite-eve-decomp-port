#!/usr/bin/env python3
"""Complete original 3EB04 with real controller SDK and menu-list lookup."""
import hashlib,struct,sys
from pe_m0000i_leaves_oracle import load_overlay
from pe_battle_hud_oracle import execute,ROOT
from pe_transition_loader_oracle import fnv
RANGES=((0x9D140,0x1A0),(0xA5B70,0x1E0),(0xA76F0,0x100),(0xB0CD8,0xF0),(0xBE9A0,0x44),(0x150000,0x40),(0x9B738,12),(0x92200,36))
def fixture(ex,n):
 r=bytearray(0x200000);r[0x10000:0x10000+len(ex)-0x800]=ex[0x800:];seed=57+n*101
 def rand():
  nonlocal seed
  seed^=(seed<<13)&0xffffffff;seed^=seed>>17;seed^=(seed<<5)&0xffffffff;return seed
 def sw(a,v):struct.pack_into('<I',r,a,v&0xffffffff)
 def sh(a,v):struct.pack_into('<H',r,a,v&65535)
 for a,length in RANGES:
  for i in range(0,length,4):sw(a+i,rand())
 sw(0x9B738,0x80084B20);sw(0x9B740,0x80084F8C)
 sw(0xA5B80,0x800A5B70);sw(0xA5BA0,0x800BE9A0);sw(0xA5BA4,0)
 r[0xA5BB9]=(n//32)%8;r[0xA5BB6]=255 if n&16 else 1;sh(0xA5C56,0 if n&512 else 2)
 sw(0x9D1A0,(0,1,0x4000,0x4001,0xC000,0xC001,0x8000,0x8001)[n//4%8])
 sh(0xBE9A0,(0,0x4100,0x7300,0xFFFF)[n%4]);sh(0xBE9A2,rand())
 edges=(0,19,20,89,90,160,161,230,231,255)
 r[0xBE9A6]=edges[n//256%10];r[0xBE9A7]=edges[n//25%10]
 sw(0x9D154,0x80150000 if n&128 else 0);sw(0x150000,0);sw(0x150020,1);sw(0x150024,0)
 sw(0xB0CD8,(n//8%4)*0x200);r[0xB0DBF]=n//16%3
 sw(0x9D2A8,n%9)
 for i in range(32):sw(0xA76F0+i*4,1<<(i%16))
 for i in range(9):sw(0x92200+i*4,8 if n&1 else 0x40)
 if n>=2560:
  # Exercise every step of the original special sequence and completion.
  sh(0xBE9A0,0x4100);sh(0xBE9A2,0x7FEF);sw(0x9D1A0,1);sw(0x9D2D4,0)
  for i in range(32):sw(0xA76F0+i*4,0)
  sw(0xA76F0+31*4,0x8000);sw(0xA76F0+3*4,0x10)
 return r

def main():
 ex,o,b=load_overlay();digest=hashlib.sha256(ex[0x3EB04-0xF800:0x3F074-0xF800]).hexdigest()
 assert digest=='c6a27aa96c180ee441866ac86dfc995683fb48c2366dfd864f0bbd03aeb4660b'
 out=['/* Complete original 3EB04 results, real SDK/menu lookup. */','static const uint64_t DAY1_game_input_hashes[]={'];visited=set()
 for n in range(2816):
  r=fixture(ex,n);execute(r,0x8003EB04,visited_pcs=visited)
  out.append(f' UINT64_C(0x{fnv(b"".join(r[a:a+length] for a,length in RANGES)):016X}),')
 out.append('};');header='\n'.join(out)+'\n'
 if '--write-header' in sys.argv:(ROOT/'pc_port/tests/retail_game_input_cases.h').write_text(header)
 else:assert (ROOT/'pc_port/tests/retail_game_input_cases.h').read_text()==header
 for pc in (0x8003EB38,0x8003EBC8,0x8003EC10,0x8003EC30,0x8003ED9C,0x8003EEF0,0x8003EF80,0x8003EFAC,0x8003F010):assert pc in visited,hex(pc)
 print(f'PASS 2816 complete original input cases; SHA256 {digest}')
if __name__=='__main__':main()
