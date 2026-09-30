#!/usr/bin/env python3
"""Original message numeric setup and complete renderer insertion graphs."""
import hashlib,struct,sys
from pe_battle_hud_oracle import ROOT,execute
RANGES=((0x9CE90,0x48),(0x9EC70,0x70),(0xBCEA8,224),(0x150000,0x2000),(0x155000,0x40),(0x156000,0x100),(0xB0E38,24),(0x9CDDC,4))
VALUES=(0,1,9,10,99,100,999,1000,9999,10000,32767,32768,65534,65535)
def digest(r):
 h=14695981039346656037
 for a,n in RANGES:
  for b in r[a:a+n]:h=((h^b)*1099511628211)&0xffffffffffffffff
 return h

def main():
 ex=(ROOT/'build/disc1.candidate.exe').read_bytes();assert hashlib.sha1(ex).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
 common=None;patches=[];cases=[];seen=set()
 for value in range(len(VALUES)):
  for length in range(6):
   for bank in (0,1):
    for variant in (0,1):
     r=bytearray(0x200000);r[0x10000:0x10000+len(ex)-2048]=ex[2048:]
     for a,n in RANGES:r[a:a+n]=bytes(n)
     def sw(a,v):struct.pack_into('<I',r,a&0x1fffff,v&0xffffffff)
     def sh(a,v):struct.pack_into('<H',r,a&0x1fffff,v&65535)
     execute(r,0x800371B0,(0x80156000,));execute(r,0x80037454,(65530 if variant else 100,65535 if variant else 15,0,0))
     sw(0x9CDDC,bank)
     for i in range(2):
      sw(0xB0E38+i*4,0x80155000+i*32);sw(0xB0E44+i*4,0x80150000+i*0x1000)
      for j in range(4):sw(0x155000+i*32+j*4,0xFFFFFF if not j else 0x155000+i*32+(j-1)*4)
     stream=bytes((0xFF,0xFE,7,0xFB,4 if variant else 5))+bytes((0xFB,8,0x0F))*5+bytes((0xFF,))
     r[0x156000:0x156000+len(stream)]=stream
     for i in range(6):sh(0x156080+i*2,VALUES[(value+i)%len(VALUES)] if i<length else 65535)
     seed={a+i:struct.unpack_from('<I',r,a+i)[0] for a,n in RANGES for i in range(0,n,4)}
     if common is None:common={a:v for a,v in seed.items() if v}
     first=len(patches);patches.extend((a,v) for a,v in seed.items() if v!=common.get(a,0))
     execute(r,0x800375E0,(7,2,0x80156080),visited_pcs=seen);opened=digest(r)
     execute(r,0x80037870,visited_pcs=seen,instruction_budget=100000)
     cases.append((first,len(patches),opened,digest(r)))
 assert {0x800375E0,0x800380CC,0x8003811C}<=seen
 lines=['/* Original375E0 numeric rows -> full37870 insertion, explicit text/draw fixture. */']
 for name,rows in (('ranges',RANGES),('common',sorted(common.items())),('patches',patches)):
  lines.append(f'static const uint32_t MN_{name}[][2]={{');lines.extend(f'{{0x{a:X}u,0x{b:X}u}},' for a,b in rows);lines.append('};')
 lines.append('static const struct { unsigned first,end; uint64_t opened,draw; } MN_cases[]={');lines.extend('{%d,%d,UINT64_C(0x%016X),UINT64_C(0x%016X)},'%c for c in cases);lines.append('};')
 header='\n'.join(lines)+'\n';p=ROOT/'pc_port/tests/retail_message_number_cases.h'
 if '--check' in sys.argv:assert p.read_text()==header
 else:p.write_text(header)
 print(f'PASS {len(cases)} original numeric message setup/render graphs')
if __name__=='__main__':main()
