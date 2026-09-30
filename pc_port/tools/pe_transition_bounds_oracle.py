#!/usr/bin/env python3
"""Original transition boundary planes and SDK vector helpers, real callees."""
import hashlib, random, struct, sys
from pe_battle_hud_oracle import ROOT, execute
from pe_m0000i_leaves_oracle import load_overlay
from pe_transition_loader_oracle import fnv
RANGES=((0x963DC,0x290),(0x140000,128),(0x19BFD0,32),(0x19CA90,0x190),(0x19CDF0,32))
def main():
 ex,o,b=load_overlay();base=bytearray(0x200000);base[0x10000:0x10000+len(ex)-0x800]=ex[0x800:];base[b&0x1fffff:(b&0x1fffff)+len(o)]=o
 assert hashlib.sha256(base[0x18F92C:0x18FFF4]).hexdigest()=='9c77bdeac144aeac997604f2c0a5b7cd52aab9b582bba9be26590296b045246e'
 # load_overlay pins the full original EXE and overlay, including SDK helpers.
 rng=random.Random(44);out=['/* Generated from original instructions by pe_transition_bounds_oracle.py. */', 'static const uint8_t DAY1_bounds_tables[]={'+','.join(map(str,base[0x95D00:0x963DC]))+'};','static const struct { unsigned kind,seed,offset,depth; uint32_t matrix[8],input[8],ret,state[18]; uint64_t hash; } DAY1_bounds_cases[]={']
 for k in range(600):
  kind=k//200;seed=k&255;offset=(0,4,16,32,64)[k%5];depth=(0,32,608)[k%3]
  mat=[rng.getrandbits(32) for _ in range(8)];inp=[rng.getrandbits(32) for _ in range(8)]
  if k%4==0:mat=[4096,0,4096,0,4096,0,0,0]
  if k%7==0:inp=[0]*8
  if kind==0 and k%2==0:mat[5:]=[rng.randrange(-10000,10000)&0xffffffff for _ in range(3)]
  r=bytearray(base)
  for a,n in RANGES:r[a:a+n]=bytes((seed+i*17)&255 for i in range(n))
  struct.pack_into('<I',r,0x963E8,depth)
  struct.pack_into('<8I',r,0x19CDF0,*mat);struct.pack_into('<8I',r,0x140000,*inp);struct.pack_into('<8I',r,0x19BFD0,*inp)
  before=bytes(r)
  code=[0x3C098014,0x35292100,(43<<26)|(9<<21)|(2<<16)]
  for j,reg in enumerate(range(8)):code.extend(((18<<26)|(2<<21)|(8<<16)|(reg<<11),(43<<26)|(9<<21)|(8<<16)|(4+j*4)))
  for j,reg in enumerate((8,9,10,11,20,21,22,25,26,27)):code.extend(((18<<26)|(8<<16)|(reg<<11),(43<<26)|(9<<21)|(8<<16)|(36+j*4)))
  code.extend((8,0))
  for j,w in enumerate(code):struct.pack_into('<I',r,0x142000+j*4,w)
  args=((0x80140000 if k%3==0 else 0x8019CBB0 if k%3==1 else 0x8019CB50),) if kind==0 else (0x80140000,0x80140000+offset,0x80140060) if kind==1 else (0x80140000,0x80140010,0x80140000+offset)
  execute(r,(0x8018F92C,0x800792D4,0x800791D0)[kind],args,initial_regs={31:0x80142000},initial_cop_control={i:mat[i] for i in range(8)},strict_gte_flags=True)
  ret=struct.unpack_from('<I',r,0x142100)[0];state=list(struct.unpack_from('<18I',r,0x142104));state[4]&=65535
  for i in range(8,12):state[i]&=65535
  check=bytearray(r[:0x1f0000]);check[0x142000:0x142180]=before[0x142000:0x142180]
  for a,n in RANGES:check[a:a+n]=before[a:a+n]
  assert check==before[:0x1f0000],f'unexpected persistent write {k}'
  h=fnv(b''.join(r[a:a+n] for a,n in RANGES))
  arr=lambda xs:'{'+','.join(f'0x{x:X}u' for x in xs)+'}'
  out.append(f' {{{kind},{seed},{offset},{depth},'+arr(mat)+','+arr(inp)+f',0x{ret:X}u,'+arr(state)+f',UINT64_C(0x{h:016X})'+'},')
 out.append('};')
 if '--write-header' in sys.argv:(ROOT/'pc_port/tests/retail_transition_bounds_cases.h').write_text('\n'.join(out)+'\n')
 print('PASS: 600 original boundary/SDK executions; persistent RAM and 18 terminal GTE words')
if __name__=='__main__':main()
