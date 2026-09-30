#!/usr/bin/env python3
"""Original Aya awakening/reaction particles and point-glow packet graphs."""
import hashlib
import struct
import sys
from pe_eve_charge_oracle import ROOT, execute, fixture as charge_fixture, RANGES as CHARGE_RANGES
from pe_scripted_exit_oracle import words

RANGES = CHARGE_RANGES + ((0xE17E0,64),(0xE21E0,12),(0xF34E4,2))
ENTRIES = (0x800D751C,0x800D71B8,0x800D70C0,0x800D1DEC,0x800CE688,0x800CE78C)
CASES = []
def case(entry=0, **kw): CASES.append(dict(entry=entry, **kw))
case(mode=0); case(mode=0,seed=71); case(mode=-1); case(mode=3)
for time in (-3,-1,0,1,2,31,32,33,69,70,71): case(mode=1,time=time)
case(mode=1,time=1,full=1); case(mode=1,time=3,sparks=3)
for bank in (0,1):
    for sparks in (0,1,5): case(mode=2,bank=bank,sparks=sparks)
for time in (-1,0,17,35,36,37,7000000):
    for seed in (1,10): case(1,mode=1,time=time,seed=seed)
case(1,mode=1,full=1,seed=10)
for state in (0,1,2,3):
    for age in (0,1,11,12,13,24): case(1,mode=2,state=state,age=age)
case(1,mode=2,state=2,age=-1); case(1,mode=2,type=4,alternate=1,bank=1)
case(1,mode=0); case(1,mode=3)
for time in (-1,0,17,18,19,300): case(2,mode=1,time=time)
for time in (0,1,9,17,18,30): case(2,mode=2,time=time)
case(2,mode=0); case(2,mode=3)
for bank in (0,1):
    for blend in (-1,0,1,3,255,256): case(3,bank=bank,blend=blend)
for depth in (-100,0,31,32,36,16384,65535): case(3,depth=depth)
for brightness in (-32768,-1,0,1,127,128,255,32767): case(3,brightness=brightness)
for time in (0,17,35,36): case(4,time=time,particles=3)
for state in (0,1,2): case(5,state=state,age=12,particles=3)

def fixture(exe,c):
    # These callbacks are in the executable; no room-overlay instructions run.
    r,s,_,ctrl=charge_fixture(exe,bytes(0x18000),dict(c,entry=0))
    def put(a,b): r[a:a+len(b)]=b;s[a:a+len(b)]=b
    def sw(a,v): put(a,struct.pack('<I',v&0xFFFFFFFF))
    def sh(a,v): put(a,struct.pack('<H',v&65535))
    for a,n in ((0xE17E0,64),(0xC22D4,4)):
        put(a,exe[a-0x10000+0x800:a-0x10000+0x800+n])
    sw(0x9D254,0x80140000)
    for i,v in enumerate((300,-400,700)): sw(0x140028+i*4,v<<16);sh(0xE21E0+i*2,v)
    sh(0xE11F6,1);sh(0xE2852,0x1E);sh(0xF34E4,1)
    sw(0x150100,713);sw(0x150104,0x80153000);sw(0xE21E8,0x80153000)
    sw(0x150034,0x80152000)
    for pool,count,stride,fn in ((0x152000,24,20,0x800D71B8),(0x153000,18,12,0x800D70C0)):
        sw(pool,stride);sw(pool+4,count);sw(pool+8,fn)
        for i in range(count):
            p=pool+12+i*stride
            sh(p,int(bool(c.get('full')) or i<c.get('particles' if pool==0x152000 else 'sparks',0)))
            sh(p+2,c.get('time',17)+i)
            for j,v in enumerate((c.get('state',0),c.get('angle',713),c.get('age',1),100,-200,300,200,0) if pool==0x152000 else (100,-200,300,0)):
                sh(p+4+j*2,v)
    for i,v in enumerate((c.get('state',0),c.get('angle',713),c.get('age',1),100,-200,300,200,0)):
        sh(0x151800+i*2,v)
    sh(0xF336C,c.get('type',1));sw(0xE27EC,c.get('time',17))
    args=((c.get('mode',2),0x80150100),(c.get('mode',1),0x80151800),
          (c.get('mode',2),0x80151806),
          (0x80151806,0x80151110,c.get('brightness',128),c.get('blend',1)),
          (0x80152000,),(0x80152000,))[c['entry']]
    for i in range(3):ctrl[5+i]=struct.unpack_from('<I',r,0x148014+i*4)[0]
    ctrl[29]=0x155
    return r,s,tuple(v&0xFFFFFFFF for v in args),ctrl

def fingerprint(r):
    r=bytearray(r);bank=struct.unpack_from('<I',r,0x9CDDC)[0]
    base=0x160000+bank*0x2000;size=struct.unpack_from('<I',r,0x9CDD8)[0]
    linked=set()
    for i in range(4096):
        p=struct.unpack_from('<I',r,0x164000+bank*0x4000+i*4)[0]&0xFFFFFF
        while base<=p<base+size and p not in linked:
            linked.add(p);p=struct.unpack_from('<I',r,p)[0]&0xFFFFFF
    p=base
    while p<base+size:
        length=r[p+3]
        assert length in (1,2,3,9),(hex(p),length,size)
        if length==9:
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
    _,s,_,_=fixture(exe,CASES[0]);base=words(s);common=[(i*4,v) for i,v in enumerate(base) if v]
    patches=[];cases=[]
    for k,c in enumerate(CASES):
        r,s,args,controls=fixture(exe,c);first=len(patches)
        patches.extend((i*4,v) for i,(v,b) in enumerate(zip(words(s),base)) if v!=b)
        regs=execute(r,ENTRIES[c['entry']],args,initial_cop_control=controls,bios_seed=c.get('seed',1))
        result=0 if c['entry']==3 else regs[2]
        cases.append((c['entry'],c.get('seed',1),first,len(patches),args,controls,result,fingerprint(r)))
        print(k,c,hex(result),hex(fingerprint(r)),flush=True)
        if '--dump' in sys.argv:(ROOT/f'pc_port/build/aya-reaction-oracle-{k}.bin').write_bytes(r)
    out=['/* Generated by pe_aya_reaction_oracle.py --write-header. */']
    for name,rows in (('ranges',RANGES),('common',common),('patches',patches)):
        out.append(f'static const uint32_t ATK35_reaction_{name}[][2]={{')
        out.extend(f'    {{0x{a:X}u,0x{b:X}u}},' for a,b in rows);out.append('};')
    out.append('static const struct { unsigned entry,seed,first,end; uint32_t args[4],result; uint64_t hash; } ATK35_reaction_cases[]={')
    for e,seed,a,b,args,ctrl,v,h in cases:
        params=','.join(f'0x{x:08X}u' for x in args)
        out.append(f'    {{{e},{seed},{a},{b},{{{params}}},0x{v:08X}u,UINT64_C(0x{h:016X})}},')
    out.append('};')
    if '--write-header' in sys.argv:(ROOT/'pc_port/tests/retail_aya_reaction_cases.h').write_text('\n'.join(out)+'\n')
    print(f'PASS: {len(cases)} complete original Aya reaction / glow cases')

if __name__=='__main__':main()
