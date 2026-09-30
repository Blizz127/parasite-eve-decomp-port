#!/usr/bin/env python3
"""Original A4 VM and parent-joint transform, including M0034I's arguments."""
import hashlib,struct,sys
from pe_attachment_vm_oracle import ROOT,execute,fixture as attachment_fixture,RANGES as ATT_RANGES
from pe_day2_route_audit import parse_field_table,extract_script_from_package,decode_script,find_disc,read_form1
RANGES=ATT_RANGES+((0x180000,0x2000),(0xB89F8,0x20))
CASES=[(v,t,j) for v in range(32) for t in (0,2,3,6,259) for j in (-1,0,1,49)]
def fixture(ex,c):
 v,target,joint=c;r=attachment_fixture(ex,(0xC5,v,target))
 def sw(a,x):struct.pack_into('<I',r,a,x&0xFFFFFFFF)
 def sh(a,x):struct.pack_into('<H',r,a,x&65535)
 for i in range(6):
  a=0x150000+i*0x400;r[a+13]=0
  sw(a+0x1CC,0x80180100);sw(a+0x238,0x80181000)
  sh(a+0x230,0x1234+i)
  for axis in range(3):sh(a+0x228+axis*2,0xFF00+i*4+axis)
 for i,x in enumerate((4096,0,0,0,4096,0,0,0,4096)):sh(0xB89F8+i*2,x)
 for i,x in enumerate((20,-30,200)):sw(0xB8A0C+i*4,x)
 # The record/joint arrays include a negative index slot before their base.
 record=0x180100+joint*16;matrix=0x181000+joint*32
 for i,x in enumerate((300 if v&4 else -300,-32768,32767,1 if v&8 else 65535)):sh(record+i*2,x)
 cells=(4096,0,0,0,4096,0,0,0,4096) if not v&8 else (0,0,4096,0,4096,0,-4096,0,0)
 for i,x in enumerate(cells):sh(matrix+i*2,x)
 for i,x in enumerate((100,-200,30000 if v&4 else 500)):sw(matrix+20+i*4,x)
 sw(0x170100,0xA4|(3<<13));sw(0x170108,target);sw(0x17010C,0);sw(0x170110,joint);sw(0x170114,0x20);sw(0x170118,0)
 return r

def fingerprint(r):
 h=14695981039346656037
 for a,n in RANGES:
  for b in r[a:a+n]:h=((h^b)*1099511628211)&0xFFFFFFFFFFFFFFFF
 return h

def main():
 ex=(ROOT/'build/disc1.candidate.exe').read_bytes();assert hashlib.sha1(ex).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
 rec=next(r for r in parse_field_table(ex) if r['map_id']==34);m=rec['meta'];pkg=read_form1(find_disc(ROOT),1013+rec['start'],(m&255)+(m>>8&4095)+(m>>20));_,raw=extract_script_from_package(pkg,m);s=decode_script(raw)
 assert s['sha256']=='dc224a516c3d0eaaf16d87b03bc1dc622ebca19ef419a6a2d51c8dfe8daa3603'
 site=next(c for c in s['modules'][4]['commands'] if c['offset']==0x40CC)
 assert site['opcode']==0xA4 and site['args']==[3,0,49] and site['modes']==[0,0,0]
 common=None;patches=[];cases=[];seen=set()
 for c in CASES:
  r=fixture(ex,c)
  if c[1:]==(3,49):assert r[0x170100:0x170114]==raw[0x40CC:0x40E0]
  seed={a+i:struct.unpack_from('<I',r,a+i)[0] for a,n in RANGES for i in range(0,n,4)}
  if common is None:common={a:v for a,v in seed.items() if v}
  first=len(patches);patches.extend((a,v) for a,v in seed.items() if v!=common.get(a,0))
  execute(r,0x80017018,visited_pcs=seen,scratchpad=bytearray(0x400),initial_cop_control={26:256,24:160<<16,25:112<<16})
  assert struct.unpack_from('<H',r,0x170008)[0]&0x10
  cases.append((first,len(patches),fingerprint(r)))
 assert {0x80015108,0x8003DF50,0x8003A6A8,0x8003E188,0x80015200}<=seen
 out=['/* Original A4 VM and parent joint transform expectations. */']
 for name,rows in (('ranges',RANGES),('common',sorted(common.items())),('patches',patches)):
  out.append(f'static const uint32_t ANC_{name}[][2]={{');out.extend(f'{{0x{a:X}u,0x{b:X}u}},' for a,b in rows);out.append('};')
 out.append('static const struct { unsigned first,end; uint64_t hash; } ANC_cases[]={');out.extend(f'{{{a},{b},UINT64_C(0x{h:016X})}},' for a,b,h in cases);out.append('};')
 header='\n'.join(out)+'\n';p=ROOT/'pc_port/tests/retail_anchor_attachment_cases.h'
 if '--write-header' in sys.argv:p.write_text(header)
 else:assert p.read_text()==header
 print(f'PASS {len(cases)} original A4 VM/joint-transform graphs; M0034I command bytes verified, continuation is synthetic20')
if __name__=='__main__':main()
