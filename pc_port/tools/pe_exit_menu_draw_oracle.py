#!/usr/bin/env python3
"""Original exit-menu predicate, list drawing and row labels."""
import hashlib,struct,sys
from pe_battle_hud_oracle import ROOT,execute
from pe_inventory_draw_oracle import fixture as draw_fixture,RANGES as DRAW_RANGES
from pe_scripted_exit_oracle import words
from pe_transition_loader_oracle import fnv
RANGES=DRAW_RANGES+((0xA0E00,0xA70),)
def fixture(ex,n):
 r,s,_=draw_fixture(ex,dict(entry=11,bank=n%2,frame=(n//2%2)*8,offset=0x3FE0 if n>=48 else 0))
 execute(r,0x8004D18C)
 def sw(a,v):struct.pack_into('<I',r,a&0x1FFFFF,v&0xFFFFFFFF)
 node=struct.unpack_from('<I',r,0x9D15C)[0]
 r[0xA0ED4]=(n//4)%4;r[0xA12EC]=(n//16)%4
 sw(node+72,(n//4)%3);sw(node+68,0)
 for a,size in RANGES:s[a:a+size]=r[a:a+size]
 return r,s,node
def main():
 ex=(ROOT/'build/disc1.candidate.exe').read_bytes();assert hashlib.sha1(ex).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
 for a,b,want in ((0x42770,0x42798,'d7402296e472db8a1ceaa0da190dad0b33ca01eb6d84484c9e125a2ffe2b29ab'),(0x4FDA4,0x4FE58,'195a711c9047332870ac24e1be664e47713c67d50e9cb944d7244adcc6aad2be'),(0x50C08,0x50C50,'1c30fa45849ee274fec64c80586df607dac8bc016043d3f217037222fb2d646c')):
  digest=hashlib.sha256(ex[a-0xF800:b-0xF800]).hexdigest();assert digest==want;print(hex(a),digest)
 _,s,_=fixture(ex,0);base=words(s);common=[(i*4,v) for i,v in enumerate(base) if v];patches=[];cases=[]
 for n in range(64):
  r,s,node=fixture(ex,n);first=len(patches);patches.extend((i*4,v) for i,(v,b) in enumerate(zip(words(s),base)) if v!=b)
  execute(r,0x8004FDE8,(node,))
  h=fnv(b''.join(r[a:a+size] for a,size in RANGES));cases.append((first,len(patches),node,(n//2%2)*8,h))
 # Both predicate paths and unchecked wrapped table stride, independently
 # from the valid three-row menu fixture.
 predicates=[]
 for n in range(1024):
  r=bytearray(0x200000);r[0x10000:0x10000+len(ex)-0x800]=ex[0x800:]
  index=(0,1,2,3,0xFFFFFFFF,0x80000000,0x40000000,15)[n//256%8] if n<512 else (0,1,2,3,0xFFFFFFFF,0x80000000,0x40000000,15)[n%8]
  value=n&255;r[(0xA0ED4+index*0x418)&0x1FFFFF]=value
  result=execute(r,0x8004FDA4,(index,))[2];predicates.append((index,value,result))
 out=['/* Original exit menu drawing and predicates. */']
 for name,data in (('ranges',RANGES),('common',common),('patches',patches)):
  out.append(f'static const uint32_t DAY1_exit_draw_{name}[][2]={{');out.extend(f'{{0x{a:X}u,0x{b:X}u}},' for a,b in data);out.append('};')
 out.append('static const struct { unsigned first,end; uint32_t node,frame; uint64_t hash; } DAY1_exit_draw_cases[]={')
 for a,b,c,d,e in cases:out.append(f'{{{a},{b},0x{c:X}u,{d},UINT64_C(0x{e:016X})}},')
 out.append('};\nstatic const uint32_t DAY1_exit_draw_predicates[][3]={')
 out.extend(f'{{0x{a:X}u,{b},{c}}},' for a,b,c in predicates);out.append('};')
 header='\n'.join(out)+'\n';path=ROOT/'pc_port/tests/retail_exit_menu_draw_cases.h'
 if '--write-header' in sys.argv:path.write_text(header)
 else:assert path.read_text()==header
 print('PASS 64 original menu draw graphs and1024 predicates')
if __name__=='__main__':main()
