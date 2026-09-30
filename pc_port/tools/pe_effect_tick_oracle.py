#!/usr/bin/env python3
"""Original effect scheduling, room VM, pool allocation and owner handshake.

These cases execute the original complete call graphs. Child animation and
particle drawing callbacks have separate coverage as they are translated.
"""
import hashlib
import struct
import sys
from pe_battle_hud_oracle import ROOT,execute
from pe_scripted_exit_oracle import words

RANGES=((0,4),(0x140000,0x38000),(0x942E0,12),(0x9D1A0,4),(0xB0CD8,4),
        (0xB0E64,4),(0xBCFA4,4),(0xBCF94,4),(0xE10A0,28),
        (0xE2368,4),(0xE27EC,4),(0xF32D0,4),(0xF33E0,4),(0xF3428,4))
ENTRIES=(0x8006F9F0,0x800D413C,0x80069594,0x800D4704,0x800D401C,
         0x800CE560,0x800CE5AC,0x800CE610,0x800CE688,0x800CE78C,
         0x800CE870,0x800CE8F0,0x800D3FD8,0x8006DC18)
CASES=[]
def case(entry=0,**kw):CASES.append(dict(entry=entry,**kw))
for index in (0,10,11,21,22,0xFFFFFFFF):
    for state in (0,1,2,3,4,5,6):case(index=index,state=state)
for state in (1,2,5):
    for code in (0x54,0x55,0x72,0x73,0xBF,0xC0,0xFF):case(state=state,code=code)
case(missing=1);case(missing=2);case(tick=0xFFFFFFFF)
for delay in (0,1,2,65535):case(1,delay=delay)
case(1,program=[(1,6,0xABCD),(4,3,0)])
case(1,program=[(4,0,1)],local=0)
case(1,program=[(4,0,1),(0,0,0)],local=1)
case(1,program=[(3,0,0)],local=0)
case(1,program=[(3,0,0)],local=1)
case(1,program=[(3,0x6006,0)],local=2,jump=1)
case(1,program=[(6,0,0),(0,0,0)])
for notify in (0,1):
    for timer in (0,1,2,63):
        for action in (0,1,2):case(1,notify=notify,timer=timer,action=action,delay=2)
case(1,notify=1,hp=0,action=0,delay=2,timer=2)
case(1,notify=1,status=0x180E,action=0,delay=2)
case(1,notify=1,action=0,cmd=0,delay=2)
case(1,notify=1,empty_body=1);case(1,notify=1,empty_owner=1,delay=2)
case(1,program=[(5,0,0),(4,2,0)],action=1)
case(1,program=[(5,1,0),(4,2,0)],notify=1,action=1)
for flags in (0,0x80,0x84,0x180,0x184):
    for code in (0x54,0x55,0x72,0x73):case(2,flags=flags,code=code,state=4)
case(2,state=2,flags=0x80)
case(3);case(3,notify=1)
case(4,full=1)
for count in (-1,0,1,24):case(5,count=count,size=16)
case(6,count=3,size=8)
for occupied in (0,1,2,3):case(7,occupied=occupied)
case(8);case(9)
for mode in (0,1,2):case(10,mode=mode)
case(11)
for owner_id in (-1,0,64,65,128):case(12,owner_id=owner_id)
case(12,empty_body=1)
for sound in (0,1,2,3):case(13,sound=sound)
for action in (0,1,2):
    for timer in (0,1,2):case(1,notify=1,timer=timer,action_pointer=0,physical_action=action,delay=2)
case(1,notify=1,action_pointer=0,physical_action=0,cmd=0,delay=2)
case(1,notify=1,action_pointer=0,physical_action=0,status=0x180E,delay=2)
case(1,notify=1,action_pointer=0,physical_action=1,program=[(5,1,0),(4,2,0)])
case(1,notify=1,action_pointer=0,physical_action=1)

def fixture(exe,c):
    r=bytearray(0x200000);r[0x10000:0x10000+len(exe)-0x800]=exe[0x800:]
    s=bytearray(0x200000)
    def put(a,b):r[a:a+len(b)]=b;s[a:a+len(b)]=b
    def sw(a,v):put(a,struct.pack('<I',v&0xFFFFFFFF))
    def sh(a,v):put(a,struct.pack('<H',v&65535))
    def sb(a,v):put(a,bytes((v&255,)))
    for a,n in RANGES:put(a,bytes(n))
    sw(0x942E0,0x80170000);sw(0x942E4,0x80150000);sw(0x942E8,0x80160000)
    for i in range(86):sw(0x170000+i*4,0 if c.get('missing')==1 else 0x80171000)
    sw(0x17100C,0x800D4704);sw(0x171010,0 if c.get('missing')==2 else 0x800D413C)
    index=c.get('index',0);index=index if index<22 else 0
    a=0x150000+index*0xA0C if index<11 else 0x160000+(index-11)*0x10C
    sb(a,c.get('state',1));sb(a+1,c.get('code',0x75));sb(a+2,7);sb(a+3,9)
    sw(a+4,c.get('tick',77));sw(a+8,0 if c.get('empty_owner') else 0x80140000)
    sw(a+12,0x80172000);sw(a+16,0x80000000+a+0x90);sw(a+0x8C,0x80173000)
    sb(a+25,c.get('notify',0));sh(a+26,c.get('delay',0));sh(a+30,c.get('local',0))
    for i in range(8):sh(a+44+i*12,0 if c.get('full') else 65535)
    for i,op in enumerate(c.get('program',[(0,0,0)])):
        for j,v in enumerate(op):sh(0x172000+i*6+j*2,v)
    if c.get('jump'):
        sh(0x6000,4);sh(0x6002,1);sh(0x6004,0)
    sw(0x140000,0 if c.get('empty_body') else 0x80144000)
    sw(0x140238,0x80146000);sb(0x14000E,c.get('cmd',7))
    for i in range(3):sw(0x140028+i*4,(-102+i*197)<<16)
    sw(0x144000,(c.get('timer',0)<<24)|c.get('status',0));sw(0x144008,c.get('owner_id',32))
    sw(0x144010,c.get('hp',40));sw(0x144018,c.get('action_pointer',0x80145000));sb(0x145000,c.get('action',1))
    sb(0,c.get('physical_action',0))
    sw(0xF32D0,0x80000000+a);sw(0xE2368,0x80000000+a+12)
    sw(0xF33E0,0x80147000);sw(0x147008,0x80148000);sw(0xE27EC,29)
    sw(0x148000,12);sw(0x148004,3)
    for i in range(c.get('occupied',0)):sh(0x14800C+i*12,1)
    sw(0x9D1A0,c.get('flags',0x82));sw(0xB0CD8,0xABCDEFFF)
    sw(0xBCFA4,0x80146000);sh(0xBCF94,153);sh(0xBCF96,-12)
    for i,v in enumerate((0,0,4096,0,4096,0,-4096,0,0)):sh(0x146000+i*2,v)
    for i,v in enumerate((300,-400,700)):sw(0x146014+i*4,v)
    for i,v in enumerate((100,-20,50)):sh(0x149000+i*2,v)
    for i in range(7):sw(0xE10A0+i*4,0x1234+i)
    if c.get('sound'):
        sw(0xB0E64,0x80174000);sw(0x174004,0x40);sw(0x174048,(1<<22)|0x80)
        sw(0x174088,0x73DECD80);sw(0x174084,0x100)
        sb(0x174103,0x75 if c['sound']!=3 else 0x74)
        sw(0x174108,0x200D0000 if c['sound']==1 else 0x400D0000)
    args=((c.get('index',0),),(0x80000000+a,),(),(0x80000000+a,),(0,),
          (0x80148000,c.get('size',16),c.get('count',3)&0xFFFFFFFF,0x8018F018),
          (0x80149000,64,c.get('size',8),c.get('count',3)&0xFFFFFFFF,0x8018F018),
          (0x80148000,),(0x80148000,),(0x80148000,),
          (0x80140000,c.get('mode',0),0x80149000),
          (0x80140000,0,0x80149000,0x80149100),(),(0x75,))[c['entry']]
    return r,s,args

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
        if c['entry'] in (10,11):regs[2]=0 # void C interfaces
        cases.append((c['entry'],first,len(patches),args,regs[2],fingerprint(r)))
        print(k,c,hex(regs[2]),hex(fingerprint(r)),flush=True)
        if '--dump' in sys.argv:(ROOT/f'pc_port/build/effect-tick-oracle-{k}.bin').write_bytes(r)
    out=['/* Generated by pe_effect_tick_oracle.py --write-header. */']
    for name,rows in (('ranges',RANGES),('common',common),('patches',patches)):
        out.append(f'static const uint32_t ATK23_effect_{name}[][2]={{')
        out += [f'    {{0x{a:X}u,0x{b:X}u}},' for a,b in rows];out.append('};')
    out.append('static const struct { unsigned entry,first,end; uint32_t args[5],result; uint64_t hash; } ATK23_effect_cases[]={')
    for e,a,b,args,v,h in cases:
        params=','.join(f'0x{x:08X}u' for x in args) or '0'
        out.append(f'    {{{e},{a},{b},{{{params}}},0x{v:08X}u,UINT64_C(0x{h:016X})}},')
    out.append('};')
    if '--write-header' in sys.argv:(ROOT/'pc_port/tests/retail_effect_tick_cases.h').write_text('\n'.join(out)+'\n')
    print(f'PASS: {len(cases)} complete original effect scheduling / VM / allocation cases')

if __name__=='__main__':main()
