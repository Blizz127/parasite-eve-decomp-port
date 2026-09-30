#!/usr/bin/env python3
"""Original escape failure -> status join -> rendered message expiry."""
import hashlib,struct,sys
from pe_menu_window_oracle import fixture,ROOT
from pe_controller_join_oracle import RANGES as JOIN_RANGES
from pe_scripted_exit_oracle import words
from pe_battle_hud_oracle import execute
RANGES=JOIN_RANGES+((0xB0E38,24),(0x180000,0x4800),(0x185000,0x40),(0x188000,0x1000),(0x9EC70,0x70))
def digest(r):
 h=14695981039346656037
 for a,n in RANGES:
  for b in r[a:a+n]:h=((h^b)*1099511628211)&0xffffffffffffffff
 return h

def main():
 ex=(ROOT/'build/disc1.candidate.exe').read_bytes();assert hashlib.sha1(ex).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
 common=None;patches=[];cases=[];frames=[];seen=set()
 for bank in (0,1):
  for language in (0,1):
   for blocked in (0,1):
    r,s,_=fixture(ex,dict(entry=2))
    def sw(a,v):struct.pack_into('<I',r,a&0x1fffff,v&0xffffffff)
    def sh(a,v):struct.pack_into('<H',r,a&0x1fffff,v&65535)
    r[0x180000:0x184800]=bytes(0x4800);r[0x185000:0x185040]=bytes(0x40)
    r[0x188000:0x189000]=bytes(0x1000)
    sw(0x9D20C,0x80188000);sw(0x9D254,0);sw(0x9D278,0x80188800);sw(0x188000,0x80188400);r[0x188404]=7;sw(0x188410,40);sw(0x1884cc,0x40000 if blocked else 0);r[0x188804]=1;sh(0x18880c,45);sh(0x18881c,45)
    execute(r,0x800371B0,(0x80189000,));sw(0x9D218,language);sw(0x9D28C,0);r[0x9D244]=0;sh(0x9D2A4,0);r[0x9CE80]=bank*2
    for i in range(2):sw(0xB0E38+i*4,0x80185000+i*32);sw(0xB0E44+i*4,0x80180000+i*0x2400)
    for a,n in RANGES:s[a:a+n]=r[a:a+n]
    w=words(s)
    if common is None:common=w
    pfirst=len(patches);patches.extend((i*4,v) for i,(v,c) in enumerate(zip(w,common)) if v!=c)
    result=execute(r,0x800255E4,bios_seed=1,visited_pcs=seen)[2];assert result==(0xffffffff if blocked else 0),result
    producer=digest(r);first=len(frames)
    for frame in range(78):
     sw(0x9CDDC,(frame+bank)&1);sw(0x9D100,0x80160000)
     for i in range(2):
      for j in range(4):sw(0x185000+i*32+j*4,0xffffff if not j else 0x185000+i*32+(j-1)*4)
     execute(r,0x8002AA24,stop_at={0x8002AA80},visited_pcs=seen)
     execute(r,0x80037870,instruction_budget=1000000,visited_pcs=seen)
     assert r[0x9D1CE]==(2 if frame<75 else 0)
     assert r[0x9CE88]==(74-frame if frame<75 else 255)
     assert r[0xBCEA8]==(2 if frame<75 else 0)
     frames.append(digest(r))
    cases.append((bank,pfirst,len(patches),first,len(frames),result,producer))
 assert {0x800255E4,0x80034DE0,0x800375E0,0x80037870,0x80061C34}<=seen
 lines=['/* Original escape judgement and full timed status rendering, explicit battle/draw fixture. */']
 for name,rows in (('ranges',RANGES),('common',[(i*4,v) for i,v in enumerate(common) if v]),('patches',patches)):
  lines.append(f'static const uint32_t EM_{name}[][2]={{');lines.extend(f'{{0x{a:X}u,0x{b:X}u}},' for a,b in rows);lines.append('};')
 lines.append('static const struct { unsigned bank,pfirst,pend,first,end; uint32_t result; uint64_t producer; } EM_cases[]={');lines.extend('{%d,%d,%d,%d,%d,0x%Xu,UINT64_C(0x%016X)},'%c for c in cases);lines.append('};')
 lines.append('static const uint64_t EM_frames[]={');lines.extend('UINT64_C(0x%016X),'%h for h in frames);lines.append('};')
 header='\n'.join(lines)+'\n';p=ROOT/'pc_port/tests/retail_escape_message_cases.h'
 if '--check' in sys.argv:assert p.read_text()==header
 else:p.write_text(header)
 print(f'PASS {len(cases)} original escape-message lifetimes, {len(frames)} rendered frame checkpoints')
if __name__=='__main__':main()
