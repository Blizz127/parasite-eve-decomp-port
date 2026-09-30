#!/usr/bin/env python3
"""Full original E01BC effect walk through E026C/E03A0/E051C."""
import hashlib,struct,sys
from pe_battle_hud_oracle import ROOT,execute
RANGES=((0x9CDD8,8),(0xB0E38,8),(0xB0E58,8),(0xBCFA4,8),(0xE21A4,4),(0xE2800,4),(0x150000,0x1000),(0x152000,0x2000),(0x155000,0x200))
def digest(r,s):
 h=14695981039346656037
 for a,n in RANGES:
  for b in r[a:a+n]:h=((h^b)*1099511628211)&0xffffffffffffffff
 for b in s[:0x38]:h=((h^b)*1099511628211)&0xffffffffffffffff
 return h

def main():
 ex=(ROOT/'build/disc1.candidate.exe').read_bytes();assert hashlib.sha1(ex).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
 common=None;patches=[];cases=[]
 for bank in range(2):
  for mode in range(2):
   for state in (1,2,3,255):
    for value in (0,48,255,0x8000):
     for speed in (0,10,255):
      r=bytearray(0x200000);r[0x10000:0x10000+len(ex)-2048]=ex[2048:];s=bytearray(0x400)
      def sw(a,v):struct.pack_into('<I',r,a&0x1fffff,v&0xffffffff)
      def sh(a,v):struct.pack_into('<H',r,a&0x1fffff,v&65535)
      sw(0x9CDD8,0);sw(0x9CDDC,bank);sw(0xB0E38,0x80152000);sw(0xB0E3C,0x80153000);sw(0xB0E58,0x80150000);sw(0xB0E5C,0x80150800)
      sw(0xBCFA4,0x80155000);sw(0xBCFA8,0x80155040);sw(0x155040,256);sw(0xE21A4,3);sw(0xE2800,0x80155100)
      for i in (0,8,16):sh(0x155000+i,4096)
      sw(0x15501c,1024)
      for i in range(0,0x2000,4):sw(0x152000+i,0xABFFFFFF)
      for j in range(3):
       a=0x155100-j*40;r[a:a+20]=bytes(20);r[a]=state if j==0 else 1;r[a+1]=speed;r[a+2]=j;r[a+3]=value&255;r[a+10]=mode if j<2 else 2
       sh(a+4,-100+j*60);sh(a+6,20-j*50);sh(a+8,j*256);sh(a+12,value);sh(a+14,30 if value!=0x8000 else -30);r[a+16:a+19]=bytes((192,8,8))
      seed={a+i:struct.unpack_from('<I',r,a+i)[0] for a,n in RANGES for i in range(0,n,4)}
      if common is None:common={a:v for a,v in seed.items() if v}
      first=len(patches);patches.extend((a,v) for a,v in seed.items() if v!=common.get(a,0));hs=[]
      for frame in range(8):
       execute(r,0x800E01BC,scratchpad=s,initial_cop_control={24:160<<16,25:112<<16,26:256});hs.append(digest(r,s))
      cases.append((first,len(patches),hs))
 lines=['/* Full original effect walk/update/draw graphs, synthetic pool/camera. */']
 for name,rows in (('ranges',RANGES),('common',sorted(common.items())),('patches',patches)):
  lines.append(f'static const uint32_t PET_{name}[][2]={{');lines.extend(f'{{0x{a:X}u,0x{b:X}u}},' for a,b in rows);lines.append('};')
 lines.append('static const struct { unsigned first,end; uint64_t hash[8]; } PET_cases[]={')
 for a,b,hs in cases:lines.append('{%d,%d,{%s}},'%(a,b,','.join('UINT64_C(0x%016X)'%h for h in hs)))
 lines.append('};');header='\n'.join(lines)+'\n';p=ROOT/'pc_port/tests/retail_park_effect_tick_cases.h'
 if '--check' in sys.argv:assert p.read_text()==header
 else:p.write_text(header)
 print(f'PASS {len(cases)} original effect graphs, {len(cases)*8} frame checkpoints')
if __name__=='__main__':main()
