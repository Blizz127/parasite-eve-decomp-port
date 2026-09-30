#!/usr/bin/env python3
"""Original E7 and complete menu constructor, including existing callees."""
import hashlib,struct,sys
from pe_battle_hud_oracle import ROOT,execute
from pe_inventory_menu_oracle import fixture as menu_fixture,RANGES as MENU_RANGES
from pe_scripted_exit_oracle import words
from pe_transition_loader_oracle import fnv
from pe_day2_entry_paths import load
RANGES=tuple(x for x in MENU_RANGES if x[0]!=0xA1870)+((0xA0E00,0x2600),(0xBCF88,0x80))
def fixture(ex,n):
 r,s,_=menu_fixture(ex,dict(entry=0,existing=n%4))
 def sw(a,v):struct.pack_into('<I',r,a&0x1FFFFF,v&0xFFFFFFFF)
 sw(0x8009D300,0x80150000);sw(0x8009CE00,0x801910C8)
 sw(0x800B0CD8,(0,0x1000,0x200,0x80000000)[n//4%4]);sw(0x800BCF88,n*0x12345)
 sw(0x8009D030,n//16%2);sw(0x8009D16C,1)
 struct.pack_into('<H',r,0x150008,(0,0x20,0xFFFF,0xFFDF)[n//32%4])
 r[0xA3060+36*4]=1;r[0xA3061+36*4]=(0,1,2,255)[n//8%4]
 for a,size in RANGES:s[a:a+size]=r[a:a+size]
 return r,s
def main():
 ex=(ROOT/'build/disc1.candidate.exe').read_bytes();assert hashlib.sha1(ex).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
 for a,b,want in ((0x15AF0,0x15BAC,'2d4a817590127635949524c2af473ebc41aa77d79c7b3681fada37ee2657a1e7'),(0x4D18C,0x4D27C,'80a6ad99960fd8029c6bbe703a9dcdfd67d0406957b7478983ae03e73feeca2a')):
  digest=hashlib.sha256(ex[a-0xF800:b-0xF800]).hexdigest();assert digest==want;print(hex(a),digest)
 _,_,script,script_base=load()
 command=next(c for c in script['modules'][1]['commands'] if script_base+c['offset']==0x801910C0)
 assert command['opcode']==0xE7 and command['argc']==0
 seen=set()
 _,s=fixture(ex,0);base=words(s);common=[(i*4,v) for i,v in enumerate(base) if v];patches=[];cases=[]
 for n in range(128):
  r,s=fixture(ex,n);first=len(patches);patches.extend((i*4,v) for i,(v,b) in enumerate(zip(words(s),base)) if v!=b)
  result=execute(r,0x80015AF0,(0x80150100,),visited_pcs=seen)[2]
  h=fnv(b''.join(r[a:a+size] for a,size in RANGES))
  # For first-stage waits, execute the retry with scheduler-supplied next-PC.
  second=0xFFFFFFFF;h2=0
  if result==0:
   struct.pack_into('<I',r,0x9CE00,0x801910C8)
   second=execute(r,0x80015AF0,(0x80150100,),visited_pcs=seen)[2];assert second==1
   h2=fnv(b''.join(r[a:a+size] for a,size in RANGES))
  cases.append((first,len(patches),result,h,second,h2))
 assert {0x80015B34,0x80015B54,0x8004D1FC,0x8004D224}<=seen
 out=['/* Original E7 + menu constructor state. */']
 for name,data in (('ranges',RANGES),('common',common),('patches',patches)):
  out.append(f'static const uint32_t DAY1_exit_menu_{name}[][2]={{');out.extend(f'{{0x{a:X}u,0x{b:X}u}},' for a,b in data);out.append('};')
 out.append('static const struct { unsigned first,end,result; uint64_t hash; unsigned second; uint64_t hash2; } DAY1_exit_menu_cases[]={')
 for a,b,c,d,e,f in cases:out.append(f'{{{a},{b},{c},UINT64_C(0x{d:016X}),0x{e:X}u,UINT64_C(0x{f:016X})}},')
 out.append('};');header='\n'.join(out)+'\n';path=ROOT/'pc_port/tests/retail_exit_menu_cases.h'
 if '--write-header' in sys.argv:path.write_text(header)
 else:assert path.read_text()==header
 print('PASS 128 original E7/menu cases, including retry after yield')
if __name__=='__main__':main()
