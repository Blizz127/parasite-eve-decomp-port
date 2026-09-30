#!/usr/bin/env python3
"""Original menu36 confirm/cancel graph with explicit unresolved I/O frontier."""
import hashlib,struct,sys,itertools
from pe_battle_hud_oracle import ROOT,execute
from pe_exit_menu_draw_oracle import fixture as draw_fixture,RANGES as DRAW_RANGES
from pe_scripted_exit_oracle import words
from pe_transition_loader_oracle import fnv
from pe_card_modal_oracle import fixture as modal_fixture
RANGES=DRAW_RANGES+((0xB0CD8,16),(0xBCF88,128))
CASES=list(itertools.product((0,0x40,0x10000,0x10040),(-1,0,1,2,3),(0,1,4,5),(0,1),(0,8)))
def fixture(ex,n):
 if n>=320:
  r,s,index=modal_fixture(ex,12);execute(r,0x80042848,(index,))
  window=execute(r,0x80062A34,(1,40))[2];assert window
  struct.pack_into('<I',r,0xB0E08,0)
  for a,size in RANGES:s[a:a+size]=r[a:a+size]
  s[0xB0E08:0xB0E0C]=r[0xB0E08:0xB0E0C]
  return r,s,window,(0,1,0x40,0x10000,0x10040,0xFFFFFFFF,0x100,0x20000)[n-320]
 event,selection,flags,busy,state=CASES[n];r,s,node=draw_fixture(ex,0)
 def sw(a,v):struct.pack_into('<I',r,a&0x1FFFFF,v&0xFFFFFFFF)
 window=execute(r,0x80062A34,(1,36))[2]
 sw(node+68,0);sw(node+72,selection);sw(node+116,0xFFFFFFFF)
 sw(0xA1838,busy);sw(0xA1860,0);sw(0x9CFF8,0xBAD);sw(0xB0E08,0)
 for i in range(2):r[0xA0ED4+i*0x418]=flags;r[0xA0ED5+i*0x418]=state if i==n%2 else 0
 for a,size in RANGES:s[a:a+size]=r[a:a+size]
 # The sound package is deliberately absent; sound call identities are checked.
 s[0xB0E08:0xB0E0C]=r[0xB0E08:0xB0E0C]
 return r,s,window,event
def main():
 ex=(ROOT/'build/disc1.candidate.exe').read_bytes();assert hashlib.sha1(ex).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
 digest=hashlib.sha256(ex[0x4D2DC-0xF800:0x4D4A0-0xF800]).hexdigest();assert digest=='d06876052f322de0fa143cfa7b8e3755822ccf3209d89af7ae8006bb8e83374d';print('SHA256',digest)
 _,s,_,_=fixture(ex,0);base=words(s);common=[(i*4,v) for i,v in enumerate(base) if v];patches=[];cases=[];all_seen=set()
 for n in range(328):
  r,s,window,event=fixture(ex,n);first=len(patches);patches.extend((i*4,v) for i,(v,b) in enumerate(zip(words(s),base)) if v!=b)
  seen=set();regs=execute(r,0x8004D2DC if n<320 else 0x8004D030,(window,event),stop_at={0x80072774},visited_pcs=seen);all_seen|=seen
  stopped=int(regs[31]!=0)
  if not stopped:assert regs[2]==1
  h=fnv(b''.join(r[a:a+size] for a,size in RANGES));cases.append((first,len(patches),window,event,stopped,h))
 assert {0x800525EC,0x80052634,0x800526C4,0x8004298C,0x8004DAA4,0x8004D388,0x8004D418}<=all_seen
 out=['/* Original exit-menu input states and I/O frontiers. */']
 for name,data in (('ranges',RANGES),('common',common),('patches',patches)):
  out.append(f'static const uint32_t DAY1_exit_input_{name}[][2]={{');out.extend(f'{{0x{a:X}u,0x{b:X}u}},' for a,b in data);out.append('};')
 out.append('static const struct {unsigned first,end;uint32_t window,event;unsigned stopped;uint64_t hash;} DAY1_exit_input_cases[]={')
 for a,b,c,d,e,f in cases:out.append(f'{{{a},{b},0x{c:X}u,0x{d:X}u,{e},UINT64_C(0x{f:016X})}},')
 out.append('};');header='\n'.join(out)+'\n';path=ROOT/'pc_port/tests/retail_exit_menu_input_cases.h'
 if '--write-header' in sys.argv:path.write_text(header)
 else:assert path.read_text()==header
 print(f'PASS {len(cases)} original input/notice graphs, all three sound paths and I/O frontiers')
if __name__=='__main__':main()
