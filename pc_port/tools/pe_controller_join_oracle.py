#!/usr/bin/env python3
"""Original controller join through timed status message and border packets."""
import hashlib,struct,sys
from pe_menu_window_oracle import fixture,RANGES as WINDOW_RANGES
from pe_scripted_exit_oracle import words
from pe_battle_hud_oracle import ROOT,execute
RANGES=WINDOW_RANGES+((0xBCEA8,224),(0xBCF88,4))
def digest(r):
 h=14695981039346656037
 for a,n in RANGES:
  for b in r[a:a+n]:h=((h^b)*1099511628211)&0xffffffffffffffff
 return h

def main():
 ex=(ROOT/'build/disc1.candidate.exe').read_bytes();assert hashlib.sha1(ex).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
 common=None;patches=[];cases=[];seen=set()
 for state in (0,1,2,255):
  for timer in (0,1,75):
   for location in (0,2):
    for language in (0,1):
     for mode,pad in ((0,0),(0,1),(7,0),(7,65535)):
      r,s,_=fixture(ex,dict(entry=2))
      def put(a,b):r[a:a+len(b)]=b;s[a:a+len(b)]=b
      def sw(a,v):put(a,struct.pack('<I',v&0xffffffff))
      def sb(a,v):put(a,bytes((v,)))
      sb(0x9D1CE,state);sb(0x9CE88,timer);sb(0x9CE80,location);sw(0x9D218,language)
      sw(0x9D28C,mode);put(0x9D2A4,struct.pack('<H',pad));sb(0x9D244,0)
      sw(0x9D1F8,0x80149800);sw(0xBCF88,0xA5A5F123);put(0xBCEA8,bytes([0xA5])*224)
      w=words(s)
      if common is None:common=w
      first=len(patches);patches.extend((i*4,v) for i,(v,c) in enumerate(zip(w,common)) if v!=c)
      execute(r,0x8002AA24,stop_at={0x8002AA80},visited_pcs=seen,instruction_budget=1000000)
      cases.append((first,len(patches),digest(r)))
 assert {0x80034DE0,0x800375E0,0x80061C34,0x80067CBC}<=seen
 lines=['/* Original controller join, HUD disabled; complete timed message/border graph. */']
 for name,rows in (('ranges',RANGES),('common',[(i*4,v) for i,v in enumerate(common) if v]),('patches',patches)):
  lines.append(f'static const uint32_t CJ_{name}[][2]={{');lines.extend(f'{{0x{a:X}u,0x{b:X}u}},' for a,b in rows);lines.append('};')
 lines.append('static const struct { unsigned first,end; uint64_t hash; } CJ_cases[]={');lines.extend('{%d,%d,UINT64_C(0x%016X)},'%c for c in cases);lines.append('};')
 header='\n'.join(lines)+'\n';p=ROOT/'pc_port/tests/retail_controller_join_cases.h'
 if '--check' in sys.argv:assert p.read_text()==header
 else:p.write_text(header)
 print(f'PASS {len(cases)} original controller join/status/border graphs')
if __name__=='__main__':main()
