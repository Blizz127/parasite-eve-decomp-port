#!/usr/bin/env python3
"""Original card dispatcher through formatting to first BIOS/cleanup boundary."""
import hashlib,struct,sys
from pe_battle_hud_oracle import ROOT,execute
from pe_card_operation_oracle import fixture
from pe_transition_loader_oracle import fnv
RANGES=((0xA0ED4,0xA00),(0x92224,4),(0x92230,4),(0x160000,16),(0x9D154,16),(0x9EE70,96),(0x1FEA00,0x800))
STATIC=((0x10ED4,160),(0x1161C,220),(0x94528,12))
STOPS={0x80042228,0x8004D5CC,0x800727B4,0x80072754,0x80072764,0x80072774,0x80072734,0x80072784,0x80072314,0x80072324}
def main():
 ex=(ROOT/'build/disc1.candidate.exe').read_bytes();assert hashlib.sha1(ex).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
 assert hashlib.sha256(ex[0x41108-0xF800:0x42020-0xF800]).hexdigest()=='a9033ae5110fc414ffe13fa229fc7ffe9bc17a9e563eae68fc8e49b712fd3c6a'
 common=None;patches=[];cases=[];seen=set()
 for n in range(1280):
  r,index=fixture(ex,n%128+(n//128)*384)
  struct.pack_into('<I',r,0x92224,0x80010ED4)
  r[0x9EE70:0x9EED0]=bytes(96);r[0x1FEA00:0x1FF200]=bytes(0x800)
  stack=0x801FEE00 if n&128 else 0x801FF000
  entry=0x80040F80 if n>=1024 else 0x80041108
  argument=0x800A0ED4+index*0x418 if n>=1024 else index
  if n>=1024:
   record=argument&0x1FFFFF;r[record+1]=9 if n&2 else 12
   struct.pack_into('<I',r,record+12,4 if n&1 else 0xFFFFFFFF)
   struct.pack_into('<I',r,0xA185C,1 if n&4 else 0)
  seed={a:struct.unpack_from('<I',r,a)[0] for start,size in RANGES+STATIC for a in range(start,start+size,4)}
  if common is None:common=seed
  first=len(patches);patches.extend((a,v) for a,v in seed.items() if v!=common[a])
  initial={16+i:0x13570000+i*0x1111 for i in range(8)};initial[29]=stack;initial[7]=0x13579BDF
  regs=execute(r,entry,(argument,),initial_regs=initial,stop_at=STOPS,visited_pcs=seen)
  target=mask=0;args=[0]*4
  if regs[31]:
   word=struct.unpack_from('<I',r,(regs[31]-8)&0x1FFFFF)[0];assert word>>26==3
   target=0x80000000|((word&0x3FFFFFF)<<2);assert target in STOPS
   mask=7 if target in (0x80072754,0x80072764,0x80072324) else 3 if target in (0x80072734,0x800727B4) else 1
   args=[regs[4+i] if mask&(1<<i) else 0 for i in range(4)]
  if target==0x80042228:mask=0;args=[0]*4
  h=fnv(b''.join(r[a:a+size] for a,size in RANGES));cases.append((entry,argument,stack,first,len(patches),target,mask,args,h))
 assert {0x80041284,0x800416E0,0x80041840,0x80041910,0x800419DC,0x80041EA4,0x80072334,0x80041040,0x8004105C,0x800721A8}<=seen
 out=['/* Original dispatcher plus formatter; guest stack and first BIOS effects. */','static const uint32_t DAY1_card_operation_frame_ranges[][2]={']
 out += [f'{{0x{a:X}u,0x{size:X}u}},' for a,size in RANGES];out+=['};','static const uint32_t DAY1_card_operation_frame_common[][2]={']
 out += [f'{{0x{a:X}u,0x{v:X}u}},' for a,v in common.items() if v];out+=['};','static const uint32_t DAY1_card_operation_frame_patches[][2]={']
 out += [f'{{0x{a:X}u,0x{v:X}u}},' for a,v in patches];out+=['};','static const struct {uint32_t entry,argument,stack,first,end,target,mask,args[4];uint64_t hash;} DAY1_card_operation_frame_cases[]={']
 for entry,argument,stack,first,end,target,mask,args,h in cases:
  a=','.join(f'0x{v:X}u' for v in args);out.append(f'{{0x{entry:X}u,0x{argument:X}u,0x{stack:X}u,{first}u,{end}u,0x{target:X}u,{mask}u,{{{a}}},UINT64_C(0x{h:016X})}},')
 out.append('};');header='\n'.join(out)+'\n';p=ROOT/'pc_port/tests/retail_card_operation_frame_cases.h'
 if '--write-header' in sys.argv:p.write_text(header)
 else:assert p.read_text()==header
 print('PASS1280 original framed card dispatch/cleanup executions through first unresolved call')
if __name__=='__main__':main()
