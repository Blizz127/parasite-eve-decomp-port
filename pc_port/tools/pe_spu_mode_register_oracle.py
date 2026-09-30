#!/usr/bin/env python3
"""Full original 8D140 masked mode-register writes with explicit RAM register fixture."""
import hashlib,struct,sys
from pe_battle_hud_oracle import ROOT,execute
RANGES=((0x9B3FC,4),(0x150000,0x44),(0x151000,0x200))
def digest(r):
 h=14695981039346656037
 for a,n in RANGES:
  for b in r[a:a+n]:h=((h^b)*1099511628211)&0xffffffffffffffff
 return h

def main():
 ex=(ROOT/'build/disc1.candidate.exe').read_bytes();assert hashlib.sha1(ex).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
 masks=[0,0xffffffff,0x55555555,0xaaaaaaaa]+[1<<i for i in range(32)]+[0xffffffff^(1<<i) for i in range(32)]
 common=None;patches=[];cases=[]
 for variant in range(3):
  for mask in masks:
   r=bytearray(0x200000);r[0x10000:0x10000+len(ex)-2048]=ex[2048:]
   for a,n in RANGES:r[a:a+n]=bytes(n)
   struct.pack_into('<I',r,0x9b3fc,0x80151000);struct.pack_into('<I',r,0x150000,mask)
   for i in range(32):struct.pack_into('<H',r,0x150004+i*2,(i*0x1235+variant*0x7fff)&0xffff)
   r[0x151000:0x151200]=bytes([0xa5])*0x200
   seed={a+i:struct.unpack_from('<I',r,a+i)[0] for a,n in RANGES for i in range(0,n,4)}
   if common is None:common={a:v for a,v in seed.items() if v}
   first=len(patches);patches.extend((a,v) for a,v in seed.items() if v!=common.get(a,0))
   execute(r,0x8008D140,(0x80150000,))
   cases.append((first,len(patches),digest(r)))
 lines=['/* Original 8D140; explicit RAM register fixture, no SPU hardware claim. */']
 for name,rows in (('ranges',RANGES),('common',sorted(common.items())),('patches',patches)):
  lines.append(f'static const uint32_t SMR_{name}[][2]={{');lines.extend(f'{{0x{a:X}u,0x{v:X}u}},' for a,v in rows);lines.append('};')
 lines.append('static const struct { unsigned first,end; uint64_t hash; } SMR_cases[]={');lines.extend('{%d,%d,UINT64_C(0x%016X)},'%c for c in cases);lines.append('};')
 out='\n'.join(lines)+'\n';p=ROOT/'pc_port/tests/retail_spu_mode_register_cases.h'
 if '--check' in sys.argv:assert p.read_text()==out
 else:p.write_text(out)
 print(f'PASS {len(cases)} complete original SPU mode-register graphs')
if __name__=='__main__':main()
