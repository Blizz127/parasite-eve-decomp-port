#!/usr/bin/env python3
"""Execute original scripted exit, hit-flash, inventory and effect cleanup.

The generated sparse inputs are shared by the native test. Expected memory
and return values come only from executing the original complete call graph.
"""
import hashlib
import struct
import sys
from pe_battle_hud_oracle import ROOT, execute

RANGES=((0x140000,0x8000),(0x150000,11*0xA0C),(0x160000,11*0x10C),
        (0x9CD70,0x5A0),(0xA5D58,7*220),(0xB00E8,0xD4),(0xB6920,56),
        (0xB8628,72),(0xB8A90,64),(0xBCEA8,224),(0xBCD80,32),
        (0xB0CD8,0x400),(0xC0E00,0x100),(0xE10A0,28))
ENTRIES=(0x8002DC58,0x80027A08,0x800295E4,0x80051510,
         0x800703F4,0x800702DC,0x800701B4,0x800192C8)
CASES=[]
def case(entry=0,**kw): CASES.append(dict(entry=entry,**kw))
for bank in range(2):
    for tick in range(4): case(phase=2,bank=bank,tick=tick,at=9000,pe=0x2000)
case(phase=2,at=8999,aya_cmd=7)
for state in (0,0x2000,0x4000,0x6000): case(state=state)
case(cmd=7,frame=8);case(cmd=7,frame=3);case(cmd=3);case(kind=1)
for flags in (0,0x1800):
    case(phase=1,flags=flags)
    case(phase=1,flags=flags,media=64,busy=4)
case(phase=1,media=64)
case(phase=1,media=64,busy=4,timer=1)
for kind in (0,1,2,3,4):
    for color in (255,136,128,0): case(1,state=0x4000,kind=kind,color=color)
case(1,state=0x4000,kind=2,hp=0);case(1,state=0x4000,kind=4,empty_list=1)
for command in (1,393,406):
    case(1,state=0x2000,action=command)
case(1,state=0x2000,count=2);case(1,state=0x2000,weapon=6,target=0)
case(1,state=0x2000,weapon=6,target=1,free_effect=1)
case(2);case(2,flags=0x1902)
for missing in (0,1,2,3,4): case(3,missing=missing)
case(3,reuse=1)
for entry in (4,5,6):
    for pool_error in (0,1,2): case(entry,pool_error=pool_error)
case(7)

def fixture(exe,c):
    r=bytearray(0x200000);r[0x10000:0x10000+len(exe)-0x800]=exe[0x800:]
    seed=bytearray(0x200000)
    def put(a,b):r[a:a+len(b)]=b;seed[a:a+len(b)]=b
    def sw(a,v):put(a,struct.pack('<I',v&0xFFFFFFFF))
    def sh(a,v):put(a,struct.pack('<H',v&65535))
    def sb(a,v):put(a,bytes((v&255,)))
    for a,n in RANGES:put(a,bytes(n))
    sw(0x9D254,0x80140000);sw(0x9D278,0x80144000)
    sw(0x9D20C,0 if c.get('empty_list') else 0x80141000)
    sw(0x140000,0x80144000);sw(0x141000,0x800A5D5C)
    sw(0x141004,0x80142000);sw(0x142000,0x800A5E38)
    sw(0x142004,0x80140000);sw(0x14118C,0x80142000)
    sw(0xA5D58,0x80141000);sw(0xA5E34,0x80142000)
    sw(0xA5D5C,c.get('state',0));sb(0xA5D61,c.get('kind',2));sb(0xA5D62,2)
    sw(0xA5D6C,c.get('hp',40));sb(0xA5E3D,4)
    sb(0x14100E,c.get('cmd',2));sb(0x14100F,8);sh(0x14101A,c.get('frame',8))
    sw(0x14101C,0x30000);sb(0x14200E,2)
    sb(0x14000E,c.get('aya_cmd',4));sb(0x144012,4)
    for a in (0x140068,0x14006C,0x140070):sw(a,0xFFFF0000)
    sh(0x14400C,39);sh(0x14401C,45);sh(0x144010,c.get('at',0))
    sw(0x14404C,c.get('pe',0));sw(0x144068,0x80147000)
    sh(0x147006,c.get('weapon',1));sw(0x14700C,0xABCD0007)
    sw(0x9D1A0,c.get('flags',0x82));sw(0x9D2E8,0x35);sw(0x9D28C,8)
    sw(0x9D250,c.get('tick',0));sw(0x9CDDC,c.get('bank',0));sb(0x9CE74,c.get('phase',0))
    sb(0x9CE68,c.get('color',255));sb(0x9D23C,c.get('count',0))
    sw(0xBE830,0x80141000 if c.get('target',1) else 0x80142000);sh(0xBE834,c.get('action',1))
    for i in range(3):sw(0xB8A90+i*4,0x80142000+i*0x100)
    for cmd in (2,4,7,21,24):
        sw(0xB0E98+cmd*4,0x80145080+cmd*4);sb(0x145082+cmd*4,8)
    for a,p in ((0x1411B4,0x146000),(0x1421B4,0x146100)):
        sw(a,0x80145000);sh(a+0xBA,1);sw(a+0x54,0x80000000+p);sb(p+7,0x3C)
    sh(0x145008,1);sw(0x915E0,0x80012340)
    sw(0xB0CD8,c.get('busy',0));sb(0xB0DCA,c.get('media',0));sh(0xB0DC0,-1)
    sw(0x9D190,c.get('timer',0))
    sb(0xC0E0C,10);sb(0xC0E20,0);sh(0xC0E48,0x100);sh(0xC0EB6,15)
    if c.get('reuse'):
        sw(0x9D048,0x80174000);sw(0x9D04C,0x80174000);sw(0x9D054,12)
        sw(0x9D058,0x800A1F84);sw(0x9D064,4)
    missing=c.get('missing',0)
    if missing==1:sw(0x9D254,0)
    if missing==2:sw(0x140000,0)
    if missing==3:sw(0x144068,0)
    if missing==4:sb(0xC0E20,255)
    sw(0x942E0,0x80170000);sw(0x942E4,0x80150000);sw(0x942E8,0x80160000)
    for code in (2,4,0x55):
        sw(0x170000+code*4,0x80171000+code*24)
        sw(0x171000+code*24+4,0x800D4620)
        sw(0x171000+code*24+20,0x800D4850)
    sw(0xE105C,0x80172000);sw(0x172034,0x80173000)
    # 6914C has initialized pools (battle flag 0x80).
    for i in range(22):
        a=0x150000+i*0xA0C if i<11 else 0x160000+(i-11)*0x10C
        sb(a,i%7);sb(a+1,(2,4,0x72)[i%3]);sw(a+4,123+i);sw(a+8,0x80141000)
    if c.get('free_effect'):sb(0x150000,0)
    if c.get('pool_error')==1:sb(0x150000,1);sb(0x150001,0xFF)
    if c.get('pool_error')==2:sb(0x160000,1);sb(0x160001,0xFF)
    return r,seed

def fingerprint(r):
    h=14695981039346656037
    for a,n in RANGES:
        for b in r[a:a+n]:h=((h^b)*1099511628211)&0xFFFFFFFFFFFFFFFF
    return h

def words(b):return struct.unpack('<524288I',b)

def main():
    exe=(ROOT/'build/disc1.candidate.exe').read_bytes()
    assert hashlib.sha1(exe).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
    _,seed=fixture(exe,CASES[0]);base=words(seed)
    common=[(i*4,v) for i,v in enumerate(base) if v]
    patches=[];cases=[]
    for i,c in enumerate(CASES):
        r,s=fixture(exe,c);start=len(patches)
        patches.extend((j*4,v) for j,(v,b) in enumerate(zip(words(s),base)) if v!=b)
        regs=execute(r,ENTRIES[c['entry']],(0x80141000,))
        cases.append((c['entry'],start,len(patches),regs[2],fingerprint(r)))
        print(i,c,hex(regs[2]),hex(fingerprint(r)),flush=True)
        if '--dump' in sys.argv:(ROOT/f'pc_port/build/exit-oracle-{i}.bin').write_bytes(r)
    out=['/* Generated by pe_scripted_exit_oracle.py --write-header. */']
    for name,rows in (('ranges',RANGES),('common',common),('patches',patches)):
        out.append(f'static const uint32_t ATK20_exit_{name}[][2]={{')
        out += [f'    {{0x{a:X}u,0x{b:X}u}},' for a,b in rows];out.append('};')
    out.append('static const struct { unsigned entry,first,end; uint32_t result; uint64_t hash; } ATK20_exit_cases[]={')
    out += [f'    {{{e},{a},{b},0x{v:08X}u,UINT64_C(0x{h:016X})}},' for e,a,b,v,h in cases]
    out.append('};')
    if '--write-header' in sys.argv:(ROOT/'pc_port/tests/retail_scripted_exit_cases.h').write_text('\n'.join(out)+'\n')
    print(f'PASS: {len(cases)} complete original scripted exit / hit flash / inventory / pool cleanup cases')

if __name__=='__main__':main()
