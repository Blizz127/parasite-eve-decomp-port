#!/usr/bin/env python3
"""Original formatter copy direction, alias behavior and asymmetric return."""
import hashlib,struct,sys
from pe_battle_hud_oracle import ROOT,execute
from pe_transition_loader_oracle import fnv
def main():
 ex=(ROOT/'build/disc1.candidate.exe').read_bytes();assert hashlib.sha1(ex).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
 digest=hashlib.sha256(ex[0x72334-0xF800:0x723A0-0xF800]).hexdigest();assert digest=='9b433f057891fb4d72c9ba2a09d9dc6d95e85cf1f7f8a2e82b40ce03d85f4732';print('SHA256',digest)
 out=['/* Original SDK formatter copy including raw-address alias ordering. */','static const struct {uint32_t dst,src,count,result;uint64_t hash;} DAY1_formatter_copy_cases[]={']
 seen=set()
 for n in range(2048):
  r=bytearray(0x200000);r[0x10000:0x10000+len(ex)-0x800]=ex[0x800:]
  r[0x140000:0x140200]=bytes((i*73+19)&255 for i in range(512))
  src=(0x80000000,0xA0000000)[n//512%2]+0x140040+n%32
  dst=(0x80000000,0xA0000000)[n//1024%2]+0x140040+n//32%32
  count=(0,1,2,8,31,32,127,0xFFFFFFFF,0x80000000)[n//7%9]
  regs=execute(r,0x80072334,(dst,src,count),visited_pcs=seen)
  h=fnv(r[0x140000:0x140200]);out.append(f'{{0x{dst:X}u,0x{src:X}u,0x{count:X}u,0x{regs[2]:X}u,UINT64_C(0x{h:016X})}},')
 assert {0x8007234C,0x80072378,0x80072394}<=seen
 out.append('};');header='\n'.join(out)+'\n';p=ROOT/'pc_port/tests/retail_formatter_copy_cases.h'
 if '--write-header' in sys.argv:p.write_text(header)
 else:assert p.read_text()==header
 print('PASS2048 original copy cases with signed counts and physical aliases')
if __name__=='__main__':main()
