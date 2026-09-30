#!/usr/bin/env python3
"""Execute the original M0013I projectile/particle graph directly from Disc 1.

No reconstructed callback formulas or substituted callees in the oracle.
Fixtures cover charging, launch, motion, collision, fade, pool saturation,
particle expiry and both packet banks. Only GPU stack padding is masked.
"""
import hashlib
import struct
import sys
from pe_eve_charge_oracle import ROOT, execute, find_disc, read_form1, fixture as charge_fixture
from pe_eve_charge_oracle import RANGES, fingerprint as packet_fingerprint
from pe_scripted_exit_oracle import words

EXTRA_RANGES=((0x9D254,4),(0xB0DD0,4),(0xB8628,0x90),(0xBCD80,0x18),
              (0x9CDF0,4),(0x9D268,4),(0x18FCEC,12))
ENTRIES=(0x8018F20C,0x8018F004,0x800C6B90,0x800D3F64,0x800CE688,0x800CE78C,0x800187C0)
CASES=[]
def case(entry=0,**kw):CASES.append(dict(entry=entry,**kw))
for kind in (-1,0,1,2,3):case(mode=0,kind=kind)
case(mode=3)
for timer in (-1,0,1,50,51,52,32767):
    for joint in (0,1):case(mode=1,timer=timer,joint=joint)
case(mode=1,timer=51,scripted=1);case(mode=1,full=1)
for angle in (-32768,-1024,-1,0,1024,2048,32767):
    for tick in (2,3):case(mode=1,state=1,angle=angle,time=tick)
case(mode=1,state=1,inside=0);case(mode=1,state=1,hit=1)
case(mode=1,state=1,hit=1,scripted=1);case(mode=1,state=1,hit=1,scripted=1,attack=0x11000000)
case(mode=1,state=1,full=1);case(mode=1,state=1,speed=-32768)
for timer in (-1,0,14,15,16,32767):case(mode=1,state=2,timer=timer)
case(mode=1,state=3)
for bank in (0,1):
    for state in (-1,0,1,2,3):
        for timer in (0,7,15):case(mode=2,bank=bank,state=state,timer=timer)
for depth in (-100,0,31,65535):case(mode=2,depth=depth)
for time in (-1,0,1,7,13,14,15):
    case(1,mode=1,time=time);case(1,mode=2,time=time)
case(1,mode=0);case(1,mode=3);case(1,mode=2,bank=1,texture_type=4,alternate=1)
for distance in (0,56,57,58,100,32767,-32768):case(2,distance=distance)
case(2,distance=100,radius=50);case(3)
case(4,particles=3);case(4,particles=3,particle_time=14);case(5,particles=3)
case(mode=0,sound_found=1);case(3,sound_found=1)
case(mode=1,state=1,inside=0,sound_handle=0x405)
case(mode=1,state=1,hit=1,scripted=1,sound_handle=0x405)
case(mode=1,state=1,inside=0,hit=1,sound_handle=0x405)

for command_values in ((200,400,1),(0,0,0),(0x80012000,-1,0x12345)):
    case(6,command_values=command_values)

def fixture(exe,overlay,c):
    r,s,args,ctrl=charge_fixture(exe,overlay,dict(c,entry=0,sound=1))
    def put(a,b):r[a:a+len(b)]=b;s[a:a+len(b)]=b
    def sw(a,v):put(a,struct.pack('<I',v&0xFFFFFFFF))
    def sh(a,v):put(a,struct.pack('<H',v&65535))
    for a,n in ((0x960BC,0x200),(0xC2258,24)):
        put(a,exe[a-0x10000+0x800:a-0x10000+0x800+n])
    put(0x18EFFC,overlay[0x14:0x1C])
    for a in (0x18F20C,0x18F210,0x18F214,0x18FC4C):
        put(a,overlay[a-0x18EFE8:a-0x18EFE8+4])
    for k in range(26):
        m=0x146000+k*32
        for j,v in enumerate((0,0,4096,0,4096,0,-4096,0,0)):sh(m+j*2,v)
        for j,v in enumerate((300,-400,700)):sw(m+20+j*4,v+k*7)
    for j,v in enumerate((300,-400,700)):sh(0x140268+j*2,v)
    sw(0x144000,c.get('attack',0x01000000));sw(0x144018,0x80144100);put(0x144100,b'\x01')
    put(0x150019,bytes((c.get('scripted',0),)));sh(0x15001E,c.get('kind',0))
    sw(0x150034,0x80150200)
    put(0x150100,bytes(0x120))
    for j,v in enumerate((300,-400,700)):sh(0x150100+j*2,v)
    sh(0x150108,c.get('state',0));sh(0x15010A,c.get('timer',12));sh(0x15010C,c.get('sound_handle',-1))
    sh(0x15010E,c.get('angle',713));sh(0x150110,4096);sh(0x150112,96)
    sw(0x150200,16);sw(0x150204,16);sw(0x150208,0x8018F004)
    for i in range(16):
        p=0x15020C+i*16
        sh(p,int(bool(c.get('full')) or i<c.get('particles',0)));sh(p+2,c.get('particle_time',7)+i)
        for j,v in enumerate((300+i*3,-400,700)):sh(p+4+j*2,v)
        sw(p+12,4096)
    sw(0x150800,c.get('speed',200));sw(0x150804,400);sw(0x150808,c.get('joint',1))
    sw(0x9D254,0x80142000);sw(0x142000,0x80143000)
    for j,v in enumerate((300 if c.get('hit') else 2000,0,700 if c.get('hit') else 2000)):
        sw(0x142028+j*4,(c.get('distance',v) if c['entry']==2 and j==0 else v)*65536)
    sh(0x142224,c.get('radius',80 if c.get('hit') else 0))
    if c['entry']==2:
        sh(0x150100,0);sh(0x150104,0);sw(0x142030,0)
    sw(0x9D248,0x80151000);sh(0x9D1CC,4 if c.get('inside',1) else 0)
    for j,(x,z) in enumerate(((-10000,-10000),(-10000,10000),(10000,10000),(10000,-10000))):
        sw(0x151000+j*8,x*65536);sw(0x151004+j*8,z*65536)
    sw(0xBCFA8,0x80148100);sw(0x148100,256)
    sh(0xB0DD0,100);sh(0xB0DD2,1000);put(0xB0DCE,bytes((20,120)))
    for a,n in EXTRA_RANGES[2:]:put(a,bytes(n))
    sw(0x9CDF0,0x405)
    if c.get('sound_found'):
        sw(0x17206C,(1<<22)|0x100);sw(0x172104,0x200);sh(0x17210A,0x565)
        sw(0x172200,0x4F414B41)
    for a in (0x18FC54,0x18FC58,0x18FC5C,0x18FC70):
        put(a,overlay[a-0x18EFE8:a-0x18EFE8+4])
    if c['entry']==6:
        sw(0x942E0,0x80151C00);sw(0x942E4,0x80151800)
        sw(0x151D54,0x80151D60);sw(0x151D68,0x800D4698)
        put(0x151801,b'\x75');sw(0x15188C,0x80151E00);sw(0x151E30,0x8018FC54)
        for i,v in enumerate((0,1,*c['command_values'])):
            sw(0x152000+i*4,0x80152100+i*4);sw(0x152100+i*4,v)
    args=((c.get('mode',1),0x80150100,0x80150800),(c.get('mode',1),0x80150210),
          (0x80150100,57),(0x565,32),(0x80150200,),(0x80150200,),(0x80152000,))[c['entry']]
    return r,s,args,ctrl

def fingerprint(r):
    h=packet_fingerprint(r)
    for a,n in EXTRA_RANGES:
        for b in r[a:a+n]:h=((h^b)*1099511628211)&0xFFFFFFFFFFFFFFFF
    return h

def main():
    exe=(ROOT/'build/disc1.candidate.exe').read_bytes()
    assert hashlib.sha1(exe).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
    disc=find_disc(ROOT);assert disc,'provide PE_DISC1_BIN'
    overlay=read_form1(disc,12007,2)
    assert hashlib.sha256(overlay).hexdigest()=='5be7c97af6b8fe3f51dc34350c6e16c2bc0a659e187ed7fd7aa5d6506ed063a1'
    _,s,_,_=fixture(exe,overlay,CASES[0]);base=words(s)
    common=[(i*4,v) for i,v in enumerate(base) if v];patches=[];cases=[]
    for k,c in enumerate(CASES):
        r,s,args,ctrl=fixture(exe,overlay,c);first=len(patches)
        patches.extend((i*4,v) for i,(v,b) in enumerate(zip(words(s),base)) if v!=b)
        regs=execute(r,ENTRIES[c['entry']],args,initial_cop_control=ctrl)
        h=fingerprint(r);cases.append((c['entry'],first,len(patches),args,regs[2],h))
        print(k,c,hex(regs[2]),hex(h),flush=True)
        if '--dump' in sys.argv:(ROOT/f'local/live/m0013i-oracle-{k}.bin').write_bytes(r)
    out=['/* Generated by pe_m0013i_effect_oracle.py from original Disc 1 instructions. */']
    for name,rows in (('ranges',RANGES),('extra_ranges',EXTRA_RANGES),('common',common),('patches',patches)):
        out.append(f'static const uint32_t SEW14_{name}[][2]={{')
        out += [f'    {{0x{a:X}u,0x{b:X}u}},' for a,b in rows];out.append('};')
    out.append('static const struct { unsigned entry,first,end; uint32_t args[3],result; uint64_t hash; } SEW14_cases[]={')
    for e,a,b,args,v,h in cases:
        params=','.join(f'0x{x&0xFFFFFFFF:08X}u' for x in args)
        out.append(f'    {{{e},{a},{b},{{{params}}},0x{v:08X}u,UINT64_C(0x{h:016X})}},')
    out.append('};')
    if '--write-header' in sys.argv:(ROOT/'pc_port/tests/retail_m0013i_effect_cases.h').write_text('\n'.join(out)+'\n')
    print(f'PASS: {len(cases)} complete original M0013I effect cases')

if __name__=='__main__':main()
