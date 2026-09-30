#!/usr/bin/env python3
"""Original dispatcher, registered game callback, RNG and four timer records."""
import hashlib,struct,sys
from pe_m0000i_leaves_oracle import load_overlay
from pe_battle_hud_oracle import execute,ROOT
from pe_transition_loader_oracle import fnv
RANGES=((0x70DCC,0x84),(0x9568C,36),(0xA76A0,48),(0x9D1A0,4),(0xB0DB8,8))
def main():
 ex,o,b=load_overlay();spans=((0x7440C,0x74478),(0x3E91C,0x3E944),(0x36F7C,0x3708C));digest=hashlib.sha256(b''.join(ex[a-0xF800:z-0xF800] for a,z in spans)).hexdigest();print('SHA256',digest);assert digest=='8b623f46317d202830636347024c7c3fb20712bc247b0e6869521c6c5a7c019f'
 code=ex[0x70DCC-0xF800:0x70E04-0xF800]
 out=['/* Complete original VBlank dispatcher/callback fixtures. */','static const uint8_t DAY1_vblank_rng_code[]={'+','.join(str(x) for x in code)+'};','static const uint64_t DAY1_vblank_hashes[]={'];visited=set()
 for n in range(512):
  r=bytearray(0x200000);r[0x10000:0x10000+len(ex)-0x800]=ex[0x800:]
  def sw(a,v):struct.pack_into('<I',r,a,v&0xffffffff)
  for a,length in RANGES:
   for i in range(0,length,4):sw(a+i,(n*101+i*2654435761)&0xffffffff)
  r[0x70DCC:0x70E04]=code;sw(0x70E04,(n%17)*4);sw(0x70E08,((n+3)%17)*4)
  for i in range(8):sw(0x9568C+i*4,0x8003E91C if n>>i&1 else 0)
  sw(0x956AC,0xFFFFFFFF if n&1 else n)
  sw(0x9D1A0,(0,1,0x40,0x41)[n//16%4]);sw(0xB0DB8,((n//64)%4)<<16);sw(0xB0DBC,(0,1,0xFFFF)[n//128%3])
  for i in range(4):sw(0xA76A0+i*12,(n//4+i)%8);sw(0xA76A4+i*12,(0,1,0xFFFFFFFF,0x7FFFFFFF)[(n+i)%4])
  execute(r,0x8007440C,visited_pcs=visited,instruction_budget=100000)
  out.append(f' UINT64_C(0x{fnv(b"".join(r[a:a+z] for a,z in RANGES)):016X}),')
 out.append('};');header='\n'.join(out)+'\n'
 if '--write-header' in sys.argv:(ROOT/'pc_port/tests/retail_vblank_cases.h').write_text(header)
 else:assert (ROOT/'pc_port/tests/retail_vblank_cases.h').read_text()==header
 assert 0x80036FF0 in visited and 0x8003702C in visited
 print('PASS 512 complete original VBlank cases')
if __name__=='__main__':main()
