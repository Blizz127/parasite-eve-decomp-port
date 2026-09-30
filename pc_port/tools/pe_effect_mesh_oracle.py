#!/usr/bin/env python3
"""Complete original effect-mesh drawing and color/UV helper call graphs."""
import hashlib
import struct
import sys
from pe_battle_hud_oracle import ROOT,execute
from pe_scripted_exit_oracle import words

ENTRIES=(0x800C71E4,0x800C6D5C,0x800C6EC0,0x800C6ED8,0x800C6EF8,0x800C6F4C,0x800C6FA0,0x800C70EC)
RANGES=((0,0x70),(0x140000,0x1400),(0x160000,0xC000),(0x9CDD8,8),
        (0xE2370,0x100),(0xF33E4,2),(0xF3414,2),(0xF3420,2),(0xF346C,2))
CASES=[]
def case(entry=0,**kw):CASES.append(dict(entry=entry,**kw))
for group in range(4):
    for bank in (0,1):
        for winding in (1,0,-1):case(group=group,bank=bank,winding=winding)
    for depth in (-100,0,1,31,32,16384,65535):case(group=group,depth=depth)
    case(group=group,bias=-256);case(group=group,bias=-257)
case(group=4);case(group=4,semi=0);case(group=4,rotation=1);case(group=4,rotation=2)
case(group=4,translation=(30000,-32768,66000));case(group=4,bias=5000)
case(group=4,mixed=1);case(group=4,empty=1)
for u,v in ((0,0),(1,2),(255,255),(0x102,0x103)):
    case(1,u=u,v=v);case(1,u=u,v=v,cached=1)
case(1,empty=1);case(2);case(3);case(4);case(4,empty_colors=1);case(5)
for level in (0,1,50,128,200,256,65535,65536):case(6,level=level)
for delta in ((0,0,0),(-255,-130,48),(32767,-32768,65535),(256,-256,1234)):
    case(7,delta=delta)

def fixture(exe,c):
    r=bytearray(0x200000);r[0x10000:0x10000+len(exe)-0x800]=exe[0x800:]
    s=bytearray(0x200000)
    def put(a,b):r[a:a+len(b)]=b;s[a:a+len(b)]=b
    def sw(a,v):put(a,struct.pack('<I',v&0xFFFFFFFF))
    def sh(a,v):put(a,struct.pack('<H',v&65535))
    for a,n in RANGES:put(a,bytes((i*17+7)&255 for i in range(n)))
    for a,v in ((0xBCFA4,0x80140000),(0x9CDD8,0),(0x9CDDC,c.get('bank',0)),
                (0xB0E58,0x80160000),(0xB0E5C,0x80162000),
                (0xB0E38,0x80164000),(0xB0E3C,0x80168000)):sw(a,v)
    for a in range(0x164000,0x16C000,4):sw(a,0xCCFFFFFF)
    sh(0xF346C,0x135);sh(0xF3414,0x7BC4);sh(0xF33E4,c.get('semi',1));sh(0xF3420,c.get('bias',0))
    matrices=((4096,0,0,0,4096,0,0,0,4096),(2896,0,2896,0,4096,0,-2896,0,2896),
              (-4096,0,0,0,-4096,0,0,0,4096))
    for k in range(2):
        a=0x140000+k*0x40
        for j,v in enumerate(matrices[c.get('rotation',0) if k else 0]):sh(a+j*2,v)
        for j,v in enumerate(c.get('translation',(0,0,0)) if k else (0,0,c.get('depth',1024))):sw(a+20+j*4,v)
    mesh=0x141000;group=c.get('group',4);face=mesh+16
    for i in range(4):
        count=0 if c.get('empty') else 2 if group in (i,4) else 0
        sh(mesh+i*2,count);vertices=3 if i<2 else 4
        for n in range(count):
            order=(0,1,2,3)
            winding=c.get('winding',1) if not c.get('mixed') else (-1 if n==0 else 1)
            if winding<0:order=(0,2,1,3)
            if winding==0:order=(0,0,0,0)
            for j in range(vertices):
                sh(face+j*2,0x200+order[j]*8);sh(face+vertices*2+j*2,0x1234+i*23+n*31+j*0x1122)
            face+=vertices*4
    for i,p in enumerate(((-100,-100,0),(100,-100,0),(-100,100,0),(100,100,0))):
        for j,v in enumerate(p):sh(mesh+0x200+i*8+j*2,v)
        sh(mesh+0x206+i*8,0x300+i*4)
        sw(mesh+0x300+i*4,0xCA112233+i*0x051725)
    sh(mesh+8,0x300);sh(mesh+10,0 if c.get('empty_colors') else 4)
    sh(mesh+12,c.get('u',0)&255 if c.get('cached') else 0xFFFF)
    sh(mesh+14,c.get('v',0)&255 if c.get('cached') else 0xFFFF)
    args=((0x80141000,0x80140040),(0x80141000,c.get('u',0),c.get('v',0)),
          (0xABCD1234,0xCAFE5678),(0xABCD1234,),(0x80141000,),(0x80141000,),
          (0x80141000,c.get('level',128)),(0x80141000,*c.get('delta',(0,0,0))))[c['entry']]
    return r,s,tuple(v&0xFFFFFFFF for v in args)

def fingerprint(r):
    h=14695981039346656037
    for a,n in RANGES:
        for b in r[a:a+n]:h=((h^b)*1099511628211)&0xFFFFFFFFFFFFFFFF
    return h

def main():
    exe=(ROOT/'build/disc1.candidate.exe').read_bytes()
    assert hashlib.sha1(exe).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
    _,s,_=fixture(exe,CASES[0]);base=words(s);common=[(i*4,v) for i,v in enumerate(base) if v]
    patches=[];cases=[]
    for k,c in enumerate(CASES):
        r,s,args=fixture(exe,c);first=len(patches)
        patches.extend((i*4,v) for i,(v,b) in enumerate(zip(words(s),base)) if v!=b)
        execute(r,ENTRIES[c['entry']],args,initial_cop_control={24:160<<16,25:112<<16,26:256,29:0x155})
        cases.append((c['entry'],first,len(patches),args,fingerprint(r)))
        print(k,c,hex(fingerprint(r)),flush=True)
        if '--dump' in sys.argv:(ROOT/f'pc_port/build/effect-mesh-oracle-{k}.bin').write_bytes(r)
    out=['/* Generated from original executable by pe_effect_mesh_oracle.py. */']
    for name,rows in (('ranges',RANGES),('common',common),('patches',patches)):
        out.append(f'static const uint32_t ATK28_mesh_{name}[][2]={{')
        out += [f'    {{0x{a:X}u,0x{b:X}u}},' for a,b in rows];out.append('};')
    out.append('static const struct { unsigned entry,first,end; uint32_t args[4]; uint64_t hash; } ATK28_mesh_cases[]={')
    for e,a,b,args,h in cases:
        params=','.join(f'0x{x:08X}u' for x in args)
        out.append(f'    {{{e},{a},{b},{{{params}}},UINT64_C(0x{h:016X})}},')
    out.append('};')
    if '--write-header' in sys.argv:(ROOT/'pc_port/tests/retail_effect_mesh_cases.h').write_text('\n'.join(out)+'\n')
    print(f'PASS: {len(cases)} complete original effect mesh cases')

if __name__=='__main__':main()
