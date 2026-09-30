#!/usr/bin/env python3
"""Original card cleanup, callbacks and selection restoration."""
import hashlib,struct,sys
from pe_battle_hud_oracle import ROOT,execute
from pe_exit_menu_draw_oracle import fixture as draw_fixture
from pe_exit_menu_input_oracle import RANGES
from pe_card_operation_oracle import STOPS,boundary
from pe_scripted_exit_oracle import words
from pe_transition_loader_oracle import fnv
UNKNOWN=0x8017FFF0
def fixture(ex,n):
 r,s,_=draw_fixture(ex,n%48)
 def sw(a,v):struct.pack_into('<I',r,a&0x1FFFFF,v&0xFFFFFFFF)
 for ident in (37,38,39,40,41,31,63):execute(r,0x80062D2C,(ident,0,0,0))
 if n>=192 and n%4==0:execute(r,0x80062F3C,(36,))
 if n>=192:
  node=struct.unpack_from('<I',r,0x9D154)[0]
  while node:
   sw(node+68,0);node=struct.unpack_from('<I',r,node&0x1FFFFF)[0]
 if n>=192 and n%4==1:
  w=execute(r,0x80062D2C,(60,0,0,0))[2];sw(w+68,1)
 index=n%2;record=0x800A0ED4+index*0x418
 sw(record+12,4 if n//2%2 else 0xFFFFFFFF);r[(record+1)&0x1FFFFF]=9 if n//8%2 else 12
 sw(0xA185C,n//4%2);sw(0xA1854,0x80158000);sw(0xA1838,1)
 sw(0xA1860,index+1);sw(0xA1868,7);sw(0xB0E08,0)
 sw(0x9CF44,index if (n//96%2 if n<192 else n%8!=0) else 1-index)
 sw(0x9CFFC,(0,0x80042910,0x80042928,0x8005C488,0x80062F9C,UNKNOWN)[n//16%6])
 for a,size in RANGES:s[a:a+size]=r[a:a+size]
 s[0xB0E08:0xB0E0C]=r[0xB0E08:0xB0E0C]
 s[0x92224:0x92228]=r[0x92224:0x92228]
 return r,s,record if n<192 else index
def main():
 ex=(ROOT/'build/disc1.candidate.exe').read_bytes();assert hashlib.sha1(ex).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
 for a,b,want in ((0x40F80,0x41108,'4b7854a10db5e134b7377f736d2f4207b72adca48cf9a96d766e19ca48c14212'),(0x42228,0x42264,'1c958adc13bb55f6474559dd7dfb18594affde554547a516f78c4a03ab8e620c'),(0x4D5CC,0x4D690,'5fb97d0fd44a3474d2bcded181f1973f74cbe470adcc406875b1e60645e3dbaf')):
  digest=hashlib.sha256(ex[a-0xF800:b-0xF800]).hexdigest();assert digest==want;print(hex(a),digest)
 _,s,_=fixture(ex,0);base=words(s);common=[(i*4,v) for i,v in enumerate(base) if v];patches=[];cases=[];seen=set()
 for n in range(256):
  r,s,arg=fixture(ex,n);first=len(patches);patches.extend((i*4,v) for i,(v,b) in enumerate(zip(words(s),base)) if v!=b)
  regs=execute(r,0x80040F80 if n<192 else 0x8004D5CC,(arg,),stop_at=STOPS,visited_pcs=seen)
  target,mask,args,fifth=boundary(r,regs);h=fnv(b''.join(r[a:a+size] for a,size in RANGES));cases.append((first,len(patches),arg,target,mask,args,fifth,h))
 assert {0x80042228,0x8004D5CC,0x8004D610,0x8004D624,0x8004D660,0x8004D670,0x800410C8}<=seen
 out=['/* Original card cleanup and notice callback paths. */']
 for name,data in (('ranges',RANGES),('common',common),('patches',patches)):
  out.append(f'static const uint32_t DAY1_card_cleanup_{name}[][2]={{');out.extend(f'{{0x{a:X}u,0x{b:X}u}},' for a,b in data);out.append('};')
 out.append('static const struct {unsigned first,end;uint32_t argument,target,mask,args[4],fifth;uint64_t hash;} DAY1_card_cleanup_cases[]={')
 for a,b,arg,t,m,args,f,h in cases:out.append('{%u,%u,0x%Xu,0x%Xu,%u,{%s},0x%Xu,UINT64_C(0x%016X)},'%(a,b,arg,t,m,','.join('0x%Xu'%v for v in args),f,h))
 out.append('};');header='\n'.join(out)+'\n';path=ROOT/'pc_port/tests/retail_card_cleanup_cases.h'
 if '--write-header' in sys.argv:path.write_text(header)
 else:assert path.read_text()==header
 print('PASS192 original cleanup and64 direct window/callback graphs')
if __name__=='__main__':main()
