#!/usr/bin/env python3
"""Original M0023I flash callback with real sound/GTE/ring/fan callees.

Only undefined GPU packet padding is masked. No callback or callee formulas
are substituted for original instruction execution. The six borrowed stack
bytes are explicit inputs and outputs, including in paired-call cases. This
verifies the callback, not the outstanding production caller-stack binding.
"""
import hashlib,struct,sys
from pe_eve_charge_oracle import ROOT,execute,find_disc,read_form1
from pe_scripted_exit_oracle import words
from pe_m0023i_beam_oracle import fixture as beam_fixture,RANGES as BEAM_RANGES
ENTRIES=(0x8018F710,0x8018F710)
RANGES=BEAM_RANGES+((0xB8628,0x90),(0xBCD80,0x18),(0x9CDF0,4),(0x9D268,4),(0x1FEFB8,6))
CASES=[]
def case(**kw):CASES.append(dict(entry=0,**kw))
for kind in (-1,0,1,2):case(mode=0,kind=kind)
for group in (-1,0,32,64,65,128):case(mode=0,group=group,sound_found=1)
for mode in (-1,3,0x7FFFFFFF):case(mode=mode)
for state in (-1,0,1,2,3,4):
 for timer in (-32768,-1,0,1,7,8,31,32,32767):case(mode=1,state=state,timer=timer)
for state in (-1,0,1,2,3):
 for timer in (-1,0,1,7,8,31,32,32767):case(mode=2,state=state,timer=timer)
for bank in (0,1):
 for state in (0,1,2):
  for time in (0,1,16,31):case(mode=2,bank=bank,state=state,time=time)
for position in ((0,0,0),(400,200,700),(-6120,-32756,0),(-32768,32767,-32768),(32767,-32768,32767)):
 for timer in (0,1,7,8):case(mode=2,position=position,timer=timer)
for depth in (-100,0,31,32,16384,65535):case(mode=2,depth=depth)
for joint in (0,8,16,20):case(mode=2,joint=joint,state=1)
for timer in (0,1,7,8):
 for position in ((0,0,0),(-6120,-32756,0),(400,200,700)):
  CASES.append(dict(entry=1,mode=2,timer=timer,position=position))

def fixture(exe,overlay,c):
 r,s,_,ctrl=beam_fixture(exe,overlay,c)
 def put(a,b):r[a:a+len(b)]=b;s[a:a+len(b)]=b
 def sw(a,v):put(a,struct.pack('<I',v&0xFFFFFFFF))
 def sh(a,v):put(a,struct.pack('<H',v&65535))
 for a,n in ((0xB8628,0x90),(0xBCD80,0x18),(0x9CDF0,4),(0x9D268,4)):put(a,bytes(n))
 sw(0x9CDF0,0x405);sw(0x144008,c.get('group',32))
 sw(0xBCFA8,0x80148100);sw(0x148100,256)
 sh(0xB0DD0,100);sh(0xB0DD2,1000);put(0xB0DCE,bytes((20,120)))
 sh(0x150100,c.get('joint',8));sh(0x150102,c.get('timer',2));sh(0x150104,c.get('state',0))
 if c['entry']==1:
  sh(0x150140,16);sh(0x150142,c.get('timer',2));sh(0x150144,c.get('state',0))
 for j,v in enumerate(c.get('position',(-6120,-32756,0))):sh(0x1FEFB8+j*2,v)
 if c.get('sound_found'):
  sw(0x17206C,(2<<22)|0x100)
  for j,key in enumerate((0x5AE,0x5AF)):
   sw(0x172104+j*12,0x200+j*0x100);sh(0x17210A+j*12,key);sw(0x172200+j*0x100,0x4F414B41)
 return r,s,(c.get('mode',2)&0xFFFFFFFF,0x80150100),ctrl

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
        assert length in (0,1,6,7,8,9,12),(hex(p),length)
        if length==0:p+=4;continue
        if length==6:
            r[p+15]=0;r[p+23]=0
        if length==8:
            for j in (15,23,31):r[p+j]=0
            if p not in linked:r[p+32:p+36]=bytes(4)
        if length in (1,6,8) and p not in linked:r[p:p+3]=bytes(3)
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
 assert hashlib.sha256(overlay[0x8018F710-0x8018EFE8:0x8018FC14-0x8018EFE8]).hexdigest()=='6bee6882d0a259e2d326e4bd70941ff28d6622266e24d4f098dcb6cf5cc228aa'
 _,s,_,_=fixture(exe,overlay,CASES[0]);base=words(s);common=[(i*4,v) for i,v in enumerate(base) if v]
 patches=[];cases=[]
 for k,c in enumerate(CASES):
  r,s,args,ctrl=fixture(exe,overlay,c);first=len(patches)
  patches.extend((i*4,v) for i,(v,b) in enumerate(zip(words(s),base)) if v!=b)
  seed=c.get('seed',1)
  regs=execute(r,ENTRIES[c['entry']],args,initial_cop_control=ctrl,bios_seed=seed)
  if c['entry']==1:
   assert regs[2]==0
   regs=execute(r,0x8018F710,(2,0x80150140),initial_cop_control=ctrl)
  h=fingerprint(r);cases.append((c['entry'],first,len(patches),args,seed,regs[2],h))
  print(k,c,hex(regs[2]),hex(h),flush=True)
  if '--dump' in sys.argv:(ROOT/f'local/live/m0023i-flash-oracle-{k}.bin').write_bytes(r)
 out=['/* Generated original M0023I flashes with explicit borrowed stack bytes. */']
 for name,rows in (('ranges',RANGES),('common',common),('patches',patches)):
  out.append(f'static const uint32_t DAY1_flash_{name}[][2]={{')
  out += [f'    {{0x{a:X}u,0x{b:X}u}},' for a,b in rows];out.append('};')
 out.append('static const struct { unsigned entry,first,end; uint32_t args[2],seed,result; uint64_t hash; } DAY1_flash_cases[]={')
 for e,a,b,args,seed,v,h in cases:
  params=','.join(f'0x{x:08X}u' for x in args)
  out.append(f'    {{{e},{a},{b},{{{params}}},0x{seed:08X}u,0x{v:08X}u,UINT64_C(0x{h:016X})}},')
 out.append('};')
 if '--write-header' in sys.argv:(ROOT/'pc_port/tests/retail_m0023i_flash_cases.h').write_text('\n'.join(out)+'\n')
 print(f'PASS: {len(cases)} complete original M0023I flash cases')
if __name__=='__main__':main()
