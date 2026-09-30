#!/usr/bin/env python3
"""Complete original M0005 charging particles and shared sprite call graphs.

The executable and actual Disc 1 overlay are instruction authorities. GPU
packet padding and unlinked tag/XY3 stack bytes are outside the comparison;
every defined packet field, ordering-table link and persistent state is hashed.
"""
import hashlib
import struct
import sys
from pe_battle_hud_oracle import ROOT,execute
from pe_btl14_m0005i_publish_oracle import find_disc,read_form1,PE_IMG_LBA,REL,CHUNK2_SHA
from pe_scripted_exit_oracle import words

ENTRIES=(0x8018F330,0x8018F018,0x800CEE20,0x800CF3AC,0x800783E4,
         0x80078554,0x80078CC4,0x800786E4,0x800CE688,0x800CE78C,0x800D4704)
RANGES=((0x140000,0x34000),(0x190AD4,0xC0),(0x9CDD8,8),(0x9D2F4,4),
        (0xB8848,0x300),(0xF3368,0x12),(0xE27EC,4),(0xF33E0,4),(0xE2368,4),(0xF32D0,4))
CASES=[]
def case(entry=0,**kw):CASES.append(dict(entry=entry,**kw))
case(mode=0);case(mode=2);case(mode=3)
for time in (-1,0,7,24,25,100):
    for local in (0,1):case(time=time,local=local)
for timer in (-1,0,30,31,32,32767):case(state=1,timer=timer,time=25)
case(full=1);case(time=7,sound=1)
for state in (0,1,2):
    for angle in (-32768,-2048,-1,0,1024,2048,3072,32767):
        case(1,state=state,angle=angle,time=17,local=1)
for timer in (-1,0,30,31,32,255,32767):case(1,state=1,timer=timer)
for value in (120,128,129,-1):case(1,brightness=value)
case(1,mode=0);case(1,mode=2);case(1,mode=2,bank=1,angle=-713,texture_type=4,alternate=1)
for time in (0,1,31,32,33,63,64,65,128):case(3,time=time)
case(3,time=47,initialized=1)
for entry in (4,5):
    for wa,wb in ((4096,0),(0,4096),(2048,2048),(-4096,8192),(32767,32767),(-32768,-32768)):
        case(entry,wa=wa,wb=wb)
case(6);case(6,sx=-12000,sy=50000);case(7)
for bank in (0,1):
    for abr in (0,3,255):
        for align in (0,1):case(2,bank=bank,abr=abr,align=align,angle=-713)
for depth in (-100,0,31,32,32768,65535):case(2,depth=depth)
for texture in (-17,-1,0,7,8,15,16,217):case(2,texture=texture,split=1)
case(2,texture_type=4,alternate=1);case(2,color=0);case(2,brightness=-20)
case(2,angles=0);case(2,sx=-4096,sy=70000);case(2,count=0);case(2,count=3)
case(2,dqa=0,dqb=0);case(2,dqa=-32768,dqb=0x80000000)
case(8,particles=3);case(8,particles=3,state=1,timer=31)
case(9,particles=3);case(10,particles=3)

def fixture(exe,overlay,c):
    r=bytearray(0x200000);r[0x10000:0x10000+len(exe)-0x800]=exe[0x800:]
    r[0x18EFE8:0x18EFE8+len(overlay)]=overlay
    s=bytearray(0x200000)
    def put(a,b):r[a:a+len(b)]=b;s[a:a+len(b)]=b
    def sw(a,v):put(a,struct.pack('<I',v&0xFFFFFFFF))
    def sh(a,v):put(a,struct.pack('<H',v&65535))
    def sb(a,v):put(a,bytes((v&255,)))
    for a,n in RANGES:put(a,bytes(n))
    for a,n in ((0x9589C,0x804),(0x966EC,0x4000),(0xE1204,12),(0xC2268,8)):
        put(a,exe[a-0x10000+0x800:a-0x10000+0x800+n])
    for a,n in ((0x18EFF4,8),(0x190AD4,0xC0)):
        put(a,overlay[a-0x18EFE8:a-0x18EFE8+n])
    if c.get('initialized'):
        sw(0x190AD4,64);sw(0x190AD8,2)
        sh(0x190AE0,32);sh(0x190AE2,0);sh(0x190AE8,32);sh(0x190AEA,32)
    sw(0x140000,0x80144000);sw(0x140238,0x80146000);sw(0x144008,32)
    for k in range(21):
        m=0x146000+k*32
        for j,v in enumerate((0,0,4096,0,4096,0,-4096,0,0)):sh(m+j*2,v)
        for j,v in enumerate((300,-400,700)):sw(m+20+j*4,v+k*7)
    sw(0xBCFA4,0x80148000)
    for j,v in enumerate((0,0,4096,0,4096,0,-4096,0,0)):sh(0x148000+j*2,v)
    sw(0x14801C,c.get('depth',2048));sh(0xBCF94,160);sh(0xBCF96,112)
    # Depth tests use an identity camera so position does not change depth.
    if 'depth' in c:
        for j,v in enumerate((4096,0,0,0,4096,0,0,0,4096)):sh(0x148000+j*2,v)
    sw(0xF32D0,0x80150000);sw(0xE2368,0x8015000C);sw(0xF33E0,0x8015002C)
    sw(0x150008,0x80140000);sw(0x15008C,0x80190B44);sh(0x15001E,c.get('local',0))
    for i in range(8):sh(0x15002C+i*12,65535)
    sh(0x15002C,0);sw(0x150030,0x80150100);sw(0x150034,0x8015010C)
    sh(0x150108,c.get('state',0));sh(0x15010A,c.get('timer',0));sw(0xE27EC,c.get('time',29))
    sw(0x15010C,20);sw(0x150110,24);sw(0x150114,0x8018F018)
    for i in range(24):
        p=0x150118+i*20
        sh(p,int(bool(c.get('full')) or i<c.get('particles',0)));sh(p+2,17+i)
        for j,v in enumerate((100+i*17,-50+i*9,0,c.get('brightness',128),c.get('angle',713),300,c.get('state',0),c.get('timer',0))):sh(p+4+j*2,v)
    for j,v in enumerate((300,-400,700)):sh(0x190B84+j*2,v)
    for j,v in enumerate((500,-300,600)):sh(0x190B8C+j*2,v)
    sh(0xF3368,16);sh(0xF336A,1);sh(0xF336C,c.get('texture_type',2));sh(0xF336E,c.get('split',0))
    sh(0xF3370,0x1E);sh(0xF3372,0);sh(0xF3374,8);sh(0xF3376,16);sh(0xF3378,16)
    sw(0xF3428,c.get('alternate',0));sh(0xE11E8,0);sh(0xE2850,0x1E)
    sw(0xB0E64,0)
    if c.get('sound'):
        # Empty real-format package exercises both original lookup calls.
        sw(0xB0E64,0x80172000);sw(0x172004,0x40)
    sw(0x9CDDC,c.get('bank',0));sw(0x9CDD8,0)
    for i in range(2):
        sw(0xB0E58+i*4,0x80160000+i*0x2000);sw(0xB0E38+i*4,0x80164000+i*0x4000)
        for j in range(4096):sw(0x164000+i*0x4000+j*4,0xABFFFFFF)
    sh(0xE1210,c.get('count',1));sw(0xE13BC,0x80170000)
    for i in range(3):
        for j,(x,y,z) in enumerate(((-60,-60,0),(60,-60,0),(-60,60,0),(60,60,0))):
            for k,v in enumerate((x+i*90,y,z,0)):sh(0x170000+i*32+j*8+k*2,v)
    for j,v in enumerate((0,0,c.get('angle',713),c.get('align',0))):sh(0x151100+j*2,v)
    put(0x151110,bytes((71,183,255,117)));put(0x151120,bytes((233,29,37,199)))
    for j,v in enumerate((-30000,21001,30000,117)):sh(0x151200+j*2,v)
    for j,v in enumerate((31000,-27000,-28001,119)):sh(0x151210+j*2,v)
    for j,v in enumerate((100,-2700,4096,520,-7700,1280,-2048,32000,-2345,117)):sh(0x151300+j*2,v)
    for j,v in enumerate((c.get('sx',2048),c.get('sy',8192),4096)):sw(0x151320+j*4,v)
    args=((c.get('mode',1),0x80150100),(c.get('mode',1),0x8015011C),
          (0x8015011C,0x80151100 if c.get('angles',1) else 0,c.get('sx',4096),c.get('sy',4096),c.get('texture',217),0x7F02,c.get('abr',3),c.get('brightness',128),0x80151110 if c.get('color',1) else 0),
          (0x80190AD4,0x80151000,c.get('time',0)),
          (0x80151200,0x80151210,c.get('wa',2048),c.get('wb',2048),0x80151000),
          (0x80151110,0x80151120,c.get('wa',2048),c.get('wb',2048),0x80151000),
          (0x80151300,0x80151320),(0x80151300,),
          (0x8015010C,),(0x8015010C,),(0x80150000,))[c['entry']]
    controls={24:160<<16,25:112<<16,26:256,27:c.get('dqa',-0x1062),28:c.get('dqb',0x1400000)}
    for j,v in enumerate(struct.unpack_from('<5I',r,0x148000)):controls[j]=v
    return r,s,tuple(v&0xFFFFFFFF for v in args),controls

def fingerprint(r):
    r=bytearray(r);bank=struct.unpack_from('<I',r,0x9CDDC)[0]
    base=0x160000+bank*0x2000;size=struct.unpack_from('<I',r,0x9CDD8)[0]
    linked=set()
    for i in range(4096):
        p=struct.unpack_from('<I',r,0x164000+bank*0x4000+i*4)[0]&0xFFFFFF
        while base<=p<base+size and p not in linked:
            linked.add(p);p=struct.unpack_from('<I',r,p)[0]&0xFFFFFF
    for p in range(base,base+size,40):
        r[p+30:p+32]=bytes(2);r[p+38:p+40]=bytes(2)
        if p not in linked:r[p:p+3]=bytes(3);r[p+32:p+36]=bytes(4)
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
        if c['entry'] in (2,3,4,5,7):regs[2]=0
        cases.append((c['entry'],first,len(patches),args,controls,regs[2],fingerprint(r)))
        print(k,c,hex(regs[2]),hex(fingerprint(r)),flush=True)
        if '--dump' in sys.argv:(ROOT/f'pc_port/build/eve-charge-oracle-{k}.bin').write_bytes(r)
    out=['/* Generated from original Disc 1 code by pe_eve_charge_oracle.py. */']
    for name,rows in (('ranges',RANGES),('common',common),('patches',patches)):
        out.append(f'static const uint32_t ATK25_charge_{name}[][2]={{')
        out += [f'    {{0x{a:X}u,0x{b:X}u}},' for a,b in rows];out.append('};')
    out.append('static const struct { unsigned entry,first,end; uint32_t args[9],dqa,dqb,result; uint64_t hash; } ATK25_charge_cases[]={')
    for e,a,b,args,ctrl,v,h in cases:
        params=','.join(f'0x{x:08X}u' for x in args)
        out.append(f'    {{{e},{a},{b},{{{params}}},0x{ctrl[27]&0xFFFFFFFF:08X}u,0x{ctrl[28]&0xFFFFFFFF:08X}u,0x{v:08X}u,UINT64_C(0x{h:016X})}},')
    out.append('};')
    if '--write-header' in sys.argv:(ROOT/'pc_port/tests/retail_eve_charge_cases.h').write_text('\n'.join(out)+'\n')
    print(f'PASS: {len(cases)} complete original Eve charging / sprite / math cases')

if __name__=='__main__':main()
