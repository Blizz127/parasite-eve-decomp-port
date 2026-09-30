#!/usr/bin/env python3
"""Original operation dispatch to its first unresolved call, without BIOS replies."""
import hashlib,struct,sys
from pe_battle_hud_oracle import ROOT,execute
from pe_transition_loader_oracle import fnv
STOPS={0x8017FFF0,0x80071A84,0x800727B4,0x80072754,0x80072764,0x80072774}
RANGES=((0xA0ED4,0xA00),(0x92224,4),(0x92230,4),(0x160000,16),(0x9D154,16))
def boundary(r,regs):
 if not regs[31]:return 0,0,[0]*4,0
 word=struct.unpack_from('<I',r,(regs[31]-8)&0x1FFFFF)[0]
 if word>>26==3:target=0x80000000|((word&0x3FFFFFF)<<2)
 else:
  assert word>>26==0 and word&63==9;target=regs[word>>21&31]
 assert target in STOPS
 if target==0x8017FFF0:return target,0,[0]*4,0
 mask=7 if target in (0x80072754,0x80072764) else 1
 fifth=0
 if target==0x80071A84:
  mask=6 if regs[5]==0x80010F60 else 15
  if mask==15:fifth=struct.unpack_from('<I',r,(regs[29]+16)&0x1FFFFF)[0]
 return target,mask,[regs[4+i] if mask&(1<<i) else 0 for i in range(4)],fifth
def fixture(ex,n):
 r=bytearray(0x200000);r[0x10000:0x10000+len(ex)-0x800]=ex[0x800:]
 def sw(a,v):struct.pack_into('<I',r,a&0x1FFFFF,v&0xFFFFFFFF)
 r[0xA0ED4:0xA18D4]=bytes([0xA5])*0xA00
 index=n//64%2;record=0xA0ED4+index*0x418;variant=n//128
 state=n%16 if n%16!=15 or variant%2==0 else 255
 r[record]=(0,1,5,255)[n//16%4];r[record+1]=state
 r[record+2]=(0,1,2,15)[variant%4];r[record+3]=variant%15;r[record+5]=(0,7,14)[variant//4%3];r[record+6]=variant%31
 r[record+8]=(0,1,4,255)[variant//4%4];r[record+11]=variant*3&255
 r[0xA0EDC+(1-index)*0x418]=(0,1,4,255)[variant//16%4]
 struct.pack_into('<H',r,record+20,(0,127,128,129,1023,1024,1025,0x8000,0xFFFF)[variant%9])
 sw(record+12,0xF1000000+n);sw(record+24,0x80170000+n*4)
 for i in range(15):r[record+i*0x44+29]=(i+variant)%3;r[record+i*0x44+69]=(i+variant)&255
 sw(0x92224,0x80161000);sw(0x92230,0x80160000);sw(0xA1704,n*0x10203)
 for i in range(4):sw(0x9D154+i*4,0)
 sw(0xA185C,0);sw(0x9CFFC,0);sw(0x9CF44,0)
 return r,index
def main():
 ex=(ROOT/'build/disc1.candidate.exe').read_bytes();assert hashlib.sha1(ex).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
 assert hashlib.sha256(ex[0x41108-0xF800:0x42020-0xF800]).hexdigest()=='a9033ae5110fc414ffe13fa229fc7ffe9bc17a9e563eae68fc8e49b712fd3c6a'
 cases=[];seen=set()
 for n in range(8192):
  r,index=fixture(ex,n);regs=execute(r,0x80041108,(index,),stop_at=STOPS,visited_pcs=seen)
  target,mask,args,fifth=boundary(r,regs);h=fnv(b''.join(r[a:a+size] for a,size in RANGES));cases.append((target,mask,args,fifth,h))
 assert {0x800411D0,0x80041248,0x80041260,0x80041300,0x80041628,0x800417AC,0x80041800,0x80041A7C,0x80041B3C,0x80041C30}<=seen
 out=['/* Original operation effects and known call arguments; stack addresses unresolved. */','static const struct {uint32_t target,mask,args[4],fifth;uint64_t hash;} DAY1_card_operation_cases[]={']
 for t,m,args,f,h in cases:out.append('{0x%Xu,%u,{%s},0x%Xu,UINT64_C(0x%016X)},'%(t,m,','.join('0x%Xu'%v for v in args),f,h))
 out.append('};');header='\n'.join(out)+'\n';p=ROOT/'pc_port/tests/retail_card_operation_cases.h'
 if '--write-header' in sys.argv:p.write_text(header)
 else:assert p.read_text()==header
 print('PASS8192 original operation dispatch paths, known arguments and first unresolved calls')
if __name__=='__main__':main()
