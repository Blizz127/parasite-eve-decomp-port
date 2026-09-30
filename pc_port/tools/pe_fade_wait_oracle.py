#!/usr/bin/env python3
"""Original fade starts, packet/timer ticks and script wait resumption."""
import hashlib,struct,sys
from pe_battle_hud_oracle import ROOT,execute
from pe_transition_loader_oracle import fnv
from pe_day2_entry_paths import load
RANGES=((0xBCF88,128),(0x150000,160),(0x9CE00,4))
def main():
 ex=(ROOT/'build/disc1.candidate.exe').read_bytes();assert hashlib.sha1(ex).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
 spans=((0x18EB4,0x18F0C),(0x19410,0x19450),(0x66B60,0x66BD8),(0x66C7C,0x66CE8),(0x68E24,0x6914C))
 hashes=[hashlib.sha256(ex[a-0xF800:b-0xF800]).hexdigest() for a,b in spans]
 assert hashes==['a1c1463de313f9423af6c3208d5e7c53f3f1b41175da6ea071edeb0abd9c315e','ab0d665bb7e9e59e7726bede5108afe7c5cd444b614ad80b4c8e072c2bfe1ce8','862ff6185fcd6ef374b96fb285a567cf59af62e7e30bdf89ad7f41f6ca0ee45e','ce26b53b3c92cae7ffdaa7c393586b80b323a6f53fc16853640d60cafba82ef0','bee869b9d9857c6d707e68201a813590713350f965c963eedde3c1673aa84267']
 _,_,script,base=load()
 commands={base+c['offset']:c for c in script['modules'][1]['commands']}
 for pc,opcode in ((0x8018F5B8,0x86),(0x801910C8,0x85)):
  c=commands[pc];assert c['opcode']==opcode and c['modes']==[0] and c['args']==[60]
 assert commands[0x801910D4]['opcode']==0x9C and commands[0x801910D4]['argc']==0
 rows=[]
 def seed(n):
  r=bytearray(0x200000);r[0x10000:0x10000+len(ex)-0x800]=ex[0x800:]
  def sw(a,v):struct.pack_into('<I',r,a&0x1FFFFF,v&0xFFFFFFFF)
  for a,size in RANGES:r[a:a+size]=bytes(size)
  for i in range(6):struct.pack_into('<H',r,0xBCFE8+(i if i<3 else i+1)*2,(0x7FFF,0x8000,255,0x8000,0x7FFF,0)[i])
  sw(0x8009CDDC,n&1);sw(0x800B0E38,0x80150000);sw(0x800B0E3C,0x80150020)
  sw(0x8009D300,0x80150080);sw(0x80150060,0x80150064)
  sw(0x8015000C,0xA5FFFFFF);sw(0x8015002C,0x5AFFFFFF)
  return r,sw
 def record(r,result):
  h=fnv(b''.join(r[a:a+s] for a,s in RANGES));rows.append(f'{{UINT64_C(0x{h:016X}),{result}u}},')
 for n in range(512):
  r,sw=seed(n);r[0xBCFEE]=n%8;r[0xBCFEF]=n//8%4
  struct.pack_into('<H',r,0xBCFF6,(0,1,2,60,32768,65535)[n//8%6]);struct.pack_into('<H',r,0xBCFF8,(0,1,59,60,32768,65535)[n//48%6])
  record(r,execute(r,0x80068E24)[2])
 for n in range(16):
  r,sw=seed(n);duration=(0,1,2,60)[n//2%4];sw(0x80150064,duration)
  record(r,execute(r,0x80018EB4 if n<8 else 0x80018EE0,(0x80150060,))[2])
  for frame in range(max(1,duration)+1):
   sw(0x8009CE00,0x801910DC);sw(0x80150090,0xBAD)
   result=execute(r,0x80019410,(0x80150060,))[2];record(r,result)
   assert result==(1 if frame==max(1,duration) else 0)
   if result:break
   sw(0x8009CDDC,frame&1);sw(0x8015000C+(frame&1)*32,0xFFFFFF)
   record(r,execute(r,0x80068E24)[2])
 header='/* Original fade/tick/wait RAM hashes and returns. */\nstatic const struct { uint64_t hash; uint32_t result; } DAY1_fade_wait_cases[]={\n'+'\n'.join(rows)+'\n};\n'
 path=ROOT/'pc_port/tests/retail_fade_wait_cases.h'
 if '--write-header' in sys.argv:path.write_text(header)
 else:assert path.read_text()==header
 print('spans',list(zip(spans,hashes)));print(f'PASS 512 original fade ticks and16 start/wait sequences; {len(rows)} checkpoints')
if __name__=='__main__':main()
