#!/usr/bin/env python3
"""Original VM dispatch through actor/model attachment and detachment commands."""
import hashlib,struct,sys
from pe_linked_animation_oracle import ROOT,execute,fixture as linked_fixture,RANGES as LINK_RANGES
RANGES=LINK_RANGES+((0,0x100),(0x910A0,0x400),(0x9CE00,4),(0x9D1A0,4),(0x9D2F0,4),(0x9D300,4),(0x170000,0x200))
OPS=(0xC5,0xCC,0xD4,0xD5,0xD6)
CASES=[(op,v,target) for op in OPS for v in range(32) for target in (0,1,6,257)]

def fixture(ex,c):
 op,v,target=c;r=linked_fixture(ex,(0x8001A784,v,5))
 for a,n in RANGES[len(LINK_RANGES):]:
  if a!=0x910A0:r[a:a+n]=bytes(n)
 def sw(a,x):struct.pack_into('<I',r,a,x&0xFFFFFFFF)
 sw(0x98,0xFFFFFFFF);r[2]=2 if v&8 else 1
 sw(0x9D1A0,0);sw(0x9D254,0 if v&1 else 0x80150000)
 actor=0x80150800;sw(0x9D2F0,actor);sw(0x9D300,0x80170000)
 if v&2:sw(0x9D20C,0)
 for i in range(6):
  a=0x150000+i*0x400
  sw(a+0x1B4,0 if v&4 else 0x80160100+i*8)
  r[0x160102+i*8]=2 if v&8 else 1
  sw(a+0x1D8,0xA5A5A5A5);sw(a+0x1DC,0xDEADBEEF)
  sw(a+0x98,0x700008|(0x10 if v&16 else 0))
  # No parentless peer makes D5 take its physical-RAM-zero tail.
  if v&4:sw(a+0x18C,actor)
 sw(0x170000,0x80170100);sw(0x170010,1)
 argc=3 if op==0xC5 else 2 if op==0xD4 else 0
 sw(0x170100,op|(argc<<13));sw(0x170104,0)
 for i,x in enumerate((target,11,0xFFFF8123)):sw(0x170108+i*4,x)
 sw(0x170108+argc*4,0x20);sw(0x17010C+argc*4,0)
 return r

def fingerprint(r):
 h=14695981039346656037
 for a,n in RANGES:
  for b in r[a:a+n]:h=((h^b)*1099511628211)&0xFFFFFFFFFFFFFFFF
 return h

def main():
 ex=(ROOT/'build/disc1.candidate.exe').read_bytes();assert hashlib.sha1(ex).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
 common=None;patches=[];cases=[];seen=set()
 for c in CASES:
  r=fixture(ex,c);seed={a+i:struct.unpack_from('<I',r,a+i)[0] for a,n in RANGES for i in range(0,n,4)}
  if common is None:common={a:v for a,v in seed.items() if v}
  first=len(patches);patches.extend((a,v) for a,v in seed.items() if v!=common.get(a,0))
  execute(r,0x80017018,visited_pcs=seen)
  assert struct.unpack_from('<H',r,0x170008)[0]&0x10
  cases.append((first,len(patches),fingerprint(r)))
 assert {0x80019170,0x80019260,0x80019F04,0x80019FE0,0x8001A064,0x8003E0A4,0x8003E0D0,0x8001A04C}<=seen
 out=['/* Generated original VM attachment graph expectations. */']
 for name,rows in (('ranges',RANGES),('common',sorted(common.items())),('patches',patches)):
  out.append(f'static const uint32_t ATT_{name}[][2]={{');out.extend(f'{{0x{a:X}u,0x{b:X}u}},' for a,b in rows);out.append('};')
 out.append('static const struct { unsigned first,end; uint64_t hash; } ATT_cases[]={')
 out.extend(f'{{{a},{b},UINT64_C(0x{h:016X})}},' for a,b,h in cases);out.append('};')
 header='\n'.join(out)+'\n';p=ROOT/'pc_port/tests/retail_attachment_vm_cases.h'
 if '--write-header' in sys.argv:p.write_text(header)
 else:assert p.read_text()==header
 print(f'PASS {len(cases)} original VM attachment graphs, including low-RAM detach tail')
if __name__=='__main__':main()
