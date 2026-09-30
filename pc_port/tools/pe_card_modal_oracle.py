#!/usr/bin/env python3
"""Original card-state gate and both modal constructor modes."""
import hashlib,struct,sys
from pe_battle_hud_oracle import ROOT,execute
from pe_exit_menu_draw_oracle import fixture as draw_fixture,RANGES
from pe_scripted_exit_oracle import words
from pe_transition_loader_oracle import fnv
def fixture(ex,n):
 r,s,_=draw_fixture(ex,n%48)
 def sw(a,v):struct.pack_into('<I',r,a&0x1FFFFF,v&0xFFFFFFFF)
 mode=n%2;existing=n//2%3;index=n//24%2
 sw(0x9CF50,mode);sw(0xA1860,2 if n//6%2 else 0)
 r[0xA0ED4+index*0x418]=4 if n//12%2 else 0
 if existing:execute(r,0x80062D2C,((42 if mode else 40) if existing==1 else (40 if mode else 42),0,0,1))
 for ident in (0x47,0x48):
  ptr=execute(r,0x8005DC4C,(ident,))[2]&0x1FFFFF
  r[ptr:ptr+41]=bytes([32])*(40 if n//48%2 else 0)+bytes([255])+bytes(0 if n//48%2 else 40)
 if n>=96:
  sw(0x9CF50,1);sw(0xA1860,0x100) # Missing first text; second text remains valid.
 for a,size in RANGES:s[a:a+size]=r[a:a+size]
 return r,s,index
def main():
 ex=(ROOT/'build/disc1.candidate.exe').read_bytes();assert hashlib.sha1(ex).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
 for a,b,want in ((0x42848,0x428C4,'c671b8b029ab6145ec837dfda77f6c39d239a87dfd8d2bec18feb1c6b3f7048b'),(0x4DAA4,0x4DC84,'59f33cebaf98ea30ad979b913d7b96049459d44d034844811572827b3817af33')):
  digest=hashlib.sha256(ex[a-0xF800:b-0xF800]).hexdigest();assert digest==want;print(hex(a),digest)
 _,s,_=fixture(ex,0);base=words(s);common=[(i*4,v) for i,v in enumerate(base) if v];patches=[];cases=[];seen=set()
 for n in range(112):
  r,s,index=fixture(ex,n);first=len(patches);patches.extend((i*4,v) for i,(v,b) in enumerate(zip(words(s),base)) if v!=b)
  regs=execute(r,0x80042848 if n<96 else 0x8004DAA4,(index,),visited_pcs=seen)
  result=regs[2] if n<96 else 0;h=fnv(b''.join(r[a:a+size] for a,size in RANGES));cases.append((first,len(patches),index,result,h))
 assert {0x8004DB94,0x8004DBD4,0x8004DC4C,0x8004DB04}<=seen
 out=['/* Original card modal state. */']
 for name,data in (('ranges',RANGES),('common',common),('patches',patches)):
  out.append(f'static const uint32_t DAY1_card_modal_{name}[][2]={{');out.extend(f'{{0x{a:X}u,0x{b:X}u}},' for a,b in data);out.append('};')
 out.append('static const struct {unsigned first,end,index,result;uint64_t hash;} DAY1_card_modal_cases[]={')
 for a,b,c,d,e in cases:out.append(f'{{{a},{b},{c},{d},UINT64_C(0x{e:016X})}},')
 out.append('};');header='\n'.join(out)+'\n';path=ROOT/'pc_port/tests/retail_card_modal_cases.h'
 if '--write-header' in sys.argv:path.write_text(header)
 else:assert path.read_text()==header
 print('PASS 96 original gate/modal graphs and16 direct missing-text cases')
if __name__=='__main__':main()
