#!/usr/bin/env python3
"""Complete original M0023I effects, including real pool/GTE/ribbon/sprite callees.

Only undefined GPU packet padding is masked. No callback or callee formulas
are substituted for original instruction execution.
"""
import hashlib,struct,sys
from pe_eve_charge_oracle import ROOT,execute,find_disc,read_form1,fixture as charge_fixture,RANGES as CHARGE_RANGES
from pe_scripted_exit_oracle import words
ENTRIES=(0x8018F3C8,0x8018F004,0x800CE688,0x800CE78C,0x80190644)
RANGES=CHARGE_RANGES+((0x190758,8),)
CASES=[]
def case(entry=0,**kw):CASES.append(dict(entry=entry,**kw))
for kind in (-1,0,1,2):case(mode=0,kind=kind)
case(mode=3)
for joint in (0,8,16,20):case(mode=2,joint=joint)
for timer in (-1,0,1,2,32767):
 for interval in (-1,0,1,2,32767):case(mode=1,timer=timer,interval=interval)
for seed in (1,2,3,7,12345,0xFFFFFFFF):case(mode=1,timer=5,interval=1,seed=seed)
case(mode=1,timer=5,interval=1,full=1)
for state in (-1,0,1,2):
 for time in (-1,0,1,3,4,7,8,9,32767):case(1,mode=1,state=state,time=time)
case(1,mode=0);case(1,mode=3)
for state in (-1,0,1,2):
 for bank in (0,1):
  for time in (0,1,3,4,7,8):case(1,mode=2,state=state,bank=bank,time=time)
for state in (0,1):
 for depth in (-100,0,31,32,65535):case(1,mode=2,state=state,depth=depth)
 case(1,mode=2,state=state,texture_type=4,alternate=1)
 case(1,mode=2,state=state,angle=-32768)
for entry in (2,3):
 for state in (0,1):
  for time in (0,3,4,7,8):case(entry,state=state,time=time,particles=3)

for mode in (-1,0,1,2,3,0x7FFFFFFF):case(4,mode=mode)

def fixture(exe,overlay,c):
 r,s,_,ctrl=charge_fixture(exe,overlay,dict(c,entry=0))
 def put(a,b):r[a:a+len(b)]=b;s[a:a+len(b)]=b
 def sw(a,v):put(a,struct.pack('<I',v&0xFFFFFFFF))
 def sh(a,v):put(a,struct.pack('<H',v&65535))
 for a,n in ((0x960BC,0x180),(0xC22A0,4)):
  put(a,exe[a-0x10000+0x800:a-0x10000+0x800+n])
 for a in (0x18F3C8,0x18F3CC,0x18F49C,0x18F708):
  put(a,overlay[a-0x18EFE8:a-0x18EFE8+4])
 sh(0xE11EA,1);sh(0xE2852,0x11)
 sh(0x15001E,c.get('kind',0));sh(0x150020,c.get('interval',1))
 sw(0x150034,0x80150200);put(0x150100,bytes(16))
 sh(0x15010C,c.get('timer',0));sh(0x15010E,c.get('joint',8))
 sw(0x150200,20);sw(0x150204,12);sw(0x150208,0x8018F004)
 for i in range(12):
  p=0x15020C+i*20;put(p,bytes(20))
  sh(p,int(bool(c.get('full')) or i<c.get('particles',0)));sh(p+2,c.get('time',3)+i)
  for j,v in enumerate((c.get('angle',100)+i*17,-50+i*9,0,c.get('state',0),-3,4,5,0)):
   sh(p+4+j*2,v)
 for j,v in enumerate((300,-400,700,0)):sh(0x190758+j*2,v)
 args=((c.get('mode',1),0x80150100),(c.get('mode',1),0x80150210),(0x80150200,),(0x80150200,),(c.get('mode',1),0x80150100))[c['entry']]
 ctrl[29]=0x155
 return r,s,tuple(v&0xFFFFFFFF for v in args),ctrl

def fingerprint(r):
    r=bytearray(r);bank=struct.unpack_from('<I',r,0x9CDDC)[0]
    base=0x160000+bank*0x2000;size=struct.unpack_from('<I',r,0x9CDD8)[0];linked=set()
    for i in range(4096):
        p=struct.unpack_from('<I',r,0x164000+bank*0x4000+i*4)[0]&0xFFFFFF
        while base<=p<base+size and p not in linked:
            linked.add(p);p=struct.unpack_from('<I',r,p)[0]&0xFFFFFF
    p=base
    while p<base+size:
        length=r[p+3]
        assert length in (7,9,12),(hex(p),length)
        if length==9 and (r[p+7]&0xFC)==0x2C:
            r[p+30:p+32]=bytes(2);r[p+38:p+40]=bytes(2)
            if p not in linked:r[p:p+3]=bytes(3);r[p+32:p+36]=bytes(4)
        p+=(length+1)*4
    h=14695981039346656037
    for a,n in RANGES:
        for b in r[a:a+n]:h=((h^b)*1099511628211)&0xFFFFFFFFFFFFFFFF
    return h

def main():
 exe=(ROOT/'build/disc1.candidate.exe').read_bytes()
 assert hashlib.sha1(exe).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
 overlay=read_form1(find_disc(ROOT),13904,172)
 for a,b,h in ((0x8018F004,0x8018F3C8,'2e2c57c73f6b63fab5ffa871e5b66be5fd3ca84e29c158e3f5259b87d4f6a221'),
               (0x8018F3C8,0x8018F710,'b39ab2a332607673c32fa36e41b2b2428dcc58020316681b553df9ab4b79863b'),
               (0x80190644,0x80190670,'06bd7eb3f2fba46e855437b0c3bbc727f28f6bbb34f799f9e73bc7d373d7364c')):
  assert hashlib.sha256(overlay[a-0x8018EFE8:b-0x8018EFE8]).hexdigest()==h
 _,s,_,_=fixture(exe,overlay,CASES[0]);base=words(s);common=[(i*4,v) for i,v in enumerate(base) if v]
 patches=[];cases=[]
 for k,c in enumerate(CASES):
  r,s,args,ctrl=fixture(exe,overlay,c);first=len(patches)
  patches.extend((i*4,v) for i,(v,b) in enumerate(zip(words(s),base)) if v!=b)
  seed=c.get('seed',1)
  regs=execute(r,ENTRIES[c['entry']],args,initial_cop_control=ctrl,bios_seed=seed)
  h=fingerprint(r);cases.append((c['entry'],first,len(patches),args,seed,regs[2],h))
  print(k,c,hex(regs[2]),hex(h),flush=True)
  if '--dump' in sys.argv:(ROOT/f'local/live/m0023i-oracle-{k}.bin').write_bytes(r)
 out=['/* Generated complete original M0023I effects and real callees. */']
 for name,rows in (('ranges',RANGES),('common',common),('patches',patches)):
  out.append(f'static const uint32_t DAY1_effect_{name}[][2]={{')
  out += [f'    {{0x{a:X}u,0x{b:X}u}},' for a,b in rows];out.append('};')
 out.append('static const struct { unsigned entry,first,end; uint32_t args[2],seed,result; uint64_t hash; } DAY1_effect_cases[]={')
 for e,a,b,args,seed,v,h in cases:
  params=','.join(f'0x{x:08X}u' for x in args)
  out.append(f'    {{{e},{a},{b},{{{params}}},0x{seed:08X}u,0x{v:08X}u,UINT64_C(0x{h:016X})}},')
 out.append('};')
 if '--write-header' in sys.argv:(ROOT/'pc_port/tests/retail_m0023i_effect_cases.h').write_text('\n'.join(out)+'\n')
 print(f'PASS: {len(cases)} complete original M0023I effect cases')
if __name__=='__main__':main()
