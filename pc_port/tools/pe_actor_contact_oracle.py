#!/usr/bin/env python3
"""Compare actor contact, rollback, callbacks and task retirement with original MIPS."""
import hashlib
import struct
import sys
from pe_battle_hud_oracle import ROOT, execute

ENTRIES=(0x80036448,0x80035F54,0x8001D268,0x80012774)
RANGES=((0,0x100),(0x1FFFFC,4),(0x9CDFC,4),(0x9D1A0,4),(0x9D20C,4),(0x9D254,4),(0x9D308,4),(0x150000,0x5000))
CASES=[dict(entry=0,count=n) for n in (0,1,2,4)]
for model in (0,1):
    for battle in (0,2):
        for distance in (0,164,165,166,192,193,194,599,600,601):
            CASES.append(dict(entry=0,model=model,battle=battle,distance=distance))
for side in (0,1):
    for flag in (0x20,0x20000,0x2040000,0x1000000):
        for aya in (0,1,2):CASES.append(dict(entry=0,side=side,flag=flag,aya=aya))
for scale in (0,2049,4096,65535):
    for radius in (-32768,-65,0,65,32767):
        CASES.append(dict(entry=0,scale=scale,radius=radius,battle=2,distance=193))
for duplicate in (0,1,16,17):
    for peer in (100,101,65535):CASES.append(dict(entry=0,duplicate=duplicate,peer=peer,model=0))
for parent in (0,1,2):
    for movement in (-3,0,3):CASES.append(dict(entry=0,parent=parent,count=4,movement=movement))
for sphere_count in (0,1,3):
    for distance in (0,59,60,61):
        for reverse in (0,1):
            CASES.append(dict(entry=0,sphere_count=sphere_count,sphere_distance=distance,reverse=reverse,battle=2,radius=0))
for types in ((2,3),(0,0),(2,0)):
    CASES.append(dict(entry=0,types=types,battle=2,radius=0))
for target in range(4):
    for parent in (0,1,2):CASES.append(dict(entry=1,target=target,parent=parent,count=4))
for state in range(8):
    for hp in (-1,0,1):CASES.append(dict(entry=2,state=state,hp=hp))
for tag in (-1,0,3,4,127,255,65539):
    for slot in range(4):CASES.append(dict(entry=2,tag=tag,slot=slot))
for kind in (0,1,2):CASES.append(dict(entry=2,kind=kind))
CASES.append(dict(entry=2,missing=1))
for actor_flags in (0,16):
    for mask in (0,1,2,4,7):CASES.append(dict(entry=3,actor_flags=actor_flags,mask=mask))

# Retail lbu at 1D280 accepts physical RAM, including zero between attacks.
# Kind 1 must still acknowledge a hit: a null-pointer early return is wrong.
for entry in (0,2):
    for action in (0,7,0x80,0x1FFFFF,0x80000000,0x80000007,0x80000080,0x801FFFFF):
        for kind in (0,1,2,255):
            CASES.append(dict(entry=entry,action=action,kind=kind,battle=2,radius=0))
# Run39 callback arguments and enemy state at the recorded crash.
CASES.append(dict(entry=2,action=0,kind=0,state=3,tag=16,hp=1000072))


def fixture(exe,c):
    r=bytearray(0x200000)
    r[0x10000:0x10000+len(exe)-0x800]=exe[0x800:]
    for a,n in RANGES:r[a:a+n]=bytes(n)
    def sw(a,v):struct.pack_into('<I',r,a&0x1FFFFF,v&0xFFFFFFFF)
    def sh(a,v):struct.pack_into('<H',r,a&0x1FFFFF,v&65535)
    def sb(a,v):r[a&0x1FFFFF]=v&255
    actors=[0x80150000+i*0x280 for i in range(4)]
    count=c.get('count',2)
    sw(0x9D20C,actors[0] if count else 0);sw(0x9D254,actors[c.get('aya',0)])
    sw(0x9D1A0,0x4040|c.get('battle',0));sw(0x9D308,65534)
    for i,a in enumerate(actors):
        record=0x80152000+i*0x100;model=0x80153000+i*0x100;spheres=0x80153800+i*0x100
        sw(a,record);sw(a+4,actors[i+1] if i+1<count else 0)
        sb(a+12,c.get('types',(0,2))[i%2]);sb(a+13,i+7);sh(a+36,100+i)
        sh(a+38,c.get('scale',4096));sw(a+0x98,0x2040000)
        sw(a+0x1AC,model if c.get('model',1) else 0);sw(a+0x1B4,model)
        sw(a+0x234,spheres);sb(model+3,c.get('sphere_count',1));sw(a+0x194,0x8001D268)
        sw(a+0x1A0,0x80155000+i*32)
        for off in (0x224,0x230):sh(a+off,c.get('radius',65 if i==0 else 100) if off==0x224 else 300)
        distance=c.get('distance',100)*i
        sh(a+0x21C,distance);sh(a+0x228,distance)
        sh(a+58,0xABC);sh(a+82,0xDEF)
        for j in range(3):
            sw(a+40+j*4,(distance+j+c.get('movement',3)*(1 if i==0 else -1))*65536)
            sw(a+64+j*4,(distance+j)*65536)
        sw(a+0x1A4,0xABCD);sw(a+0x1A8,0x1234)
        sw(record+16,c.get('hp',10));sw(record+24,model);sb(model,c.get('kind',1))
        sw(record,c.get('state',0)<<21)
        for j in range(32):sb(record+0x7C+j,255)
        slot=c.get('slot',0);sw(record,(c.get('state',0)<<21)|(slot<<21))
        for j in range(4):sb(record+0x7C+slot*4+j,j)
        for j in range(3):
            sh(spheres+j*12,c.get('sphere_distance',0)*i+j*150)
            sh(spheres+j*12+6,j if not c.get('reverse') else 3-j)
            sh(spheres+j*12+8,30)
    sw(actors[c.get('side',0)]+0x98,c.get('flag',0x2040000))
    if c.get('parent'):
        sw(actors[1]+0x18C,actors[0]);sw(actors[2]+0x18C,actors[0])
        if c['parent']==2:sw(actors[3]+0x18C,actors[1])
    pool=0x80154000;sw(0x9CDFC,pool)
    for i in range(32):
        a=pool+i*44
        for j in range(11):sw(a+j*4,0xCA000000+j)
        sw(a+36,a+44 if i<31 else 0)
    if 'duplicate' in c:
        t=0x80153F00;sw(actors[0]+0xA4,t);sh(t+8,c['duplicate']);sw(t+12,c['peer'])
    if c['entry']==3:
        sw(actors[0]+0x98,c['actor_flags'])
        for chain in range(3):
            t=0x80151000+chain*0x100;sw(actors[0]+0xA0+chain*4,t)
            for i in range(3):
                sh(t+i*44+8,16 if c['mask']&(1<<i) else 1)
                sw(t+i*44+36,t+(i+1)*44 if i<2 else 0)
                sw(t+i*44+40,t+(i-1)*44 if i else 0)
    if 'action' in c:
        sw(0x80152118,c['action']);sb(c['action'],c['kind'])
    if c.get('missing'):sw(actors[1],0)
    args=((),(actors[c.get('target',0)],),(actors[0],7,actors[1],c.get('tag',3)&0xFFFFFFFF),())[c['entry']]
    return r,args


def fingerprint(r):
    h=14695981039346656037
    for a,n in RANGES:
        for b in r[a:a+n]:h=((h^b)*1099511628211)&0xFFFFFFFFFFFFFFFF
    return h


def main():
    exe=(ROOT/'build/disc1.candidate.exe').read_bytes()
    assert hashlib.sha1(exe).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
    for a,n,h in ((0x36448,608,'10e6b058a1eb554abf36c1def4c7b7f23aab0c4c6738fefbb65980e3043a19c2'),
                  (0x35F54,50,'a399991f1b401026d752c1f1485df6b677898488df8fac49b6b59a004819ac2e'),
                  (0x1D268,54,'1d6b5fb202b307fff1746b739f662efe03610dbd170fe949ed412f8b6bf9161c'),
                  (0x12774,55,'29fe15125ee3dc5a9d7c9578e7fd0da61e0bab15b24127e4838b620c0ac379c9')):
        assert hashlib.sha256(exe[a-0x10000+0x800:a-0x10000+0x800+n*4]).hexdigest()==h
    common=None;patches=[];cases=[]
    for k,c in enumerate(CASES):
        r,args=fixture(exe,c)
        initial={a+i:struct.unpack_from('<I',r,a+i)[0] for a,n in RANGES for i in range(0,n,4)}
        if common is None:common={a:v for a,v in initial.items() if v}
        first=len(patches);patches.extend((a,v) for a,v in initial.items() if v!=common.get(a,0))
        execute(r,ENTRIES[c['entry']],args,instruction_budget=900000)
        cases.append((c['entry'],first,len(patches),args,fingerprint(r)))
        if '--dump' in sys.argv:(ROOT/f'local/live/contact-oracle-{k}.bin').write_bytes(r)
        print(k,c,hex(cases[-1][-1]),flush=True)
    out=['/* Generated by pe_actor_contact_oracle.py; synthetic fixtures, original MIPS execution. */']
    for name,rows in (('ranges',RANGES),('common',sorted(common.items())),('patches',patches)):
        out.append(f'static const uint32_t SEW18_{name}[][2]={{')
        out.extend(f'    {{0x{a:X}u,0x{b:X}u}},' for a,b in rows);out.append('};')
    out.append('static const struct { unsigned entry,first,end; uint32_t args[4]; uint64_t hash; } SEW18_cases[]={')
    for e,a,b,args,h in cases:
        params=','.join(f'0x{x:08X}u' for x in args) or '0'
        out.append(f'    {{{e},{a},{b},{{{params}}},UINT64_C(0x{h:016X})}},')
    out.append('};')
    if '--write-header' in sys.argv:(ROOT/'pc_port/tests/retail_actor_contact_cases.h').write_text('\n'.join(out)+'\n')
    print('PASS:',len(cases),'complete original actor contact cases')

if __name__=='__main__':main()
