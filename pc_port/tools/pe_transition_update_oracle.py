#!/usr/bin/env python3
"""Original whole transition update, real path/dialogue/volume-queue callees.

Message-list borrowed bytes are fixture inputs. The subtitle tail is checked
against its original last writers, not synthesized by the native implementation.
"""
import hashlib,random,struct,sys,itertools
from pe_battle_hud_oracle import ROOT,execute
from pe_m0000i_leaves_oracle import load_overlay
from pe_transition_camera_oracle import fixture as camera_fixture
from pe_transition_loader_oracle import fnv
RANGES=((0x9CDF0,4),(0x9CE80,0x70),(0x9D1F4,4),(0x9D26C,4),(0x9D2F4,4),(0xB0CD8,0x120),(0xB8628,0x300),(0xBCD80,0x20),(0xBCEA8,0xE0),(0x150000,512),(0x19BFCC,4),(0x19BFF0,0x210),(0x19C330,16),(0x19C810,16),(0x19CA68,0xE0),(0x19CC14,0x44),(0x1EA268,0x350),(0xB0E08,4),(0x160000,512))
def fixture(ex,o,b,c):
 seed,mode,timer,buttons,profile,clock=c;r=camera_fixture(ex,o,b,seed,0)
 for a,n in RANGES:r[a:a+n]=bytes(n)
 def sw(a,v):struct.pack_into('<I',r,a,v&0xffffffff)
 def sh(a,v):struct.pack_into('<H',r,a,v&65535)
 if profile&1:
  sw(0xB0E08,0x80160000);sw(0x160004,64);sw(0x16006C,(2<<22)|128)
  for i in range(2):sw(0x160084+i*12,256+i*32);sh(0x16008A+i*12,0x44C+i);sw(0x160100+i*32,0x4F414B41)
 sw(0x9CDF0,0x5FF);sw(0x9D1F4,buttons);sw(0x9D26C,0x20 if profile&1 else 1)
 sw(0x19BFF8,0xffffffff if profile<6 else 0)
 sh(0x19C034,mode);sh(0x19C026,timer);sh(0x19CC52,seed)
 for i in range(8):
  a=0x1EA378+i*52;r[a:a+4]=bytes((3,4,5,6));sw(a+4,4+i);sh(a+40,3+i)
  r[a+26:a+30]=bytes(((i+1)%8,(i-1)%8,int(i!=2),i+16))
 if profile&4:r[0x1EA378+seed*52+26:0x1EA378+seed*52+28]=b'\xff\xff'
 r[0x19C044]=profile&1;r[0x19C045]=(profile>>1)&1;r[0x19C046]=0
 r[0x19C040]=(0,1,2,255)[profile%4];r[0x19C041]=255;r[0x19C042]=3
 sw(0x19C048,clock);sw(0x19C04C,clock);sw(0x19CC1C,clock);sw(0x19C038,clock);sw(0x19C03C,clock)
 sw(0x19C000,profile&1);sw(0x19C004,seed);sh(0x19C13C,int(profile==7));sh(0x19C13E,profile%2)
 sw(0x19C018,int(profile%3==0));sw(0x19C01C,profile&1);sh(0x19C028,7)
 sw(0x19C140,14 if profile&2 else 0);sw(0x19C144,7)
 sw(0x19C0C8,clock);sw(0x19C0BC,clock);sh(0x19C030,clock);sh(0x19C0C4,11)
 r[0x19C0CC]=120;r[0x19C0CD]=profile%3
 for j in range(6):
  sw(0x1EA578+j*4,0x80150000+j*64)
  for i in range(3):sw(0x150000+j*64+28+i*4,(seed*34567+j*45678+i*56789)^0x80000000)
 for j in range(4):
  a=0xBCEA8+j*56;r[a]=int(j<profile%5);sh(a+16,7 if j==0 else 16+j);sw(a+12,0xABCDEF01)
 rng=random.Random(seed*100+profile)
 for delta in (0x19C05C,0x19C08C):
  for i in range(0,48,4):sw(delta+i,rng.getrandbits(32))
 for count in (0x19C050,0x19C054):sh(count,(0,1,2,-1,-32768,32767,0,0)[profile]);sh(count+2,(0,1,31,32,255,-1,0,3)[profile])
 for i in range(10):
  for j in range(3):sw(0x19CAA8+i*16+j*4,rng.getrandbits(32))
 menu=(1234,0xffff,0x5678,0x9876) if profile&1 else (0x8000,0x7fff,0,0xffff)
 struct.pack_into('<4H',r,0x1FEFCA,*menu)
 return r,menu

def main():
 ex,o,b=load_overlay();assert hashlib.sha256(o[0x801942FC-b:0x80195994-b]).hexdigest()=='c8bfdc9dd6a45967188d48a0dadf7f5778c888c2b0b4d43e66175d15d8f704ae'
 cases=[]
 for i,(m,t,key,p) in enumerate(itertools.product((0,1,2,-1),(0,1,2,16),(0,2,0x10,0x40,0x50,0x20000000,0x40000000,0x60000052),range(8))):
  cases.append(((i//5)%8,m,t,key,p,(0,3,199,200,201,0xFFFFFFFF)[(i//7)%6]))
 cases += [(s,10,0,0,p,t) for s,p,t in itertools.product((0,3),(0,1,2,3),(0,23,24,74,75,185,186,284,314,524,554,764,794,1004,1034,1049,1200,32767,65535))]
 faults=[0]*len(cases)
 cases += [(0,0,0,0,0,0),(0,10,0,0,0,74),(0,10,0,0,0,0),(0,10,0,0,0,0)]
 faults += [1,2,3,4]
 rows=[];patches=[]
 for number,c in enumerate(cases):
  r,menu=fixture(ex,o,b,c);first=len(patches);fault=faults[number];outer=int(c[1]==10 and c[4]==3)
  if fault>=3:
   rec=0x140000+struct.unpack_from('<I',r,0x140000+(75+fault)*4)[0];struct.pack_into('<H',r,rec+6,2)
  patches.extend((a+i,struct.unpack_from('<I',r,a+i)[0]) for a,n in RANGES for i in range(0,n,4) if struct.unpack_from('<I',r,a+i)[0])
  end=len(patches);before=bytes(r)
  try:
   execute(r,0x801942FC,initial_regs=({18:1,19:0xFFFFFF} if outer else {18:0x12345678,19:0x89ABCDEF}),stop_at=(0x800375E0,) if fault==1 else (0x801958D4,) if fault==2 else (),instruction_budget=100000)
   assert fault<3
  except AssertionError as exc:
   assert fault>=3 and isinstance(exc.args[0],tuple) and exc.args[0][:2]==('division by zero in original fixture','0x8018f5c8'),exc
  check=bytearray(r[:0x1f0000])
  for a,n in RANGES:check[a:a+n]=before[a:a+n]
  assert check==before[:0x1f0000],f'unexpected persistent write {number}'
  h=fnv(b''.join(r[a:a+n] for a,n in RANGES));rows.append((c[0],first,end,menu,fault,outer,h))
 out=['/* Original942FC + all real callees; borrowed list tails are explicit inputs. */','static const uint32_t DAY1_update_ranges[][2]={'+','.join('{0x%Xu,0x%Xu}'%v for v in RANGES)+'};','static const uint32_t DAY1_update_patches[][2]={']
 out.extend(' {0x%Xu,0x%Xu},'%v for v in patches);out.append('};')
 out.append('static const struct { unsigned seed,first,end,fault,outer; uint16_t menu[4]; uint64_t hash; } DAY1_update_cases[]={')
 for seed,first,end,menu,fault,outer,h in rows:out.append(f' {{{seed},{first},{end},{fault},{outer},'+'{'+','.join(str(x) for x in menu)+f'}},UINT64_C(0x{h:016X})'+'},')
 out.append('};')
 if '--write-header' in sys.argv:(ROOT/'pc_port/tests/retail_transition_update_cases.h').write_text('\n'.join(out)+'\n')
 print(f'PASS: {len(rows)} whole original update cases (4 stop prefixes), real path/dialogue/sound/volume-queue callees')
if __name__=='__main__':main()
