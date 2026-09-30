#!/usr/bin/env python3
"""Complete original music command consumers, including fresh/cached voices."""
import hashlib,struct,sys
from pe_battle_hud_oracle import ROOT,execute
RANGES=((0x9D2C4,0x38),(0x9D22C,4),(0x9CDE8,4),(0x9C0C0,0x400),(0xB2900,0x20),(0xB6980,0x6420),(0x150000,0x100))
def fnv(r):
 h=14695981039346656037
 for a,n in RANGES:
  for b in r[a:a+n]:h=((h^b)*1099511628211)&0xffffffffffffffff
 return h

def main():
 ex=(ROOT/'build/disc1.candidate.exe').read_bytes();assert hashlib.sha1(ex).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
 common=None;patches=[];cases=[]
 for cmd in (0x10,0x12,0x19):
  for cached in (0,1):
   for paused in (0,1):
    for mask in (0,1,3,0x800000,0x555555,0xffffff):
     r=bytearray(0x200000);r[0x10000:0x10000+len(ex)-2048]=ex[2048:]
     def sw(a,v):struct.pack_into('<I',r,a,v&0xffffffff)
     def sh(a,v):struct.pack_into('<H',r,a,v&0xffff)
     for a,n in RANGES:
      if a!=0x9C0C0:r[a:a+n]=bytes(n)
     sw(0x9D2C8,0x800B6980);sw(0x9D2F4,1);sw(0x9D2DC,paused);sw(0x9CDE8,0x100 if paused else 0)
     sw(0xB6984,mask);sw(0xB69EC,0 if cached else 0x101)
     sw(0xB896C,mask);sw(0xB897C,0x123456);sw(0xB8994,0x80149000);sh(0xB89BC,7 if cached else 0)
     for i in range(24):
      v=0xB6B80+i*0x11c;r[v:v+0x11c]=bytes([0xa5])*0x11c
      sw(0xBA560+i*0x11c+0xf0,i)
     r[0xB2900:0xB2920]=bytes(range(32))
     sw(0x150000,mask|0xAB000000);sw(0x150004,0xFE123456);sw(0x150008,0xDC654321)
     for i in range(24):sh(0x150010+i*2,0x40+i*3)
     for i,v in enumerate((cmd,0x80150000,0x12345678,7,3,0)):sw(0xB8628+i*4,v)
     seed={a+i:struct.unpack_from('<I',r,a+i)[0] for a,n in RANGES for i in range(0,n,4)}
     if common is None:common={a:v for a,v in seed.items() if v}
     first=len(patches);patches.extend((a,v) for a,v in seed.items() if v!=common.get(a,0))
     execute(r,0x8008CA84,instruction_budget=1000000)
     cases.append((first,len(patches),fnv(r)))
 lines=['/* Full original music command consumers: fresh/cached/secondary voices. */']
 for name,rows in (('ranges',RANGES),('common',sorted(common.items())),('patches',patches)):
  lines.append(f'static const uint32_t MUSICCON_{name}[][2]={{');lines.extend(f'{{0x{a:X}u,0x{v:X}u}},' for a,v in rows);lines.append('};')
 lines.append('static const struct { unsigned first,end; uint64_t hash; } MUSICCON_cases[]={');lines.extend('{%d,%d,UINT64_C(0x%016X)},'%c for c in cases);lines.append('};')
 p=ROOT/'pc_port/tests/retail_music_consumer_cases.h';out='\n'.join(lines)+'\n'
 if '--check' in sys.argv:assert p.read_text()==out
 else:p.write_text(out)
 print(f'PASS {len(cases)} complete original music consumer graphs')
if __name__=='__main__':main()
