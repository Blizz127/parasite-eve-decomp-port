#!/usr/bin/env python3
"""Original M0034I module4 setup after attachment, through its first yield."""
import hashlib, json, struct, sys
from pe_day2_route_audit import ROOT,parse_field_table,extract_script_from_package,decode_script,find_disc,read_form1
from pe_battle_hud_oracle import execute
BASE=0x801B4B00
RANGES=((0x150000,0x300),(0x152000,0x20),(0x153000,216),(0xA5D58,1540),(0x9D2EC,1),(0x9D2A0,1),(0x9CE00,4),(0x9DF70,8),(0xB6AA8,4),(0x70E04,76))
def digest(r):
 h=14695981039346656037
 for a,n in RANGES:
  for b in r[a:a+n]:h=((h^b)*1099511628211)&0xFFFFFFFFFFFFFFFF
 return h

def load():
 ex=(ROOT/'build/disc1.candidate.exe').read_bytes();assert hashlib.sha1(ex).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
 rec=next(r for r in parse_field_table(ex) if r['map_id']==34);m=rec['meta'];prefix=((m&255)+(m>>8&4095))*2048
 pkg=read_form1(find_disc(ROOT),1013+rec['start'],prefix//2048+(m>>20));off,raw=extract_script_from_package(pkg,m)
 assert BASE==0x8018EFE8+off-prefix
 assert decode_script(raw)['sha256']=='dc224a516c3d0eaaf16d87b03bc1dc622ebca19ef419a6a2d51c8dfe8daa3603'
 return ex,raw

def main():
 ex,raw=load();rows=[];seen=set();results=[]
 for persist in (0,1,65536,0x12345678,0x7FFFFFFF,0x80000000,0xFFFFFFFF):
  for first in (0,9,10,39,40,99):
   for second in (0,49,50,99):
    for occupied in (0,3,6,7):
     r=bytearray(0x200000);r[0x10000:0x10000+len(ex)-0x800]=ex[0x800:];r[BASE&0x1FFFFF:(BASE&0x1FFFFF)+len(raw)]=raw
     def sw(a,v):struct.pack_into('<I',r,a&0x1FFFFF,v&0xFFFFFFFF)
     def lw(a):return struct.unpack_from('<I',r,a&0x1FFFFF)[0]
     actor=0x80150000;task=0x80152000
     sw(actor,0x80153000);r[0x15000C]=4;sw(actor+0x98,0x2000);sw(actor+0x9C,BASE+0x40CC)
     sw(0x8009D2F0,actor);sw(0x8009D300,task);sw(0x8009D1A0,0);sw(task,BASE+0x40E0);sw(task+16,1);sw(0x800A7930,persist)
     r[0x9D2EC]=255;r[0x9D2A0]=31
     for i in range(7):sw(0x800A5D58+i*220,1 if i<occupied else 0)
     sw(0x80070E04,0);sw(0x80070E08,8)
     for i in range(17):sw(0x80070E0C+i*4,0)
     sw(0x80070E0C,(first*65536+99)//100);sw(0x80070E4C,(second*65536+99)//100)
     execute(r,0x80017018,scratchpad=bytearray(0x400),visited_pcs=seen,instruction_budget=200000)
     assert lw(task)==BASE+0x451C and lw(task+16)==1
     assert lw(actor+0xBC)==(34 if first>=10 else 50)
     assert lw(actor+0xC0)==(33 if second>=50 else 49)
     value=persist or 65536
     scaled=((value*80)&0xFFFFFFFF);scaled=(scaled if scaled<0x80000000 else scaled-0x100000000)>>16
     health=max(scaled,0)+1000000
     scaled6=(value*6)&0xFFFFFFFF;scaled6=(scaled6 if scaled6<0x80000000 else scaled6-0x100000000)>>16
     body=lw(actor)
     assert body==(0x800A5D5C+occupied*220 if occupied<7 else 0x80153000)
     assert lw(body+0x10)==health and lw(body+0x14)==health and lw(body+0x88)==health, (persist,first,second,occupied,hex(body),health,lw(body+0x10),lw(body+0x14),lw(body+0x88),scaled6)
     assert lw(0x800B6AA8)==value
     rows.append((persist,first,second,occupied,digest(r)))
     results.append(dict(persist=persist,draws=[first,second],occupied=occupied,health=health,parameter60=lw(body+0x88),selection=[lw(actor+0xBC),lw(actor+0xC0)]))
 assert {0x800176FC,0x80070DD0,0x80018954,0x8002F7D8,0x80018164,0x80030220,0x80018004,0x8003010C,0x800172E0}<=seen
 lines=['/* Original M0034I setup through first delay; real script required. */','static const uint32_t M34S_ranges[][2]={']
 lines.extend(f'{{0x{a:X}u,{n}}},' for a,n in RANGES);lines.append('};')
 lines.append('static const struct { uint32_t persist; unsigned first,second,occupied; uint64_t hash; } M34S_cases[]={')
 lines.extend(f'{{0x{p:X}u,{a},{b},{o},UINT64_C(0x{h:016X})}},' for p,a,b,o,h in rows);lines.append('};')
 header='\n'.join(lines)+'\n';target=ROOT/'pc_port/tests/retail_m0034i_setup_cases.h'
 if '--check' in sys.argv:assert target.read_text()==header
 else:target.write_text(header)
 (ROOT/'local/live/m0034i-setup.json').write_text(json.dumps(dict(scope='original complete VM after attachment through first yield; supplied attached actor, task, allocation state and RNG state; no next-frame send or live scene proof',cases=results),indent=2)+'\n')
 print(f'PASS {len(rows)} original M0034I setup paths')
if __name__=='__main__':main()
