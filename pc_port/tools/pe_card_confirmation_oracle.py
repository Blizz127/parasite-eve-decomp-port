#!/usr/bin/env python3
"""Original confirmation dialog, delayed record transition and progress drawing."""
import hashlib,struct,sys
from pe_battle_hud_oracle import ROOT,execute
from pe_exit_menu_input_oracle import RANGES
from pe_exit_menu_draw_oracle import fixture as draw_fixture
from pe_scripted_exit_oracle import words
from pe_transition_loader_oracle import fnv
N=128
def fixture(ex,n):
 r,s,_=draw_fixture(ex,n%48)
 def sw(a,v):struct.pack_into('<I',r,a&0x1FFFFF,v&0xFFFFFFFF)
 sw(0x9CF50,1);sw(0xA1860,1+n%2);sw(0xB0E08,0)
 for ident,length in ((0x47,(n//4%4)*12),(0x48,(n//4%4)*12),(0x4B,(n//16%4)*12)):
  ptr=execute(r,0x8005DC4C,(ident,))[2]&0x1FFFFF
  r[ptr:ptr+length+1]=bytes([32])*length+bytes([255])
 execute(r,0x8004DAA4)
 window=execute(r,0x80062A34,(1,42))[2];listnode=execute(r,0x80062A20,(window,0))[2]
 sw(listnode+68,0);sw(listnode+72,n%2)
 # Progress input is deliberately independent of the deferred card operation.
 sw(0xA1854,0 if n//4%8==0 else 0x80158000);sw(0x80158014,(n%3)*0x8000)
 sw(0xA1858,(0,1023,1024,8192,0xFFFFFFFF,0x80000000,0x7FFFFFFF,2048)[n//4%8])
 for a,size in RANGES:s[a:a+size]=r[a:a+size]
 s[0xB0E08:0xB0E0C]=r[0xB0E08:0xB0E0C];s[0x158000:0x158020]=r[0x158000:0x158020]
 return r,s,window,(0x10000,0x10000,0x40,0x10040)[n%4]
def main():
 ex=(ROOT/'build/disc1.candidate.exe').read_bytes();assert hashlib.sha1(ex).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
 for a,b,want in (
  (0x42464,0x424B4,'2f36c1a49622eb717df9384857344051e1c1b62805c202e711453fefe94aa65f'),
  (0x428D4,0x42910,'be4a2386c8f02ce7763cdcd7d6c2faf671ec204ccb2fed237ae189dc1fa4add9'),
  (0x42B50,0x42B6C,'f1d4cafb97144b261c444d5a29e5f2457e89a61cec555bd5cb8a150539b0b274'),
  (0x4CE28,0x4D024,'c3de5ab0a5128340d5c650c7d8f672daf4a87de276b3ce882af1c3bdc9347a6a'),
  (0x4DA04,0x4DAA4,'78064bafdecda69b22e864f09347c4da45b4c10720cda0f8e1af50f69ce9c46c'),
  (0x50580,0x50618,'388ea5e89ff1d52fc4be1e4562ce1d86b9c46aca4eb145eaa300797a4ee18e81')):
  digest=hashlib.sha256(ex[a-0xF800:b-0xF800]).hexdigest();assert digest==want;print(hex(a),digest)
 _,s,_,_=fixture(ex,0);base=words(s);common=[(i*4,v) for i,v in enumerate(base) if v];patches=[];cases=[];seen=set()
 for n in range(N):
  r,s,window,event=fixture(ex,n);first=len(patches);patches.extend((i*4,v) for i,(v,b) in enumerate(zip(words(s),base)) if v!=b)
  regs=execute(r,0x80044E98,(window,event),visited_pcs=seen);assert regs[2]==1
  if n>=64:
   for i in range(3):execute(r,0x80042B6C,visited_pcs=seen)
   if n%4==0:
    if n//4%3:struct.pack_into('<I',r,0x9D000,0x41)
    execute(r,0x8004DA04,visited_pcs=seen)
    assert execute(r,0x8004DA9C,(0xDEAD,0x10040))[2]==1
   else:execute(r,0x8004CFD4,visited_pcs=seen,instruction_budget=1000000)
  h=fnv(b''.join(r[a:a+size] for a,size in RANGES));cases.append((first,len(patches),window,event,h))
 assert {0x80050580,0x8004CE28,0x800428D4,0x8004DA04,0x80042464,0x8004CFD4}<=seen
 out=['/* Original card confirmation, deferred callback and drawing states. */']
 for name,data in (('ranges',RANGES),('common',common),('patches',patches)):
  out.append(f'static const uint32_t DAY1_card_confirm_{name}[][2]={{');out.extend(f'{{0x{a:X}u,0x{b:X}u}},' for a,b in data);out.append('};')
 out.append('static const struct {unsigned first,end;uint32_t window,event;uint64_t hash;} DAY1_card_confirm_cases[]={')
 for a,b,c,d,e in cases:out.append(f'{{{a},{b},0x{c:X}u,0x{d:X}u,UINT64_C(0x{e:016X})}},')
 out.append('};');header='\n'.join(out)+'\n';path=ROOT/'pc_port/tests/retail_card_confirmation_cases.h'
 if '--write-header' in sys.argv:path.write_text(header)
 else:assert path.read_text()==header
 print('PASS128 original confirmation graphs,64 include deferred callback and drawing')
if __name__=='__main__':main()
