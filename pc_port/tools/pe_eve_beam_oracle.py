#!/usr/bin/env python3
"""Complete original M0005 beam, streaks, mesh layers and hit handshakes."""
import hashlib
import struct
import sys
from pe_eve_charge_oracle import ROOT,execute,find_disc,read_form1,PE_IMG_LBA,REL,CHUNK2_SHA
from pe_eve_charge_oracle import fixture as charge_fixture,RANGES as CHARGE_RANGES
from pe_effect_mesh_oracle import fixture as mesh_fixture
from pe_scripted_exit_oracle import words

ENTRIES=(0x8018FDC4,0x8018FB84,0x800D2370,0x800D4704,0x800CE688,0x800CE78C)
RANGES=CHARGE_RANGES+((0,0x90),(0x190B94,0x20),(0xE2370,0x100),(0xE2844,4),
    (0xF33E4,2),(0xF3414,2),(0xF3420,2),(0xF346C,2))
CASES=[]
def case(entry=0,**kw):CASES.append(dict(entry=entry,**kw))
case(mode=0);case(mode=0,notify=0);case(mode=0,action=2);case(mode=3)
for state in (0,1,2):
    for timer in (-1,0,3,4,6,7,8,32767):case(mode=1,state=state,timer=timer)
for time in (-1,0,8,9,10,13):case(mode=1,time=time)
case(mode=1,full=1);case(mode=1,notify=0);case(mode=1,owner_flags=0x91000000)
case(mode=1,aya=(3000,0,3000));case(mode=1,aya=(-20,0,300));case(mode=1,aya=(20,12345,300))
for bank in (0,1):
    for time in (0,1,3,4,5,8,10,12,13,20):case(mode=2,bank=bank,time=time)
for amplitude in (0,512,4096,-500):case(mode=2,amplitude=amplitude)
case(mode=2,depth=-100);case(mode=2,depth=65535);case(mode=2,length=32767)
case(mode=2,angle=1024);case(mode=2,angle=-713)
for time in (-1,0,1,5,6,7,100):case(1,mode=1,time=time)
case(1,mode=0);case(1,mode=3)
for time in (0,1,2,3,4,5,6):case(1,mode=2,time=time)
case(1,mode=2,texture_type=4,alternate=1)
for bank in (0,1):
    for abr in (0,1,3,255):case(2,bank=bank,abr=abr)
for depth in (-100,0,31,32,16384,65535):case(2,depth=depth)
for width in (-111,-1,0,1,110,32767,65536):case(2,width=width)
for brightness in (-32768,-127,0,128,32767):case(2,brightness=brightness)
case(2,color=0);case(2,end_color=0);case(2,dqa=0,dqb=0);case(2,angle=1024)
case(3,time=3,particles=3);case(3,time=10,particles=3)
case(4,time=5,particles=3);case(4,time=6,particles=3);case(5,time=3,particles=3)

def fixture(exe,overlay,c):
    r,s,_,ctrl=charge_fixture(exe,overlay,dict(c,entry=0))
    def put(a,b):r[a:a+len(b)]=b;s[a:a+len(b)]=b
    def sw(a,v):put(a,struct.pack('<I',v&0xFFFFFFFF))
    def sh(a,v):put(a,struct.pack('<H',v&65535))
    for a,n in RANGES[len(CHARGE_RANGES):]:put(a,bytes(n))
    for a,n in ((0x960BC,0x180),(0x9A6EC,0x804),(0xC2258,16),(0xC22A0,4)):
        put(a,exe[a-0x10000+0x800:a-0x10000+0x800+n])
    put(0x18EFFC,overlay[0x18EFFC-0x18EFE8:0x18F018-0x18EFE8])
    sh(0xE11EA,1);sh(0xE11FA,2);sh(0xE2852,0x11);sh(0xE2854,0x12);sh(0x942EC,-31)
    sw(0x144000,c.get('owner_flags',0x01000000));sw(0x144018,0x80144100);put(0x144100,bytes((c.get('action',1),)))
    put(0x150019,bytes((c.get('notify',1),)));sh(0x15002C,2);sh(0x15002E,c.get('time',3))
    sw(0x150034,0x80150200);put(0x150100,bytes(36))
    for j,v in enumerate((0,0,c.get('angle',0),1)):sh(0x150110+j*2,v)
    sw(0x150118,c.get('length',1536));sw(0x15011C,c.get('amplitude',4096))
    sh(0x150120,c.get('state',0));sh(0x150122,c.get('timer',0))
    sw(0x150200,20);sw(0x150204,8);sw(0x150208,0x8018FB84)
    for i in range(8):
        p=0x15020C+i*20;put(p,bytes(20));sh(p,int(bool(c.get('full')) or i<c.get('particles',0)));sh(p+2,c.get('time',3))
        for j,v in enumerate((100+i*17,-50+i*9,0,0,-3,4,0,0)):sh(p+4+j*2,v)
    sw(0x9D254,0x80143000);sw(0x143000,0x80145000)
    for j,v in enumerate(c.get('aya',(13,0,100))):sh(0x14302A+j*4,v)
    # Actual archive directory layout; drawing payload has every mesh facet format.
    sw(0xB0E64,0x80172000);sw(0x172004,0x40);sw(0x172048,(2<<22)|0x100)
    mr,_,_=mesh_fixture(exe,dict(entry=0,group=4))
    for i,key in enumerate((0xC5462704,0xC5862704)):
        offset=0x1000+i*0x400;sw(0x172104+i*12,offset);sw(0x172108+i*12,key)
        put(0x172000+offset,mr[0x141000:0x141400]);sw(0x190B94+i*4,0x80172000+offset)
    for j,v in enumerate((0,300,0)):sh(0x190BA0+j*2,v)
    for j,v in enumerate((300,-400,700)):sh(0x190BA8+j*2,v)
    args=((c.get('mode',2),0x80150100),(c.get('mode',2),0x80150210),
        (0x80190BA8,0x80151100,c.get('length',1100),c.get('width',110),128,32,127,15,0x7FC1,
        0x80151110 if c.get('color',1) else 0,0x80151120 if c.get('end_color',1) else 0,
        c.get('brightness',128),c.get('abr',1)),(0x80150000,),(0x80150200,),(0x80150200,))[c['entry']]
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
    disc=find_disc(ROOT);assert disc,'provide PE_DISC1_BIN'
    overlay=read_form1(disc,PE_IMG_LBA+REL+33+169,96);assert hashlib.sha256(overlay).hexdigest()==CHUNK2_SHA
    _,s,_,_=fixture(exe,overlay,CASES[0]);base=words(s);common=[(i*4,v) for i,v in enumerate(base) if v]
    patches=[];cases=[]
    for k,c in enumerate(CASES):
        r,s,args,controls=fixture(exe,overlay,c);first=len(patches)
        patches.extend((i*4,v) for i,(v,b) in enumerate(zip(words(s),base)) if v!=b)
        regs=execute(r,ENTRIES[c['entry']],args,initial_cop_control=controls)
        if c['entry']==2:regs[2]=0
        cases.append((c['entry'],first,len(patches),args,controls,regs[2],fingerprint(r)))
        print(k,c,hex(regs[2]),hex(fingerprint(r)),flush=True)
        if '--dump' in sys.argv:(ROOT/f'pc_port/build/eve-beam-oracle-{k}.bin').write_bytes(r)
    out=['/* Generated from original Disc 1 code by pe_eve_beam_oracle.py. */']
    for name,rows in (('ranges',RANGES),('common',common),('patches',patches)):
        out.append(f'static const uint32_t ATK29_beam_{name}[][2]={{')
        out += [f'    {{0x{a:X}u,0x{b:X}u}},' for a,b in rows];out.append('};')
    out.append('static const struct { unsigned entry,first,end; uint32_t args[13],dqa,dqb,result; uint64_t hash; } ATK29_beam_cases[]={')
    for e,a,b,args,ctrl,v,h in cases:
        params=','.join(f'0x{x:08X}u' for x in args)
        out.append(f'    {{{e},{a},{b},{{{params}}},0x{ctrl[27]&0xFFFFFFFF:08X}u,0x{ctrl[28]&0xFFFFFFFF:08X}u,0x{v:08X}u,UINT64_C(0x{h:016X})}},')
    out.append('};')
    if '--write-header' in sys.argv:(ROOT/'pc_port/tests/retail_eve_beam_cases.h').write_text('\n'.join(out)+'\n')
    print(f'PASS: {len(cases)} complete original Eve beam cases')

if __name__=='__main__':main()
