#!/usr/bin/env python3
"""Original whole 8F344 camera basis and 78134 normalization call graphs.

No arithmetic/callee substitution. Test-only return reader records GTE state.
Overflow cases compare persistent RAM up to the original trapping ADD only.
"""
import hashlib,random,struct,sys
from pe_battle_hud_oracle import ROOT,execute
from pe_m0000i_leaves_oracle import load_overlay
from pe_transition_loader_oracle import fnv
DATA=(8,9,10,11,20,21,22,25,26,27)
CONTROL=tuple(range(8))
def main():
 ex,o,b=load_overlay()
 assert hashlib.sha256(o[0x8018F344-b:0x8018F55C-b]).hexdigest()=='f0c1895a53bcbf8fff5a09082fec793408ac4a1aaa4ff245ca49e7fea451b270'
 assert hashlib.sha256(ex[0x68934:0x68A54]).hexdigest()=='e0e8ffd672aa993f534877c6d939d3c837fc43b0ffa8d71f6f5a53cb717ebf10'
 base=bytearray(0x200000);base[0x10000:0x10000+len(ex)-0x800]=ex[0x800:];base[b&0x1FFFFF:(b&0x1FFFFF)+len(o)]=o
 table=base[0x961D0:0x963D0]
 out=['/* Generated from original instructions by pe_transition_basis_oracle.py. */','static const uint8_t DAY1_basis_table[]={'+','.join(str(v) for v in table)+'};', 'static const struct { unsigned normal,seed,offset,trap; uint32_t vectors[9],ret,state[18]; uint64_t hash; } DAY1_basis_cases[]={']
 rng=random.Random(42);cases=[]
 vectors=[(0,0,0),(1,0,0),(0,-1,0),(0,0,1),(32767,0,0),(-32768,-32768,-32768),(32767,32767,32767),(65536,-65536,0),(0x7FFFFFFF,0x80000000,12345)]
 vectors += [tuple(rng.randrange(-65536,65536) for _ in range(3)) for _ in range(80)]
 for v in vectors:
  for offset in (0,4,64):cases.append((1,len(cases)&255,offset,v+(0,)*6))
 for i in range(320):
  eye=tuple(rng.randrange(-32768,32768) for _ in range(3));target=tuple(rng.randrange(-32768,32768) for _ in range(3));up=(0,-20000,0)
  if i<24:
   eye=(0,0,0);target=((0,0,0),(1,0,0),(0,1,0),(0,0,1),(0,-1,0),(32767,32767,32767))[i%6]
  if i%7==0:up=tuple(rng.randrange(-32768,32768) for _ in range(3))
  cases.append((0,i&255,(0,16,32,64)[i%4],eye+target+up))
 traps=0
 for normal,seed,offset,v in cases:
  r=bytearray(base);r[0x140000:0x140100]=bytes((seed+i*17)&255 for i in range(256))
  if normal:struct.pack_into('<3I',r,0x140000,*(x&0xFFFFFFFF for x in v[:3]))
  else:
   struct.pack_into('<3h',r,0x140000,*v[:3]);struct.pack_into('<3h',r,0x140010,*v[3:6]);struct.pack_into('<3i',r,0x140020,*v[6:])
  before=bytes(r)
  code=[0x3C098014,0x35292100,(43<<26)|(9<<21)|(2<<16)]
  for j,reg in enumerate(CONTROL):code.extend(((18<<26)|(2<<21)|(8<<16)|(reg<<11),(43<<26)|(9<<21)|(8<<16)|(4+j*4)))
  for j,reg in enumerate(DATA):code.extend(((18<<26)|(8<<16)|(reg<<11),(43<<26)|(9<<21)|(8<<16)|(36+j*4)))
  code.extend((8,0))
  for j,w in enumerate(code):struct.pack_into('<I',r,0x142000+j*4,w)
  trap=0;ret=0;state=[0]*18
  try:
   execute(r,0x80078134 if normal else 0x8018F344,(0x80140000,0x80140000+offset) if normal else (0x80140000+offset,0x80140000,0x80140010,0x80140020),initial_regs={31:0x80142000})
   ret=struct.unpack_from('<I',r,0x142100)[0];state=list(struct.unpack_from('<18I',r,0x142104))
   state[4]&=65535
   for j in range(8,12):state[j]&=65535
  except AssertionError as exc:
   assert exc.args==('retail signed arithmetic overflow',),exc
   trap=1;traps+=1
  check=bytearray(r[:0x1F0000]);check[0x140000:0x140100]=before[0x140000:0x140100];check[0x142000:0x142180]=before[0x142000:0x142180]
  assert check==before[:0x1F0000],'unexpected persistent write'
  h=fnv(r[0x140000:0x140100])
  out.append(f' {{{normal},{seed},{offset},{trap},{{'+','.join(f'0x{x&0xFFFFFFFF:X}u' for x in v)+f'}},0x{ret:X}u,{{'+','.join(f'0x{x:X}u' for x in state)+f'}},UINT64_C(0x{h:016X})}},')
 out.append('};')
 if '--write-header' in sys.argv:(ROOT/'pc_port/tests/retail_transition_basis_cases.h').write_text('\n'.join(out)+'\n')
 print(f'PASS: {len(cases)} original whole normalization/basis graphs; {traps} trapping ADD prefixes')
if __name__=='__main__':main()
