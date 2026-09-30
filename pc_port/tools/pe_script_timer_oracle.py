#!/usr/bin/env python3
"""Original D2 timer read + D3 signed time components, including aliases."""
import hashlib,struct,sys
from pe_battle_hud_oracle import ROOT,execute
from pe_transition_loader_oracle import fnv
VALUES=(0,1,59,60,61,3599,3600,3601,215999,216000,216001,219660,0x7FFFFFFF,0x80000000,0xFFFFFFFF,-60,-3600,-216000)
def main():
 ex=(ROOT/'build/disc1.candidate.exe').read_bytes();assert hashlib.sha1(ex).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
 digest=hashlib.sha256(ex[0x19DB8-0xF800:0x19F04-0xF800]).hexdigest();assert digest=='f684e1f70afe44e8a2195316e020111fecf4c6ad3f99e2e1e3b0ad297662effe'
 rows=[]
 for n in range(2048):
  ram=bytearray(0x200000);ram[0x10000:0x10000+len(ex)-0x800]=ex[0x800:]
  def sw(a,v):struct.pack_into('<I',ram,a&0x1FFFFF,v&0xFFFFFFFF)
  for i in range(0,128,4):sw(0x80150000+i,0xA5A50000+i)
  value=VALUES[n%len(VALUES)] if n<1024 else n*2654435761
  index=n%12
  sw(0x800A76A4+index*12,value);sw(0x80150040,index)
  sw(0x80150000,0x80150040);sw(0x80150004,0x80150044)
  assert execute(ram,0x80019DB8,(0x80150000,))[2]==1
  sw(0x80150000,0x80150044)
  for j in range(3):
   alias=n//18%8
   dest=0x80150048+j*4
   if alias==j+1:dest=0x80150044
   if alias==4:dest=0x80150048
   if alias==5 and j==2:dest=0x80150048
   if alias==6 and j==0:dest=0x80150004
   if alias==7 and j==1:dest=0x80150008
   sw(0x80150004+j*4,dest)
  assert execute(ram,0x80019DF4,(0x80150000,))[2]==1
  rows.append(f'UINT64_C(0x{fnv(ram[0x150000:0x150080]):016X}),')
 header='/* Original D2/D3 ordered-state cases. */\nstatic const uint64_t DAY1_script_timer_cases[]={\n'+'\n'.join(rows)+'\n};\n'
 path=ROOT/'pc_port/tests/retail_script_timer_cases.h'
 if '--write-header' in sys.argv:path.write_text(header)
 else:assert path.read_text()==header
 print('SHA256',digest);print('PASS 2048 original D2/D3 cases')
if __name__=='__main__':main()
