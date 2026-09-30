#!/usr/bin/env python3
"""Original choice navigation, confirmation and cursor packet rendering."""
import hashlib,struct,sys
from pe_message_glyph_oracle import RANGES as GLYPH_RANGES
from pe_battle_hud_oracle import ROOT,execute
RANGES=GLYPH_RANGES+((0x9D1F4,4),)
def digest(r):
 h=14695981039346656037
 for a,n in RANGES:
  for b in r[a:a+n]:h=((h^b)*1099511628211)&0xffffffffffffffff
 return h

def main():
 ex=(ROOT/'build/disc1.candidate.exe').read_bytes();assert hashlib.sha1(ex).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
 common=None;patches=[];cases=[];seen=set()
 for count in (*range(8),8,255):
  for current in (0,1,6,7,127,128,255):
   for pad in (0,8,32,40,256,264,288,296):
    for bank in (0,1):
     for variant in (0,1):
      r=bytearray(0x200000);r[0x10000:0x10000+len(ex)-2048]=ex[2048:]
      for a,n in RANGES:r[a:a+n]=bytes(n)
      def sw(a,v):struct.pack_into('<I',r,a&0x1fffff,v&0xffffffff)
      def sh(a,v):struct.pack_into('<H',r,a&0x1fffff,v&65535)
      execute(r,0x800371B0,(0x80156000,));sw(0x9CDDC,bank);sw(0x9D1F4,pad);r[0x9CEA0]=current;r[0x9CEA4]=0xFE
      for i in range(2):sw(0xB0E38+i*4,0x80155000+i*32);sw(0xB0E44+i*4,0x80150000+i*0x800)
      r[0xBCEA8]=2;r[0xBCEB0]=2;sw(0xBCEAC,0x80156000);sw(0xBCEB4,0x02000000|0xC03FFFFF)
      sh(0xBCEBA,65530 if variant else 100);sh(0xBCEBC,65535 if variant else 15)
      stream=bytes((0x30,0xFB,9,count,0x31,0xF7,0x32,0xFF));r[0x156000:0x156000+len(stream)]=stream
      seed={a+i:struct.unpack_from('<I',r,a+i)[0] for a,n in RANGES for i in range(0,n,4)}
      if common is None:common={a:v for a,v in seed.items() if v}
      first=len(patches);patches.extend((a,v) for a,v in seed.items() if v!=common.get(a,0));hs=[]
      for frame in range(2):
       for i in range(2):
        for j in range(4):sw(0x155000+i*32+j*4,0xFFFFFF if not j else 0x155000+i*32+(j-1)*4)
       execute(r,0x80037870,visited_pcs=seen,instruction_budget=100000);hs.append(digest(r))
      cases.append((first,len(patches),*hs))
 assert {0x800381F0,0x800382C8,0x80038328}<=seen
 lines=['/* Complete original37870 choice control and cursor packet; two input frames. */']
 for name,rows in (('ranges',RANGES),('common',sorted(common.items())),('patches',patches)):
  lines.append(f'static const uint32_t MC_{name}[][2]={{');lines.extend(f'{{0x{a:X}u,0x{b:X}u}},' for a,b in rows);lines.append('};')
 lines.append('static const struct { unsigned first,end; uint64_t hash[2]; } MC_cases[]={');lines.extend('{%d,%d,{UINT64_C(0x%016X),UINT64_C(0x%016X)}},'%c for c in cases);lines.append('};')
 header='\n'.join(lines)+'\n';p=ROOT/'pc_port/tests/retail_message_choice_cases.h'
 if '--check' in sys.argv:assert p.read_text()==header
 else:p.write_text(header)
 print(f'PASS {len(cases)} original choice graphs, {len(cases)*2} frame checkpoints')
if __name__=='__main__':main()
