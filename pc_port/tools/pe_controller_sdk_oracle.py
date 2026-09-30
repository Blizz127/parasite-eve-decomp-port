#!/usr/bin/env python3
"""Original controller SDK with real installed callback bodies, no providers."""
import hashlib,struct,sys
from pe_m0000i_leaves_oracle import load_overlay
from pe_battle_hud_oracle import execute,ROOT
from pe_transition_loader_oracle import fnv
RANGES=((0x9B738,12),(0xA5B70,0x1E0),(0x150000,0x400),(0x160000,0x100))
SOURCES=((0x800825C0,0x80082778),(0x800828F4,0x800829BC),(0x800835A4,0x800835B0),(0x80083BB8,0x80083C20),(0x80083D04,0x80083D9C),(0x80084B20,0x80084B40),(0x80084F8C,0x80084FB8))
def main():
 ex,o,b=load_overlay();digest=hashlib.sha256(b''.join(ex[a-0x8000F800:z-0x8000F800] for a,z in SOURCES)).hexdigest();print('SHA256',digest);assert digest=='fa58951f31f09efabde626872321aeacd180bc41e70fdcafcec1eea1cc446918'
 out=['/* Complete original controller SDK comparison cases. */','static const struct { uint32_t seed,kind,port,args[2],result; uint64_t hash; } DAY1_controller_sdk_cases[]={']
 visited=set()
 for kind in range(5):
  for seed in range(256):
   r=bytearray(0x200000);r[0x10000:0x10000+len(ex)-0x800]=ex[0x800:]
   for a,n in RANGES:r[a:a+n]=bytes((seed+i*29+(i>>3))&255 for i in range(n))
   def sw(a,v):struct.pack_into('<I',r,a,v&0xffffffff)
   def sh(a,v):struct.pack_into('<H',r,a,v&65535)
   sw(0x9B738,0x80084B20);sw(0x9B740,0x80084F8C)
   for j in range(2):
    p=0xA5B70+j*0xF0
    sw(p,0x80150000);sw(p+0x10,p+0x80000000 if seed&1 else 0)
    sw(p+0x30,0x80160000+j*4);sw(p+0x34,0x10000 if seed&2 else seed&255)
    r[p+0x38]=seed&4;r[p+0x49]=(seed//8)%8;r[p+0x46]=255 if seed&16 else 1
    sh(p+0xE6,0 if seed&32 else (1,2,65535)[seed%3]);r[p+0xE3]=seed%4;r[p+0xE4]=seed&1
    r[0x160000+j*4]=seed&64
   port=(0,1,15,16,31,32,255,0xFFFFFF00)[seed%8]
   args=((1,2,3,4,100,-1,0,5)[seed%8],(-1,0,1,2,3,255,-32768,0x7FFFFFFF)[seed//8%8]) if kind==1 else (0x12340000+seed,0xABCD0100+seed)
   if kind in (2,4):args=(0x800A5B98 if seed&1 else 0x80160000,seed)
   entry=(0x800825C0,0x80082680,0x800828F4,0x8008292C,0x80082974)[kind]
   regs=execute(r,entry,(port,*args),visited_pcs=visited)
   result=0 if kind==4 else regs[2]
   h=fnv(b''.join(r[a:a+n] for a,n in RANGES))
   arr=lambda vs:'{'+','.join(f'0x{x&0xffffffff:X}u' for x in vs)+'}'
   out.append(f' {{{seed},{kind},0x{port:X}u,'+arr(args)+f',0x{result:X}u,UINT64_C(0x{h:016X})'+'},')
 out.append('};');header='\n'.join(out)+'\n'
 if '--write-header' in sys.argv:(ROOT/'pc_port/tests/retail_controller_sdk_cases.h').write_text(header)
 else:assert (ROOT/'pc_port/tests/retail_controller_sdk_cases.h').read_text()==header
 for pc in (0x80082610,0x80082628,0x80082664,0x80082748,0x80083BEC,0x80083D4C,0x800835A4):assert pc in visited,hex(pc)
 print('PASS 1280 controller SDK cases, real slot and busy callbacks')
if __name__=='__main__':main()
