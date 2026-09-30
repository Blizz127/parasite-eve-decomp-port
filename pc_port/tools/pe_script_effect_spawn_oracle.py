#!/usr/bin/env python3
"""Original ED2401 color setup and ED2400/2402 complete E00CC allocation."""
import hashlib,struct,sys
from pe_battle_hud_oracle import ROOT,execute
RANGES=((0x9CDF8,4),(0xE21A4,4),(0xE2800,4),(0x150000,0x600),(0x154000,0x100))
def digest(r):
 h=14695981039346656037
 for a,n in RANGES:
  for b in r[a:a+n]:h=((h^b)*1099511628211)&0xFFFFFFFFFFFFFFFF
 return h

def main():
 ex=(ROOT/'build/disc1.candidate.exe').read_bytes();assert hashlib.sha1(ex).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
 common=None;patches=[];cases=[]
 for count in (0,1,19,20,21,0xFFFF,0x8000):
  for hole in (0,1,19,20,21,-1):
   for key in (2400,2402):
    for variant in range(4):
     r=bytearray(0x200000);r[0x10000:0x10000+len(ex)-2048]=ex[2048:]
     def sw(a,v):struct.pack_into('<I',r,a&0x1FFFFF,v&0xFFFFFFFF)
     r[0x150000:0x150600]=bytes([0xA5])*0x600
     sw(0xE2800,0x80150400);sw(0xE21A4,0xABCD0000|count);sw(0x9CDF8,0xDEADBEEF)
     if hole>=0:r[0x150400-hole*20]=0
     for i in range(8):sw(0x154000+i*4,0x80154040+i*4)
     for i,v in enumerate((2401,0x1234C0+variant,0x567808+variant,0x9ABC08+variant,0,0,0,0)):sw(0x154040+i*4,v)
     vals=(key,0xFA301234 if variant==0 else (0,0x7FFFFFFF,0x80000000,0xFFFFFFFF)[variant],0xFEA2ABCD,0xF1FC5678,(30,0,0xFFFF8000,0xFFFFFFFF)[variant],(10,0,256,0xFFFFFFFF)[variant])
     for i,v in enumerate(vals):sw(0x154080+i*4,v)
     seed={a+i:struct.unpack_from('<I',r,a+i)[0] for a,n in RANGES for i in range(0,n,4)}
     if common is None:common={a:v for a,v in seed.items() if v}
     first=len(patches);patches.extend((a,v) for a,v in seed.items() if v!=common.get(a,0))
     regs=execute(r,0x80016910,(0x80154000,));assert regs[2]==1
     color=digest(r)
     for i,v in enumerate(vals):sw(0x154040+i*4,v)
     regs=execute(r,0x80016910,(0x80154000,));assert regs[2]==1
     cases.append((first,len(patches),color,digest(r)))
 lines=['/* Original script effect color/allocation, explicit pool fixture. */']
 for name,rows in (('ranges',RANGES),('common',sorted(common.items())),('patches',patches)):
  lines.append(f'static const uint32_t ESP_{name}[][2]={{')
  lines.extend(f'{{0x{a:X}u,0x{b:X}u}},' for a,b in rows);lines.append('};')
 lines.append('static const struct { unsigned first,end; uint64_t color,spawn; } ESP_cases[]={')
 lines.extend(f'{{{a},{b},UINT64_C(0x{h:016X}),UINT64_C(0x{s:016X})}},' for a,b,h,s in cases);lines.append('};')
 header='\n'.join(lines)+'\n';target=ROOT/'pc_port/tests/retail_script_effect_spawn_cases.h'
 if '--check' in sys.argv:assert target.read_text()==header
 else:target.write_text(header)
 print(f'PASS {len(cases)} original effect color/allocation graphs')
if __name__=='__main__':main()
