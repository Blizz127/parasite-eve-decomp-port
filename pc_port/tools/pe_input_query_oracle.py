#!/usr/bin/env python3
"""Complete original opcode11, including held-counter/GTE and argument aliases."""
import hashlib,struct,sys
from pe_battle_hud_oracle import ROOT,execute
from pe_transition_loader_oracle import fnv
MASKS=tuple(1<<i for i in range(32))+(0,0xFFFFFFFF,0x7FFFFFFF,0x80000001,0xC0000000,0x40000001,0x01000100,0x80150040)
def main():
 ex=(ROOT/'build/disc1.candidate.exe').read_bytes();assert hashlib.sha1(ex).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
 digest=hashlib.sha256(ex[0x130B4-0xF800:0x131E8-0xF800]).hexdigest();assert digest=='41ee2a1659cb0f8d1905ad71fb9f8b99408d6647c1a55127e789122ae7e25bbd'
 rows=[]
 for n in range(2560):
  ram=bytearray(0x200000);ram[0x10000:0x10000+len(ex)-0x800]=ex[0x800:]
  def sw(a,v):struct.pack_into('<I',ram,a&0x1FFFFF,v&0xFFFFFFFF)
  for i in range(0,128,4):sw(0x80150000+i,0xA5A50000+i)
  for i in range(33):sw(0x800A776C+i*4,0x12340000+i*0x101+n)
  code=3 if n<1280 else (n//40)%8
  mask=MASKS[n%40];maskptr=0x80150044
  if n//640%2:maskptr=0x80150000;mask=0x80150040
  flags=(mask,~mask,0,0xFFFFFFFF)[n//40%4]
  sw(0x8009D26C,flags);sw(0x8009D1F4,flags^0x55555555);sw(0x8009D1E4,flags^0xAAAAAAAA)
  sw(0x80150040,code);sw(0x80150044,mask)
  sw(0x80150000,0x80150040);sw(0x80150004,maskptr)
  dest=(0x80150048,maskptr,0x80150000,0x80150004,0x80150008,0x8009D26C,0x800A776C,0x800A77EC)[n//160%8]
  sw(0x80150008,dest)
  final={};assert execute(ram,0x800130B4,(0x80150000,),initial_cop_data={30:0x80000000,31:1},initial_cop_control={31:0xA5A51234},final_gte=final)[2]==1
  assert final['data'][:30]==[0]*30 and final['control']==[0]*31+[0xA5A51234]
  state=ram[0x150000:0x150080]+ram[0xA776C:0xA77F0]+ram[0x9D26C:0x9D270]+ram[0x9D1F4:0x9D1F8]+ram[0x9D1E4:0x9D1E8]
  rows.append(f'{{UINT64_C(0x{fnv(state):016X}),0x{final["data"][30]:08X}u,{final["data"][31]}u}},')
 header='/* Complete original opcode11 state: RAM hash, LZCS, LZCR. */\nstatic const struct { uint64_t hash; uint32_t lzcs,lzcr; } DAY1_input_query_cases[]={\n'+'\n'.join(rows)+'\n};\n'
 path=ROOT/'pc_port/tests/retail_input_query_cases.h'
 if '--write-header' in sys.argv:path.write_text(header)
 else:assert path.read_text()==header
 print('SHA256',digest);print('PASS 2560 original input query cases; GTE and aliased RAM effects')
if __name__=='__main__':main()
