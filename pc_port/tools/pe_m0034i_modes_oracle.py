#!/usr/bin/env python3
"""Original M0034I mode gate, persistent updates and fade-wait continuations."""
import json,struct,sys
from pe_m0034i_setup_oracle import load,BASE,ROOT
from pe_battle_hud_oracle import execute
MODES=(0,8,9,10,11,0x10009,0xFFFFFFFF)
RANGES=((BASE&0x1FFFFF,0x5768),(0xA3180,36),(0x9CDB4,1),(0x150000,0xC00),(0x160000,0xC0),(0x9CE00,4),(0x9DF70,32),(0xA7820,4),(0xA7838,4),(0xB6A80,64),(0xB0CD8,4),(0x9D2E8,4),(0xBCFD0,64))
def digest(r):
 h=14695981039346656037
 for a,n in RANGES:
  for byte in r[a:a+n]:h=((h^byte)*1099511628211)&0xFFFFFFFFFFFFFFFF
 return h

def main():
 ex,raw=load();rows=[];out=[];seen=set()
 for mode in MODES:
  for v in range(16):
   r=bytearray(0x200000);r[0x10000:0x10000+len(ex)-0x800]=ex[0x800:];r[BASE&0x1FFFFF:(BASE&0x1FFFFF)+len(raw)]=raw
   def sw(a,x):struct.pack_into('<I',r,a&0x1FFFFF,x&0xFFFFFFFF)
   def lw(a):return struct.unpack_from('<I',r,a&0x1FFFFF)[0]
   actor=0x80150000;aya=0x80150800;task=0x80160040
   sw(actor+0x9C,BASE+0x453C);r[0x15000C]=5;sw(actor+0xA0,0x80160000);sw(actor+0xA4,task);sw(actor+0xA8,task+0x40)
   sw(0x80160024,task);sw(task+0x28,0x80160000)
   sw(0x8009D254,aya);sw(0x8009D2F0,actor);sw(0x8009D300,task);sw(0x8009D1A0,0)
   sw(actor+0xC4,1 if v&8 else 0);r[0x9CDB4]=0
   sw(aya+0x28,0 if v&8 else 0x10000);sw(aya+0x30,0 if v&8 else 0xFFFF0000)
   sw(task,BASE+0x4770);sw(task+16,1);sw(0x8009D28C,mode)
   sw(0x800B6A80,(v&3)*0x1000);sw(0x800A7838,0xA5A50000|v);sw(0x800A7820,0x12345678)
   sw(0x8009D2E8,0xA4);sw(0x800B0CD8,0xA00000)
   r[0x91A1C]=1;r[0x91A1D]=(0,1,10,255)[v>>2];r[0xBCFEE]=0
   hashes=[];pcs=[]
   for i in range(2):
    sw(0x8009D300,task)
    execute(r,0x80017018,scratchpad=bytearray(0x400),visited_pcs=seen,instruction_budget=200000)
    hashes.append(digest(r));pcs.append(f'{lw(task)-BASE:04X}')
   if mode not in (9,10):assert pcs==['4770','4770']
   rows.append((mode,v,hashes));out.append(dict(mode=mode,variant=v,pc=pcs,persist18=lw(0x800A7838),scratch0=lw(0x800B6A80),fade=list(r[0xBCFD0:0xBCFF0]),script_output=lw(BASE+0x4CE8),local6=lw(actor+0xC4),queued=r[0x9CDB4]))
 assert {0x80019154,0x80017988,0x80019798,0x80014694,0x80014DA0,0x80066B60,0x80019410}<=seen
 lines=['/* Original M0034I mode continuations; requires original script. */','static const uint32_t M34M_ranges[][2]={']
 lines.extend(f'{{0x{a:X}u,{n}}},' for a,n in RANGES);lines+=['};','static const struct { uint32_t mode; unsigned variant; uint64_t hash[2]; } M34M_cases[]={']
 lines.extend('{0x%Xu,%d,{%s}},'%(m,v,','.join('UINT64_C(0x%016X)'%h for h in hs)) for m,v,hs in rows);lines.append('};')
 header='\n'.join(lines)+'\n';target=ROOT/'pc_port/tests/retail_m0034i_modes_cases.h'
 if '--check' in sys.argv:assert target.read_text()==header
 else:target.write_text(header)
 (ROOT/'local/live/m0034i-modes.json').write_text(json.dumps(dict(scope='full original VM two passes from4770, supplied battle mode/Aya position/root tasks/scene index; no actual fade advancement or mode producer proof',cases=out),indent=2)+'\n')
 print(f'PASS {len(rows)} original M0034I mode continuations')
if __name__=='__main__':main()
