#!/usr/bin/env python3
"""Original multi-frame text waits/confirmation with multiple active records."""
import hashlib,struct,sys
from pe_message_glyph_oracle import RANGES as GLYPH_RANGES
from pe_battle_hud_oracle import ROOT,execute
RANGES=GLYPH_RANGES+((0x9D1F4,4),)
STREAMS=(bytes.fromhex(x) for x in ('30 fb06 06 31 fb06 32 ff','30 fb07 00 31 fb07 01 32 ff','30 fb07 02 31 fb06 32 ff','fb07 ff 30 fb07 01 31 ff','fb06 fb07 01 fb06 30 ff'))
STREAMS=tuple(STREAMS)
def digest(r):
 h=14695981039346656037
 for a,n in RANGES:
  for b in r[a:a+n]:h=((h^b)*1099511628211)&0xffffffffffffffff
 return h

def main():
 ex=(ROOT/'build/disc1.candidate.exe').read_bytes();assert hashlib.sha1(ex).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
 common=None;patches=[];cases=[];frames=[];seen=set()
 for stream in STREAMS:
  for counter in (0,1,2,15):
   for pause in (0,1,255):
    for pattern in (0,1,2):
     for records in (1,2):
      r=bytearray(0x200000);r[0x10000:0x10000+len(ex)-2048]=ex[2048:]
      for a,n in RANGES:r[a:a+n]=bytes(n)
      def sw(a,v):struct.pack_into('<I',r,a&0x1fffff,v&0xffffffff)
      def sh(a,v):struct.pack_into('<H',r,a&0x1fffff,v&65535)
      execute(r,0x800371B0,(0x80156000,))
      for i in range(2):sw(0xB0E38+i*4,0x80155000+i*32);sw(0xB0E44+i*4,0x80150000+i*0x800)
      r[0x156000:0x156000+len(stream)]=stream
      for i in range(records):
       a=0xBCEA8+i*56;r[a]=2;r[a+8]=2;sw(a+4,0x80156000);sw(a+12,0x02000000|(counter<<16)|(pause<<8));sh(a+18,20);sh(a+20,20+i*30)
      seed={a+i:struct.unpack_from('<I',r,a+i)[0] for a,n in RANGES for i in range(0,n,4)}
      if common is None:common={a:v for a,v in seed.items() if v}
      pfirst=len(patches);patches.extend((a,v) for a,v in seed.items() if v!=common.get(a,0));first=len(frames)
      for frame in range(8):
       sw(0x9CDDC,frame&1);sw(0x9D1F4,0x100 if pattern==1 or (pattern==2 and frame%3==2) else 0)
       for i in range(2):
        for j in range(4):sw(0x155000+i*32+j*4,0xFFFFFF if not j else 0x155000+i*32+(j-1)*4)
       execute(r,0x80037870,visited_pcs=seen,instruction_budget=100000);frames.append(digest(r))
      cases.append((pattern,pfirst,len(patches),first,len(frames)))
 assert {0x80037F84,0x80038010}<=seen
 lines=['/* Complete original37870 timed/confirmation streams, multi-record frames. */']
 for name,rows in (('ranges',RANGES),('common',sorted(common.items())),('patches',patches)):
  lines.append(f'static const uint32_t MW_{name}[][2]={{');lines.extend(f'{{0x{a:X}u,0x{b:X}u}},' for a,b in rows);lines.append('};')
 lines.append('static const struct { unsigned pattern,pfirst,pend,first,end; } MW_cases[]={');lines.extend('{%s},'%','.join(map(str,c)) for c in cases);lines.append('};')
 lines.append('static const uint64_t MW_frames[]={');lines.extend('UINT64_C(0x%016X),'%h for h in frames);lines.append('};')
 header='\n'.join(lines)+'\n';p=ROOT/'pc_port/tests/retail_message_wait_cases.h'
 if '--check' in sys.argv:assert p.read_text()==header
 else:p.write_text(header)
 print(f'PASS {len(cases)} original message wait graphs, {len(frames)} frame checkpoints')
if __name__=='__main__':main()
