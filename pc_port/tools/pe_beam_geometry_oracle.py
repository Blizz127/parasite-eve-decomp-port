#!/usr/bin/env python3
"""Complete original beam orientation, lookup square root and collision graphs."""
import hashlib
import struct
import sys
from pe_battle_hud_oracle import ROOT,execute
from pe_scripted_exit_oracle import words

ENTRIES=(0x80078004,0x800CFAA8,0x800CE9D4,0x800CFB7C,0x800C62DC,0x800C6B20,0x800CEB8C)
# The instruction runner's first 0x90 bytes represent physical GTE scratchpad.
RANGES=((0,0x90),(0x140000,0x14000),(0xE2844,4))
CASES=[]
def case(entry=0,**kw):CASES.append(dict(entry=entry,**kw))
for n in (0,1,2,3,4,63,64,65,255,256,257,4096,65535,123456,0x7FFFFFFF,0x80000000,0xFFFFFFFE,0xFFFFFFFF):case(value=n)
for target in ((0,0,0),(100,0,0),(-100,0,0),(0,100,0),(0,-100,0),(0,0,100),(0,0,-100),(300,200,-400),(-30000,21000,31000)):
    case(1,target=target)
case(1,origin=(32767,-32768,32767),target=(-32768,32767,-32768))
for rotation in range(4):case(2,rotation=rotation)
for angle in (-32768,-1024,-1,0,512,1024,2048,4095,32767):
    for distance in (-700,0,700):case(3,angle=angle,distance=distance)
for entry in (4,5,6):
    for point in ((0,0,0),(13,0,105),(100,0,100),(300,0,300),(600,0,600),(-1,0,-1),(150,30000,0)):
        case(entry,origin=point)
for radius in (0,1,100,-100,1000,32767):case(6,origin=(20,0,100),radius=radius)
case(4,degenerate=1);case(4,reverse=1);case(5,reverse=1)
case(6,origin=(20,0,100),target=(0,0,0));case(6,origin=(13,0,105),target=(-300,0,400))

def fixture(exe,c):
    r=bytearray(0x200000);r[0x10000:0x10000+len(exe)-0x800]=exe[0x800:]
    s=bytearray(0x200000)
    def put(a,b):r[a:a+len(b)]=b;s[a:a+len(b)]=b
    def sw(a,v):put(a,struct.pack('<I',v&0xFFFFFFFF))
    def sh(a,v):put(a,struct.pack('<H',v&65535))
    for a,n in RANGES:put(a,bytes((i*17+7)&255 for i in range(n)))
    for a,n in ((0x9589C,0x804),(0x960BC,0x180),(0x966EC,0x4000),(0x9A6EC,0x804),(0xC2258,16)):
        put(a,exe[a-0x10000+0x800:a-0x10000+0x800+n])
    sw(0x140238,0x80141000);sw(0xBCFA4,0x80142000);sw(0x9D254,0x80145000)
    matrices=((4096,0,0,0,4096,0,0,0,4096),(0,0,4096,0,4096,0,-4096,0,0),
              (-4096,0,0,0,-4096,0,0,0,4096),(2896,0,2896,0,4096,0,-2896,0,2896))
    for k in range(2):
        m=0x141000+k*0x1000
        for j,v in enumerate(matrices[c.get('rotation',1) if k==0 else 1]):sh(m+j*2,v)
        for j,v in enumerate((300,-400,700)):sw(m+20+j*4,v)
    point=c.get('origin',(0,0,0));target=c.get('target',(300,200,400))
    for j,v in enumerate(point):sh(0x14502A+j*4,v)
    for j,v in enumerate(c.get('start',(0,0,0)) if c['entry']==6 else point):sh(0x143000+j*2,v)
    for j,v in enumerate(target):sh(0x143010+j*2,v)
    for j,v in enumerate((c.get('angle',713),-311,1234,0xFACE)):sh(0x143020+j*2,v)
    vertices=((0,0,0),(0,0,300),(300,0,0),(300,0,300))
    if c.get('reverse'):vertices=tuple(reversed(vertices))
    if c.get('degenerate'):vertices=((0,0,0),)*4
    for j,v in enumerate(vertices):
        for k,x in enumerate(v):sh(0x144000+j*8+k*2,x)
    args=((c.get('value',123456),),(0x80143000,0x80143010,0x80143100),
          (0x80140000,0,0x80143100),(0x80143020,c.get('distance',700),0x80143100),
          (0x80143000,0x80144000),(0x80144000,),
          (0x80143000,0x80143010,c.get('radius',100)))[c['entry']]
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
        regs=execute(r,ENTRIES[c['entry']],args)
        if c['entry'] in (1,2,3):regs[2]=0
        cases.append((c['entry'],first,len(patches),args,regs[2],fingerprint(r)))
        print(k,c,hex(regs[2]),hex(fingerprint(r)),flush=True)
        if '--dump' in sys.argv:(ROOT/f'pc_port/build/beam-geometry-oracle-{k}.bin').write_bytes(r)
    out=['/* Generated from original executable by pe_beam_geometry_oracle.py. */']
    for name,rows in (('ranges',RANGES),('common',common),('patches',patches)):
        out.append(f'static const uint32_t ATK27_beam_{name}[][2]={{')
        out += [f'    {{0x{a:X}u,0x{b:X}u}},' for a,b in rows];out.append('};')
    out.append('static const struct { unsigned entry,first,end; uint32_t args[3],result; uint64_t hash; } ATK27_beam_cases[]={')
    for e,a,b,args,v,h in cases:
        params=','.join(f'0x{x:08X}u' for x in args)
        out.append(f'    {{{e},{a},{b},{{{params}}},0x{v:08X}u,UINT64_C(0x{h:016X})}},')
    out.append('};')
    if '--write-header' in sys.argv:(ROOT/'pc_port/tests/retail_beam_geometry_cases.h').write_text('\n'.join(out)+'\n')
    print(f'PASS: {len(cases)} complete original beam direction / collision cases')

if __name__=='__main__':main()
