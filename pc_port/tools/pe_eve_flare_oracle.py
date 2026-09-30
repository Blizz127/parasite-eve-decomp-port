#!/usr/bin/env python3
"""Original Eve eye/hand flares and shaded annulus packet generation."""
import hashlib
import struct
import sys
from pe_eve_charge_oracle import ROOT,execute,find_disc,read_form1,PE_IMG_LBA,REL,CHUNK2_SHA,RANGES
from pe_eve_charge_oracle import fixture as charge_fixture
from pe_scripted_exit_oracle import words

ENTRIES=(0x8018F614,0x800D0728,0x800D4704)
CASES=[]
def case(entry=0,**kw):CASES.append(dict(entry=entry,**kw))
case(mode=0);case(mode=0,sound=1);case(mode=3)
for time in (-1,0,1,34,35,36,999):case(mode=1,time=time)
for bank in (0,1):
    for time in (0,1,7,17,18,19,24,31,32,34,35,36):case(time=time,bank=bank)
case(time=24,depth=-100);case(time=24,depth=65535);case(time=24,align=1)
for segments in (-1,0,3,4,5,7,24,32):
    for abr in (1,255):case(1,segments=segments,abr=abr)
for align in (-1,0,1):case(1,align=align,angles=1)
case(1,angles=0);case(1,bank=1,abr=3);case(1,inner=-900)
case(1,inner=1000,outer=20);case(1,inner_color=0);case(1,outer_color=0)
case(1,brightness=-127);case(1,sx=-1234,sy=32767);case(1,angle=0,align=1,depth=300)
for depth in (-100,0,31,32,16384,65535):case(1,depth=depth)
case(2,time=0);case(2,time=24)

def fixture(exe,overlay,c):
    r,s,_,ctrl=charge_fixture(exe,overlay,dict(c,entry=0))
    def put(a,b):r[a:a+len(b)]=b;s[a:a+len(b)]=b
    def sw(a,v):put(a,struct.pack('<I',v&0xFFFFFFFF))
    def sh(a,v):put(a,struct.pack('<H',v&65535))
    for a,n in ((0xC2260,8),(0xC2290,16)):
        put(a,exe[a-0x10000+0x800:a-0x10000+0x800+n])
    put(0x18EFFC,overlay[0x18EFFC-0x18EFE8:0x18F00C-0x18EFE8])
    sh(0xE11EA,1);sh(0xE11FA,2);sh(0xE2852,0x11);sh(0xE2854,0x12)
    if c['entry']==2:sh(0x15002C,1);sw(0x150034,0);sh(0x15002E,c.get('time',0))
    args=((c.get('mode',2),0x80150100),
          (0x8015011C,c.get('inner',500),c.get('outer',700),c.get('segments',24),
           0x80151100 if c.get('angles',1) else 0,c.get('sx',4096),c.get('sy',4096),
           0x80151110 if c.get('inner_color',1) else 0,0x80151120 if c.get('outer_color',1) else 0,
           c.get('brightness',64),c.get('abr',1)),(0x80150000,))[c['entry']]
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
        if length not in (1,8,9):p+=4;continue
        if length==9:
            r[p+30:p+32]=bytes(2);r[p+38:p+40]=bytes(2)
        elif length==8:
            for k in (15,23,31):r[p+k]=0
        if p not in linked:
            r[p:p+3]=bytes(3)
            if length in (8,9):r[p+32:p+36]=bytes(4)
        p+=(length+1)*4
    h=14695981039346656037
    for a,n in RANGES:
        for b in r[a:a+n]:h=((h^b)*1099511628211)&0xFFFFFFFFFFFFFFFF
    return h

def main():
    exe=(ROOT/'build/disc1.candidate.exe').read_bytes()
    assert hashlib.sha1(exe).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
    disc=find_disc(ROOT);assert disc,'provide PE_DISC1_BIN'
    overlay=read_form1(disc,PE_IMG_LBA+REL+33+169,96)
    assert hashlib.sha256(overlay).hexdigest()==CHUNK2_SHA
    _,s,_,_=fixture(exe,overlay,CASES[0]);base=words(s);common=[(i*4,v) for i,v in enumerate(base) if v]
    patches=[];cases=[]
    for k,c in enumerate(CASES):
        r,s,args,controls=fixture(exe,overlay,c);first=len(patches)
        patches.extend((i*4,v) for i,(v,b) in enumerate(zip(words(s),base)) if v!=b)
        regs=execute(r,ENTRIES[c['entry']],args,initial_cop_control=controls)
        if c['entry']==1:regs[2]=0
        cases.append((c['entry'],first,len(patches),args,regs[2],fingerprint(r)))
        print(k,c,hex(regs[2]),hex(fingerprint(r)),flush=True)
        if '--dump' in sys.argv:(ROOT/f'pc_port/build/eve-flare-oracle-{k}.bin').write_bytes(r)
    out=['/* Generated from original Disc 1 code by pe_eve_flare_oracle.py. */']
    for name,rows in (('ranges',RANGES),('common',common),('patches',patches)):
        out.append(f'static const uint32_t ATK26_flare_{name}[][2]={{')
        out += [f'    {{0x{a:X}u,0x{b:X}u}},' for a,b in rows];out.append('};')
    out.append('static const struct { unsigned entry,first,end; uint32_t args[11],result; uint64_t hash; } ATK26_flare_cases[]={')
    for e,a,b,args,v,h in cases:
        params=','.join(f'0x{x:08X}u' for x in args)
        out.append(f'    {{{e},{a},{b},{{{params}}},0x{v:08X}u,UINT64_C(0x{h:016X})}},')
    out.append('};')
    if '--write-header' in sys.argv:(ROOT/'pc_port/tests/retail_eve_flare_cases.h').write_text('\n'.join(out)+'\n')
    print(f'PASS: {len(cases)} complete original Eve flare / ring cases')

if __name__=='__main__':main()
