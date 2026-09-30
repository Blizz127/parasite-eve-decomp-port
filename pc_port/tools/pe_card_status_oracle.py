#!/usr/bin/env python3
"""Original card-status machine through returning paths or first BIOS call."""
import hashlib,struct,sys
from pe_battle_hud_oracle import ROOT,execute
from pe_transition_loader_oracle import fnv
RANGES=((0xA0ED4,0xA00),(0x9D154,16))
def fixture(ex,n):
 r=bytearray(0x200000);r[0x10000:0x10000+len(ex)-0x800]=ex[0x800:]
 def sw(a,v):struct.pack_into('<I',r,a&0x1FFFFF,v&0xFFFFFFFF)
 r[0xA0ED4:0xA18D4]=bytes([0xA5])*0xA00
 index=n//2048;record=0xA0ED4+index*0x418
 r[record]=(0,1,4,255)[n//512%4];r[record+8]=(0,1,2,3,4,5,255,3)[n%8]
 r[record+1]=(0,12,8)[n//32%3]
 mask=n//8%64
 for i in range(6):sw(0xA1820+4*i,0x12345678 if mask&(1<<i) else 0)
 sw(0xA1838,n//128%2);sw(0xA183C,n//64%3)
 sw(0xA1840,(0,1,2,0xFFFFFFFF,0x80000000,0x7FFFFFFF,0x80000001,99)[n//256%8])
 sw(0x9D154,0x80158000 if n//16%2 else 0);sw(0x9D158,0x12345678);sw(0x9D15C,0x80159000);sw(0x9D160,0xDEADBEEF)
 sw(0x158000,0);sw(0x158020,1);sw(0x158024,36)
 for i in range(8):sw(0xBCDA8+4*i,0xF1000000+i)
 return r,index
def main():
 ex=(ROOT/'build/disc1.candidate.exe').read_bytes();assert hashlib.sha1(ex).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
 for a,b,want in ((263588, 264628, 'f8cc418dca7789e5dc2c20647b90ce8b6142e02a6edd5b4efa7ce4c73dab45d8'), (316576, 316612, '626977939ef0a819adc872eb39c2fce48fcac577a4182e799b1679c0653866cc')):
  digest=hashlib.sha256(ex[a-0xF800:b-0xF800]).hexdigest();assert digest==want;print(hex(a),digest)
 cases=[];seen=set()
 for n in range(4096):
  r,index=fixture(ex,n);regs=execute(r,0x800405A4,(index,),stop_at={0x800726F4,0x8007DD44,0x8007DD54,0x8007DD74},visited_pcs=seen)
  stopped=int(regs[31]!=0);h=fnv(b''.join(r[a:a+size] for a,size in RANGES));cases.append((stopped,regs[4] if stopped else 0,h))
 assert {0x80040604,0x8004060C,0x8004075C,0x80040820,0x800408DC,0x8004D4A0,0x8004298C,0x80040928}<=seen
 out=['/* Original card status paths; stopped argument is first TestEvent handle. */','static const struct {unsigned stopped;uint32_t argument;uint64_t hash;} DAY1_card_status_cases[]={']
 out.extend(f'{{{a},0x{b:X}u,UINT64_C(0x{c:016X})}},' for a,b,c in cases);out.append('};')
 header='\n'.join(out)+'\n';path=ROOT/'pc_port/tests/retail_card_status_cases.h'
 if '--write-header' in sys.argv:path.write_text(header)
 else:assert path.read_text()==header
 print('PASS4096 original card status cases; returning paths and exact first BIOS boundary')
if __name__=='__main__':main()
