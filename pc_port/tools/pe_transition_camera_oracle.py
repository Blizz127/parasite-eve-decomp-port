#!/usr/bin/env python3
"""Original camera selection and stepping with real package/path/atan code."""
import hashlib,itertools,struct,sys
from pe_battle_hud_oracle import ROOT,execute
from pe_m0000i_leaves_oracle import load_overlay
from pe_transition_loader_oracle import fnv
RANGES=((0x19C020,0xA0),(0x19C330,16),(0x19C810,16),(0x19BFCC,4))
SHIFTS=(0,1,15,16,31,32,255,0x10001)
def fixture(exe,o,b,seed,variant):
 r=bytearray(0x200000);r[0x10000:0x10000+len(exe)-0x800]=exe[0x800:];r[b&0x1FFFFF:(b&0x1FFFFF)+len(o)]=o
 for a,n in RANGES:r[a:a+n]=bytes((seed+i*29)&255 for i in range(n))
 struct.pack_into('<I',r,0x1D0268,(0x140000-0x1D0260)&0xFFFFFFFF)
 for i in range(-1,257):
  rec=0x141000+(i+1)*96;struct.pack_into('<I',r,0x140000+i*4,rec-0x140000)
  count=3+(i+seed+8)%7;struct.pack_into('<H',r,rec+6,count)
  for j in range(9):
   for k in range(3):struct.pack_into('<H',r,rec+8+j*8+k*2,(i*7919+j*12347+k*29009+seed*4711)&65535)
 v=variant&65535;v=v-65536 if v&32768 else v
 r[0x1EA378+v*52]=3;r[0x1EA379+v*52]=4
 return r
def main():
 exe,o,b=load_overlay();rows=[]
 for a,z,digest in ((0x80195994,0x80195BC8,'9fa29ecbb36774667554d798102011046dae04f76180c4c30ffe3446c0bdcab7'),(0x80195D3C,0x80195E4C,'0b3f22c796fb92edf367a85e2d9a63a17a68aad1ec9bd0c027c5a7b770b240bc'),(0x80195E4C,0x80195F6C,'436f4b9066142b100822f3f363ec3a2e794db7246cc906aeb8806cf441108541')):
  assert hashlib.sha256(o[a-b:z-b]).hexdigest()==digest
 for seed,variant,time in itertools.product(range(8),(0,9,0xFFFFFFFF,0x10009),(0,255,1280,0xFFFFFFFF)):
  r=fixture(exe,o,b,seed,variant)
  for step in range(16):
   if step in (0,15):fn=0x80195994;args=(variant,SHIFTS[seed],SHIFTS[7-seed],time)
   elif step in (1,14):fn=0x80195E4C;args=((0xFFFFFFFF if seed&1 else 5),123,456,time)
   else:fn=0x80195D3C;args=()
   execute(r,fn,args)
   h=fnv(b''.join(r[a:a+n] for a,n in RANGES));rows.append((seed,variant,time,step,h))
 out=['/* Original camera path history, including original6EC6C/F55C/79FB4. */','static const struct { uint32_t seed,variant,time,step; uint64_t hash; } DAY1_camera_cases[]={']
 for seed,v,t,step,h in rows:out.append(f' {{{seed},0x{v:X}u,0x{t:X}u,{step},UINT64_C(0x{h:016X})}},')
 out.append('};')
 out.append('static const struct { unsigned fn,second; uint64_t hash; } DAY1_camera_faults[]={')
 for f,second in itertools.product(range(3),range(2)):
  r=fixture(exe,o,b,0,0);r[0x19C040]=3;r[0x19C041]=255;r[0x19C042]=3
  ids=(3,4) if f==0 else (4,3)
  rec=0x140000+struct.unpack_from('<I',r,0x140000+ids[second]*4)[0]
  struct.pack_into('<H',r,rec+6,2)
  fn=(0x80195994,0x80195D3C,0x80195E4C)[f]
  args=(0,1,1,256) if f==0 else (() if f==1 else (3,0,0,256))
  regs=execute(r,fn,args,stop_at=(0x8018F5C8,))
  # If this is the second sample, let the first valid sample finish.
  if regs[4]:
   ret=regs[31]
   regs=execute(r,0x8018F5C8,initial_regs=dict(enumerate(regs)),stop_at=(ret,))
   regs=execute(r,ret,initial_regs=dict(enumerate(regs)),stop_at=(0x8018F5C8,))
  assert regs[4]==0
  h=fnv(b''.join(r[a:a+n] for a,n in RANGES))
  out.append(f' {{{f},{second},UINT64_C(0x{h:016X})}},')
 out.append('};')
 if '--write-header' in sys.argv:(ROOT/'pc_port/tests/retail_transition_camera_cases.h').write_text('\n'.join(out)+'\n')
 print('PASS:',len(rows),'original camera history steps; no provider substitution')
if __name__=='__main__':main()
