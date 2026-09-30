#!/usr/bin/env python3
"""Complete original M0023I beam callback and its drawing/collision callees.

Only undefined GPU packet padding is masked. No callback or callee formulas
are substituted for original instruction execution.
"""
import hashlib,struct,sys
from pe_eve_charge_oracle import ROOT,execute,find_disc,read_form1
from pe_scripted_exit_oracle import words
from pe_eve_beam_oracle import fixture as beam_fixture,RANGES as BEAM_RANGES
ENTRIES=(0x8018FC14,)
RANGES=BEAM_RANGES+((0x1906F0,0x74),)
CASES=[]
def case(**kw):CASES.append(dict(entry=0,**kw))
for kind in (-1,0,1,2):
 for action in (0,1,2,255):case(mode=0,kind=kind,action=action)
case(mode=0,notify=0);case(mode=0,owner_null=1);case(mode=0,record_null=1)
for mode in (-1,3,0x7FFFFFFF):case(mode=mode)
for state in (-1,0,1,2,3,4):
 for timer in (-32768,-1,0,3,4,6,7,14,15,30,31,32767):case(mode=1,state=state,timer=timer)
for joint in (0,8,16,20):case(mode=1,joint=joint)
for time in (-1,0,1,2):case(mode=1,state=2,time=time)
for notify in (0,1):
 for latch in (0,1):
  for flags in (0x01000000,0x91000000):case(mode=1,notify=notify,latch=latch,owner_flags=flags,aya=(-400,0,756))
for aya in ((10000,0,10000),(-400,0,756),(-400,12345,756)):
 case(mode=1,aya=aya)
for bank in (0,1):
 for time in (0,1,3,8,16,31,32,100):case(mode=2,bank=bank,time=time)
for amplitude in (-2147483648,-32768,-1,0,1,512,4096,2147483647):case(mode=2,amplitude=amplitude)
for radius in (-32768,-1,0,1,4096,32767):case(mode=2,radius=radius)
for length in (-2147483648,-32768,-1,0,1,32767,2147483647):case(mode=2,length=length)
for depth in (-100,0,31,32,16384,65535):case(mode=2,depth=depth)
for color_time in (0,7,8,15,16,31,32,100):case(mode=2,color_time=color_time)
for matrix in ((4096,0,0,0,4096,0,0,0,4096),(32767,-32768,12345,32767,32767,-32768,32767,-100,32767)):
 case(mode=2,matrix=matrix)
for bank in (0,1):
 for time in (0,1,3,8,16,31,32,100):case(mode=2,bank=bank,time=time,depth=512)
for state,timer in ((0,4),(1,7),(2,31),(3,14),(3,15)):
 case(mode=1,state=state,timer=timer,aya=(-400,0,756))

def fixture(exe,overlay,c):
 r,s,_,ctrl=beam_fixture(exe,overlay,dict(c,entry=0))
 def put(a,b):r[a:a+len(b)]=b;s[a:a+len(b)]=b
 def sw(a,v):put(a,struct.pack('<I',v&0xFFFFFFFF))
 def sh(a,v):put(a,struct.pack('<H',v&65535))
 for a,n in ((0xC2260,8),(0xC2290,16)):
  put(a,exe[a-0x10000+0x800:a-0x10000+0x800+n])
 put(0x1906F0,overlay[0x1906F0-0x18EFE8:0x190764-0x18EFE8])
 for a in (0x18F3C8,0x18F3CC,0x18F49C,0x18F708):
  put(a,overlay[a-0x18EFE8:a-0x18EFE8+4])
 put(0x150100,bytes((i*17+3)&255 for i in range(48)))
 for j,v in enumerate((300,-400,700)):sh(0x150100+j*2,v)
 sw(0x150118,c.get('length',1536));sw(0x15011C,c.get('amplitude',4096))
 sw(0x150120,c.get('joint',8));sw(0x150124,c.get('latch',1))
 sh(0x150128,c.get('state',0));sh(0x15012A,c.get('timer',0))
 sh(0x15012C,c.get('radius',4096));sh(0x15012E,c.get('color_time',16))
 sh(0x15001E,c.get('kind',0))
 sw(0x172108,0xC5463704);sw(0x190760,0x80173000)
 if c.get('owner_null'):sw(0x150008,0)
 if c.get('record_null'):sw(0x140000,0)
 if 'matrix' in c:
  for j,v in enumerate(c['matrix']):sh(0x146000+j*2,v)
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
        assert length in (0,1,6,7,9,12),(hex(p),length)
        if length==0:p+=4;continue
        if length==6:
            r[p+15]=0;r[p+23]=0
        if length in (1,6) and p not in linked:r[p:p+3]=bytes(3)
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
 assert hashlib.sha256(overlay[0x8018FC14-0x8018EFE8:0x80190644-0x8018EFE8]).hexdigest()=='7daa5e3bc20015de550bd7cab4a073d4e0b5ddb2f65ff52a14e9955025fb7458'
 _,s,_,_=fixture(exe,overlay,CASES[0]);base=words(s);common=[(i*4,v) for i,v in enumerate(base) if v]
 patches=[];cases=[];hit_cases=0;formats=set()
 for k,c in enumerate(CASES):
  r,s,args,ctrl=fixture(exe,overlay,c);first=len(patches)
  patches.extend((i*4,v) for i,(v,b) in enumerate(zip(words(s),base)) if v!=b)
  seed=c.get('seed',1)
  regs=execute(r,ENTRIES[c['entry']],args,initial_cop_control=ctrl,bios_seed=seed)
  if args[0]==1 and c.get('latch',1) and struct.unpack_from('<I',r,0x150124)[0]==0:hit_cases+=1
  if args[0]==2:
   p=0x160000+c.get('bank',0)*0x2000;end=p+struct.unpack_from('<I',r,0x9CDD8)[0]
   while p<end:
    length=r[p+3]
    if length:formats.add((length,r[p+7]&0xFC))
    p+=(length+1)*4
  h=fingerprint(r);cases.append((c['entry'],first,len(patches),args,seed,regs[2],h))
  print(k,c,hex(regs[2]),hex(h),flush=True)
  if '--dump' in sys.argv:(ROOT/f'local/live/m0023i-beam-oracle-{k}.bin').write_bytes(r)
 assert hit_cases>=10,('collision coverage',hit_cases)
 assert {(7,0x24),(9,0x2C),(9,0x34),(12,0x3C)}<=formats,('mesh coverage',formats)
 print(f'Coverage: {hit_cases} consumed collision latches; all four mesh packet formats')
 out=['/* Generated complete original M0023I beams and real callees. */']
 for name,rows in (('ranges',RANGES),('common',common),('patches',patches)):
  out.append(f'static const uint32_t DAY1_beam_{name}[][2]={{')
  out += [f'    {{0x{a:X}u,0x{b:X}u}},' for a,b in rows];out.append('};')
 out.append('static const struct { unsigned entry,first,end; uint32_t args[2],seed,result; uint64_t hash; } DAY1_beam_cases[]={')
 for e,a,b,args,seed,v,h in cases:
  params=','.join(f'0x{x:08X}u' for x in args)
  out.append(f'    {{{e},{a},{b},{{{params}}},0x{seed:08X}u,0x{v:08X}u,UINT64_C(0x{h:016X})}},')
 out.append('};')
 if '--write-header' in sys.argv:(ROOT/'pc_port/tests/retail_m0023i_beam_cases.h').write_text('\n'.join(out)+'\n')
 print(f'PASS: {len(cases)} complete original M0023I beam cases')
if __name__=='__main__':main()
