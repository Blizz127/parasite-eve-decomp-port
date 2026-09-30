#!/usr/bin/env python3
"""Run original VM op13 and its following wait across all five operand modes."""
import hashlib,itertools,struct,sys
from pe_battle_hud_oracle import ROOT,execute
RANGES=((0x910A0,0x80),(0x9CE00,4),(0x9D1A0,4),(0x9D254,4),(0x9D2F0,4),(0x9D300,4),(0x9DF70,32),(0xA77F0,32),(0xB6A80,32),(0x140000,0x400))
def fixture(exe,mode,flags,mask):
 r=bytearray(0x200000);r[0x10000:0x10000+len(exe)-0x800]=exe[0x800:]
 for a,n in RANGES:
  if a!=0x910A0:r[a:a+n]=bytes(n)
 def w(a,v):struct.pack_into('<I',r,a,v&0xFFFFFFFF)
 w(0x9D2F0,0x80140000);w(0x9D300,0x80140280);w(0x140098,flags)
 w(0x140280,0x80140300);w(0x140288,0x80);w(0x140290,1)
 w(0x140094,0x1738597A);w(0x14009C,0xDEADBEEF)
 w(0x140300,0x13|(1<<13)|(mode<<17));w(0x140304,0)
 w(0x140308,mask if mode==0 else 3)
 if mode:w((0x1400AC,0xA77F0,0x9DF70,0xB6A80)[mode-1]+12,mask)
 w(0x14030C,2|(1<<13));w(0x140310,0);w(0x140314,3)
 return r

def fingerprint(r):
 h=14695981039346656037
 for a,n in RANGES:
  for b in r[a:a+n]:h=((h^b)*1099511628211)&0xFFFFFFFFFFFFFFFF
 return h

def main():
 exe=(ROOT/'build/disc1.candidate.exe').read_bytes()
 assert hashlib.sha1(exe).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
 assert struct.unpack_from('<I',exe,0x910EC-0x10000+0x800)[0]==0x800176B8
 rows=[];base=None;patches=[];seen=set()
 for mode,flags,mask in itertools.product(range(5),(0,0x80000001,0xFFFFEFFF),(0,1,0x04000000,0x80000000,0xFFFFFFFF)):
  r=fixture(exe,mode,flags,mask);initial={a+i:struct.unpack_from('<I',r,a+i)[0] for a,n in RANGES for i in range(0,n,4)}
  if base is None:base=initial
  first=len(patches);patches.extend((a,v) for a,v in initial.items() if v!=base[a])
  execute(r,0x80017018,visited_pcs=seen)
  assert struct.unpack_from('<I',r,0x140098)[0]==flags|mask
  assert struct.unpack_from('<I',r,0x140280)[0]==0x80140318
  rows.append((first,len(patches),fingerprint(r)))
 assert 0x800176B8 in seen
 out=['/* Generated original VM actor flag and wait cases. */']
 for name,data in (('ranges',RANGES),('common',[(a,v) for a,v in base.items() if v]),('patches',patches)):
  out.append(f'static const uint32_t actor_flag_{name}[][2]={{')
  out.extend(f' {{0x{a:X}u,0x{v:X}u}},' for a,v in data);out.append('};')
 out.append('static const struct { unsigned first,end; uint64_t hash; } actor_flag_cases[]={')
 out.extend(f' {{{a},{b},UINT64_C(0x{h:X})}},' for a,b,h in rows);out.append('};')
 if '--write-header' in sys.argv:(ROOT/'pc_port/tests/retail_actor_flag_vm_cases.h').write_text('\n'.join(out)+'\n')
 print('PASS:',len(rows),'complete original actor-flag VM cases')
if __name__=='__main__':main()
