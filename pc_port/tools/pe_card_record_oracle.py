#!/usr/bin/env python3
"""Original card-record helpers and cleanup up to unresolved BIOS close."""
import hashlib,struct,sys
from pe_battle_hud_oracle import ROOT,execute
from pe_transition_loader_oracle import fnv
RANGES=((0xA0ED4,0xA00),(0x9D15C,8))
def main():
 ex=(ROOT/'build/disc1.candidate.exe').read_bytes();assert hashlib.sha1(ex).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
 for a,b,want in ((0x428C4,0x428D4,'bfb036fea00c9c38001077ffe409293f3a4017d9904ba3d1c50905b0fd35f6e1'),(0x42910,0x42928,'786f2a7bdec34f7abb8141329230742f79daf7503cf309ce8968f4ec6da0b511'),(0x4298C,0x42B38,'4c99b0afadcf5944072edd81116e49c1425c56f53deb04b3b9628a2056c96e3f'),(0x62CD0,0x62CE4,'758d39c1104fe870f04c38525f79d655e114d7795565417a360d7fbb5d94536b')):
  digest=hashlib.sha256(ex[a-0xF800:b-0xF800]).hexdigest();assert digest==want;print(hex(a),hex(b),digest)
 rows=[]
 for n in range(256):
  r=bytearray(0x200000);r[0x10000:0x10000+len(ex)-0x800]=ex[0x800:]
  def sw(a,v):struct.pack_into('<I',r,a&0x1FFFFF,v&0xFFFFFFFF)
  r[0xA0ED4:0xA18D4]=bytes([0xA5])*0xA00
  index=n&1;record=0xA0ED4+index*0x418;other=0xA0ED4+(1-index)*0x418
  r[record+1]=n;r[other+1]=(n*5)&255
  sw(0x9D15C,0x80140000+n*4);sw(0x9D160,0xDEADBEEF);sw(0xA1860,n*0x1020304)
  mode=n*2654435761&0xFFFFFFFF
  execute(r,0x8004298C,(index,mode))
  active=execute(r,0x80042AD8,(index,))[2];selected=execute(r,0x800428C4)[2]
  execute(r,0x80042910);busy=execute(r,0x80042B28)[2]
  before=fnv(b''.join(r[a:a+s] for a,s in RANGES))
  regs=execute(r,0x80042A10,stop_at={0x80072774});stopped=int(regs[31]!=0)
  after=fnv(b''.join(r[a:a+s] for a,s in RANGES))
  rows.append(f'{{{active}u,0x{selected:X}u,0x{busy:X}u,UINT64_C(0x{before:016X}),UINT64_C(0x{after:016X}),{stopped}}},')
 header='/* Original record helpers and BIOS-close frontier. */\nstatic const struct {uint32_t active,selected,busy; uint64_t before,after; unsigned stopped;} DAY1_card_record_cases[]={\n'+'\n'.join(rows)+'\n};\n'
 path=ROOT/'pc_port/tests/retail_card_record_cases.h'
 if '--write-header' in sys.argv:path.write_text(header)
 else:assert path.read_text()==header
 print('PASS 256 original record chains and cleanup return/stop paths')
if __name__=='__main__':main()
