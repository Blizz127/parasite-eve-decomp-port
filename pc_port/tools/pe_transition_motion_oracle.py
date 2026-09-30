#!/usr/bin/env python3
"""Whole original scene-motion and path-seeding graphs; no provider replacements."""
import hashlib,itertools,struct,sys
from pe_battle_hud_oracle import ROOT,execute
from pe_m0000i_leaves_oracle import load_overlay
from pe_transition_camera_oracle import fixture as camera_fixture
from pe_transition_loader_oracle import fnv
RANGES=((0x19BFCC,4),(0x19C038,0x24),(0x19CAA8,160),(0x1EA268,160),(0x1EA578,24),(0x150000,512))
BANKS=(0,1,-1,7,-7,8,-8,511,512,513,-511,-512,-513,383,384,385,-383,-384,-385,32767,-32768,200,-200,16)
TIMES=(0,255,0xFFFFFFFF)
SLOTS=((0,64,128,192,256,320),(0,0,128,128,256,320),(0,4,128,132,256,320),(0,0,0,0,0,0),(0,144,128,16,256,320))
def fixture(ex,o,b,seed,alias,time,kind,id,fault):
 r=camera_fixture(ex,o,b,seed,0)
 for a,n in RANGES:r[a:a+n]=bytes((seed+i*17)&255 for i in range(n))
 struct.pack_into('<2I',r,0x19C038,time,time)
 struct.pack_into('<6I',r,0x1EA578,*(0x80150000+i for i in SLOTS[alias]))
 for index,bank in ((0,BANKS[seed]),(2,-BANKS[seed])):struct.pack_into('<H',r,0x150000+SLOTS[alias][index]+44,bank&65535)
 if seed%4==0:
  for i in range(-1,257):
   rec=0x141000+(i+1)*96
   for j in range(9):
    for k in range(3):struct.pack_into('<H',r,rec+8+j*8+k*2,(i*7919+k*29009+seed*4711)&65535)
 if fault:
  index=73+fault if kind==0 else (id&65535)+64
  rec=0x140000+struct.unpack_from('<I',r,0x140000+index*4)[0]
  struct.pack_into('<H',r,rec+6,2)
 return r

def main():
 ex,o,b=load_overlay()
 assert hashlib.sha256(o[0x80193478-b:0x80193AB0-b]).hexdigest()=='ffc912797b4704567e6968ec68b5c1cabc72d327dd8c822438ef4ed3577e5b9a'
 cases=[(0,s,a,t,0,0,3) for s,a,t in itertools.product(range(24),range(5),TIMES)]
 cases += [(k,s,0,0,i,0,2) for k,s,i in itertools.product((1,2),range(24),(0xFFFFFFFF,0,1,9,0x10001))]
 cases += [(0,1,0,255,0,f,1) for f in range(1,5)] + [(k,1,0,255,1,1,1) for k in (1,2)]
 rows=[]
 for kind,seed,alias,time,id,fault,steps in cases:
  r=fixture(ex,o,b,seed,alias,time,kind,id,fault)
  for step in range(steps):
   before=bytes(r);trapped=False
   try:execute(r,(0x80193478,0x801938E8,0x801939B0)[kind],() if kind==0 else (id,))
   except AssertionError as exc:
    assert fault and isinstance(exc.args[0],tuple) and exc.args[0][:2]==('division by zero in original fixture','0x8018f5c8'),exc
    trapped=True
   assert trapped==bool(fault)
   check=bytearray(r[:0x1F0000])
   for a,n in RANGES:check[a:a+n]=before[a:a+n]
   assert check==before[:0x1F0000],f'unexpected write {kind,seed,alias,step}'
   h=fnv(b''.join(r[a:a+n] for a,n in RANGES))
   rows.append((kind,seed,alias,time,id,step,fault,h))
 out=['/* Generated from original instructions by pe_transition_motion_oracle.py. */','static const struct { uint32_t kind,seed,alias,time,id,step,fault; uint64_t hash; } DAY1_motion_cases[]={']
 for *v,h in rows:out.append(' {'+','.join(f'0x{x:X}u' for x in v)+f',UINT64_C(0x{h:016X})'+'},')
 out.append('};')
 if '--write-header' in sys.argv:(ROOT/'pc_port/tests/retail_transition_motion_cases.h').write_text('\n'.join(out)+'\n')
 print(f'PASS: {len(rows)} original scene-motion/path history steps, including 6 zero-period prefixes')
if __name__=='__main__':main()
