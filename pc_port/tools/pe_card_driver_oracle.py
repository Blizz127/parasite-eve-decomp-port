#!/usr/bin/env python3
"""Original card driver including status dialogs, stopped at operation processor."""
import hashlib,struct,sys
from pe_battle_hud_oracle import ROOT,execute
from pe_exit_menu_draw_oracle import fixture as draw_fixture
from pe_exit_menu_input_oracle import RANGES
from pe_scripted_exit_oracle import words
from pe_transition_loader_oracle import fnv
from pe_card_operation_oracle import STOPS,boundary
def fixture(ex,n):
 r,s,_=draw_fixture(ex,n%48)
 def sw(a,v):struct.pack_into('<I',r,a&0x1FFFFF,v&0xFFFFFFFF)
 sw(0xA1860,1+n%2);sw(0x9CF50,1);execute(r,0x8004DAA4);execute(r,0x80050580,(0,1))
 for i in range(6):sw(0xA1820+i*4,0)
 sw(0xA1838,1);sw(0xB0E08,0)
 sw(0xA1864,(0,1,2,0xFFFFFFFF,0xFFFFFFFE,0x80000000)[n//2%6])
 for i in range(2):
  r[0xA0ED4+i*0x418]=(0,1)[n//12%2]
  r[0xA0EDC+i*0x418]=(1,4,5,255)[n//24%4]
  r[0xA0ED5+i*0x418]=(0,12,8)[n//2%3]
 for a,size in RANGES:s[a:a+size]=r[a:a+size]
 s[0xB0E08:0xB0E0C]=r[0xB0E08:0xB0E0C]
 return r,s
def main():
 ex=(ROOT/'build/disc1.candidate.exe').read_bytes();assert hashlib.sha1(ex).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
 for a,b,want in ((271836, 272240, '362313d6800dbe4c052da87797fcb808704722bb3bf5ad01594681d6546cc730'), (272680, 272740, '0088e8bfd156e6a8d727720e43c98b1139388a1d1c7d2e2eaa75ecb612e8b32f'), (314796, 314836, 'a052df3e9ce1856de28f071530c7056e992864d12f57fe3f5b0f51399d6ede61'), (317912, 317956, '5f84d9053d3a213dbcad23ebe9735af1b739ff18b50446418820a3b996403414'), (318596, 318604, 'e8fae7347ae8681990d0f5661e4142b4d2cb15c451cfdeec3d1a392d2a292c8f')):
  digest=hashlib.sha256(ex[a-0xF800:b-0xF800]).hexdigest();assert digest==want;print(hex(a),digest)
 _,s=fixture(ex,0);base=words(s);common=[(i*4,v) for i,v in enumerate(base) if v];patches=[];cases=[];seen=set()
 for n in range(112):
  r,s=fixture(ex,n);first=len(patches);patches.extend((i*4,v) for i,(v,b) in enumerate(zip(words(s),base)) if v!=b)
  regs=execute(r,0x800425DC if n<96 else 0x80042928,stop_at=STOPS|{0x800726F4},visited_pcs=seen)
  target,mask,args,fifth=boundary(r,regs)
  h=fnv(b''.join(r[a:a+size] for a,size in RANGES));cases.append((first,len(patches),target,mask,args,fifth,h))
 assert {0x800426A4,0x800426BC,0x800426E8,0x8004272C,0x8004D9D8,0x8004CDAC,0x8004DC84,0x80042928}<=seen
 out=['/* Original card driver and completion notice callback states. */']
 for name,data in (('ranges',RANGES),('common',common),('patches',patches)):
  out.append(f'static const uint32_t DAY1_card_driver_{name}[][2]={{');out.extend(f'{{0x{a:X}u,0x{b:X}u}},' for a,b in data);out.append('};')
 out.append('static const struct {unsigned first,end;uint32_t target,mask,args[4],fifth;uint64_t hash;} DAY1_card_driver_cases[]={')
 for a,b,t,m,args,f,h in cases:out.append('{%u,%u,0x%Xu,%u,{%s},0x%Xu,UINT64_C(0x%016X)},'%(a,b,t,m,','.join('0x%Xu'%v for v in args),f,h))
 out.append('};');header='\n'.join(out)+'\n';path=ROOT/'pc_port/tests/retail_card_driver_cases.h'
 if '--write-header' in sys.argv:path.write_text(header)
 else:assert path.read_text()==header
 print('PASS96 original card driver graphs through operation dispatch and16 completion callbacks')
if __name__=='__main__':main()
