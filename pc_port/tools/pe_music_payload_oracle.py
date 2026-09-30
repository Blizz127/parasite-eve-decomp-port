#!/usr/bin/env python3
"""Original valid music payload commands with an already configured mode."""
import hashlib,struct,sys
from pe_battle_hud_oracle import ROOT,execute
RANGES=((0x9D268,4),(0x9D2C8,4),(0x9D2F4,4),(0x9B3A0,4),(0xB6980,0x60),(0xB8628,0x300),(0xBCD80,0x14),(0x150000,0x20))
def digest(r):
 h=14695981039346656037
 for a,n in RANGES:
  for b in r[a:a+n]:h=((h^b)*1099511628211)&0xffffffffffffffff
 return h

def main():
 ex=(ROOT/'build/disc1.candidate.exe').read_bytes();assert hashlib.sha1(ex).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
 common=None;patches=[];cases=[]
 for cmd in (0x10,0x12,0x19):
  for song in (0,65535):
   for same in (0,1):
    for index in (0,7,15):
     r=bytearray(0x200000);r[0x10000:0x10000+len(ex)-2048]=ex[2048:]
     def sw(a,v):struct.pack_into('<I',r,a,v&0xffffffff)
     def sh(a,v):struct.pack_into('<H',r,a,v&0xffff)
     for a,n in RANGES:r[a:a+n]=bytes(n)
     sw(0x9D2C8,0x800B6980);sw(0x9D2F4,index);sw(0x9B3A0,3)
     sh(0xB69D4,song if same else song^65535)
     sw(0x150000,0x4F414B41);sh(0x150004,song);sh(0x150008,3)
     for i,v in enumerate((cmd,0x80150000,0x87654321,0x12345678,0xABCDEF01)):sw(0xBCD80+4*i,v)
     seed={a+i:struct.unpack_from('<I',r,a+i)[0] for a,n in RANGES for i in range(0,n,4)}
     if common is None:common={a:v for a,v in seed.items() if v}
     first=len(patches);patches.extend((a,v) for a,v in seed.items() if v!=common.get(a,0))
     regs=execute(r,0x8008CBA8)
     assert regs[2]==(0 if same else song),(cmd,song,same,hex(regs[2]))
     cases.append((first,len(patches),digest(r),regs[2]))
 lines=['/* Full original valid music command graphs; configured mode matches. */']
 for name,rows in (('ranges',RANGES),('common',sorted(common.items())),('patches',patches)):
  lines.append(f'static const uint32_t MP_{name}[][2]={{');lines.extend(f'{{0x{a:X}u,0x{v:X}u}},' for a,v in rows);lines.append('};')
 lines.append('static const struct { unsigned first,end; uint64_t hash; uint32_t result; } MP_cases[]={');lines.extend('{%d,%d,UINT64_C(0x%016X),%du},'%c for c in cases);lines.append('};')
 p=ROOT/'pc_port/tests/retail_music_payload_cases.h';out='\n'.join(lines)+'\n'
 if '--check' in sys.argv:assert p.read_text()==out
 else:p.write_text(out)
 print(f'PASS {len(cases)} original valid music payload graphs')
if __name__=='__main__':main()
