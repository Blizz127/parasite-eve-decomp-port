#!/usr/bin/env python3
"""Execute the whole original96498 constructor with real pure callees.

Only scene/display initialization, window setup and GTE setters use explicit
provider contracts. Pool allocation, packets, resource lookup, path sampling
and camera setup execute original instructions. No production emulation.
"""
import hashlib,itertools,struct,sys
from pe_battle_hud_oracle import ROOT,execute
from pe_m0000i_leaves_oracle import load_overlay
from pe_transition_loader_oracle import fnv

PROVIDERS={0x80191854:0,0x800371B0:1,0x80078E34:1,0x80078E64:1,
           0x80078FC4:3,0x80077E64:3,0x80078FE4:3}
RANGES=((0x91648,32),(0x19BFCC,0xE44),(0x1E4A80,0x5DAC))
COUNTS=((-1,0,3,4,5,6,7,8,0),(0,)*9,(8,)*9)

def put(r,a,v):struct.pack_into('<I',r,a&0x1FFFFF,v&0xFFFFFFFF)
def half(r,a,v):struct.pack_into('<H',r,a&0x1FFFFF,v&65535)
def word(r,a):return struct.unpack_from('<I',r,a&0x1FFFFF)[0]

def fixture(exe,o,b,seed,mode,resume,profile,flags):
 r=bytearray(0x200000);r[0x10000:0x10000+len(exe)-0x800]=exe[0x800:]
 r[b&0x1FFFFF:(b&0x1FFFFF)+len(o)]=o
 for a,n in RANGES:r[a:a+n]=bytes((seed*31+i*29+(i>>8))&255 for i in range(n))
 for a,v in ((0x19BFF4,mode),(0x19BFF8,resume),(0x19BFFC,0x12340005),
             (0x19C008,seed&1),(0x19C020,0),(0xA77FC,flags)):
  put(r,a,v)
 half(r,0x19CC52,seed%10);r[0x19C017]=30+seed%8
 # Relative package tables. All model IDs used by the constructor are real
 # lookups into synthetic resource records, including their metadata word.
 for i in range(2):put(r,0x19CE10+4*i,0x150000+i*0x4000-0x19CE10)
 for i in range(3):put(r,0x1D0260+4*i,(0x140000 if i==2 else 0x158000+i*0x100)-0x1D0260)
 for i in range(128):
  off=0x400+i*64;put(r,0x150000+i*4,off)
  put(r,0x150000+off+0x30,0xABCD0000+seed*131+i*17)
 # Record zero supplies ten SVECTOR placements; 1..9 control the nine
 # repeated object lists. Camera tracks use 10..73 and remain nondegenerate.
 for i in range(128):
  rec=0x141000+i*128;put(r,0x140000+i*4,rec-0x140000)
  count=10 if i==0 else COUNTS[profile][i-1] if i<=9 else 3+(i+seed)%8
  half(r,rec+6,count)
  for j in range(12):
   for k in range(3):half(r,rec+8+j*8+k*2,seed*4711+i*7919+j*12347+k*29009)
 return r

def original(r,mutate,profile):
 before=bytes(r);calls=[];tim=0
 regs=execute(r,0x80196498,stop_at=PROVIDERS,instruction_budget=1000000)
 while regs[31]:
  ret=regs[31];fn=0x80000000|((word(r,ret-8)&0x3FFFFFF)<<2)
  n=PROVIDERS[fn];args=regs[4:4+n]
  calls.append((fn,*args,*([0]*(3-n))))
  if fn==0x800371B0:
   tim+=1
   if mutate and tim==1:put(r,0x19C008,1)
  regs[2]=0
  regs=execute(r,ret,stop_at=PROVIDERS,initial_regs=dict(enumerate(regs)),instruction_budget=1000000)
 assert tim==(2 if mutate and not (word(before,0x19C008)) else 1)
 # Every persistent write must be represented by the state fingerprint.
 outside=bytearray(r[:0x1F0000])
 for a,n in RANGES:outside[a:a+n]=before[a:a+n]
 assert outside==before[:0x1F0000],next((hex(i) for i,(v,w) in enumerate(zip(outside,before)) if v!=w),'')
 count=word(r,0x19C020);expected=2*sum(max(0,x) for x in COUNTS[profile])
 assert count==expected,(count,expected)
 # Follow the original used-list links and require each slot once, with
 # the full profile reaching 199 of the 200 available pool slots.
 used=[];index=struct.unpack_from('<h',r,0x1E4A88+200*4+2)[0]
 while index!=201:
  assert 0<=index<200 and index not in used
  used.append(index);index=struct.unpack_from('<h',r,0x1E4A88+index*4+2)[0]
 assert len(used)==55+count,(len(used),count)
 return len(calls),fnv(b''.join(struct.pack('<4I',*c) for c in calls)),fnv(b''.join(r[a:a+n] for a,n in RANGES)),len(used)

def boundaries(exe,o,b):
 rows=[]
 for stop,bad_id in ((1,0),(7,0),(8,0),(0,27),(0,26),(0,64)):
  r=fixture(exe,o,b,0,0,0,0,0)
  if bad_id:half(r,0x140000+word(r,0x140000+bad_id*4)+6,2)
  calls=[];tim=0;stops=(*PROVIDERS,0x8018F5C8)
  regs=execute(r,0x80196498,stop_at=stops,instruction_budget=1000000)
  while True:
   ret=regs[31];assert ret
   fn=0x80000000|((word(r,ret-8)&0x3FFFFFF)<<2)
   if fn==0x8018F55C:
    if regs[4]==0:break
    # Keep HI/LO alive from this valid DIV through the whole sampler.
    regs=execute(r,0x8018F5C8,initial_regs=dict(enumerate(regs)),stop_at=(ret,))
   else:
    n=PROVIDERS[fn];calls.append((fn,*regs[4:4+n],*([0]*(3-n))))
    if fn==0x800371B0:
     tim+=1
     if tim==1:put(r,0x19C008,1)
    if len(calls)==stop:break
    regs[2]=0
   regs=execute(r,ret,stop_at=stops,initial_regs=dict(enumerate(regs)),instruction_budget=1000000)
  trace=fnv(b''.join(struct.pack('<4I',*c) for c in calls))
  state=fnv(b''.join(r[a:a+n] for a,n in RANGES))
  rows.append((stop,bad_id,len(calls),trace,state))
 out=['/* Original constructor stop prefixes: provider stop and zero-period DIV. */',
      'static const struct { unsigned stop,bad_id,calls; uint64_t trace,state; } DAY1_constructor_faults[]={']
 for stop,bad_id,calls,trace,state in rows:out.append(f' {{{stop},{bad_id},{calls},UINT64_C(0x{trace:016X}),UINT64_C(0x{state:016X})}},')
 out.append('};')
 (ROOT/'pc_port/tests/retail_transition_constructor_faults.h').write_text('\n'.join(out)+'\n')
 print('PASS:',len(rows),'original constructor stop prefixes')

def main():
 exe,o,b=load_overlay()
 assert hashlib.sha256(o[0x80196498-b:0x80197BA0-b]).hexdigest()=='c946ea3080c7e550f1ce346ee26e40cb8006fcbcd4f288d31ef9202cbb10562e'
 if '--write-boundaries' in sys.argv:
  boundaries(exe,o,b);return
 rows=[]
 for seed,mode,resume,profile,mutate in itertools.product(range(10),(0,1,7,0xFFFFFFFF),(0,1,2),range(3),range(2)):
  flags=(0,0xFFFFFFFF,0x55555555,0xAAAAAAAA)[seed%4]
  result=original(fixture(exe,o,b,seed,mode,resume,profile,flags),mutate,profile)
  rows.append((seed,mode,resume,profile,flags,mutate,*result))
 out=['/* Whole original96498 with original pool/resource/path/camera callees. */',
      'static const struct { uint32_t seed,mode,resume,profile,flags,mutate,calls; uint64_t trace,state; unsigned objects; } DAY1_constructor_cases[]={']
 for row in rows:
  out.append(' {'+','.join(f'0x{x:X}u' for x in row[:7])+f',UINT64_C(0x{row[7]:016X}),UINT64_C(0x{row[8]:016X}),{row[9]}'+'},')
 out.append('};')
 if '--write-header' in sys.argv:(ROOT/'pc_port/tests/retail_transition_constructor_cases.h').write_text('\n'.join(out)+'\n')
 print('PASS:',len(rows),'whole original constructor cases; object counts',sorted(set(row[-1] for row in rows)),flush=True)
if __name__=='__main__':main()
