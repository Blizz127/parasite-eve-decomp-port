#!/usr/bin/env python3
"""Original name bar, help, timer and complete naming window drawing."""
import hashlib
import struct
import sys
from pe_battle_hud_oracle import ROOT,execute
from pe_menu_window_oracle import fixture as window_fixture
from pe_scripted_exit_oracle import words

RANGES=((0,128),(0x9CDB0,0x600),(0xA2180,0xF30),(0xA76A4,36),
        (0xA8028,0x2C),(0xC0DC0,0x1330),(0x140000,0x10000),
        (0x160000,0x4000),(0x170000,0x4000))
ENTRIES=(0x8005F874,0x8005FB74,0x8006006C,0x80052894,0x8005DD3C,
         0x800534E4,0x80053648,0x80061A3C,0x8004DF74,0x8004C608,0x80062FEC)
CASES=[]
def case(entry=0,**kw):CASES.append(dict(entry=entry,**kw))
for entry in (0,1):
    for value in (-2147483648,-1000,-99,-1,0,1,9,10,99,100,999,1000,2147483647):
        case(entry,value=value)
for small in (0,1):
    for value in (-2147483648,-3661,-1,0,59,60,3599,3600,3661,359999,360000,360001,2147483647):
        case(2,value=value,small=small)
for index in (0,1,2):
    for ticks in (0,59,60,3600,0xFFFFFFFF):case(3,index=index,ticks=ticks)
for index in (0,1,79,119,120,0xFFFFFFFF):case(4,index=index)
for kind in (0,1,4,5,7,8,9,18,19,22):
    for full in (0,1):case(5,kind=kind,full=full)
case(5,kind=1,full=1,maximum=1200,ammo=999)
case(5,kind=1,maximum=-12,ammo=0)
case(5,kind=1,null=1)
for kind in (1,9):
    for renamed in (0,1):case(6,kind=kind,renamed=renamed)
for width,height in ((0,0),(9,4),(320,224),(65535,65535)):
    for shade in (0,1):case(7,width=width,height=height,shade=shade)
for record in (0,1,2):
    for name in ('','104830','fa10fb11fc12','1011121314151617'):
        case(8,record=record,name=name)
for group in (2,3,4,9,10,12,15,17,18,19,20,21,22,23,24,25,26,27,28,30,
              31,32,33,34,35,40,41,42,43,44,45,46,47,48,49,53,54,55,56,57,58,59,61):
    case(9,group=group,index=1,ticks=219660)
case(9,focused=0);case(9,group=32,index=4);case(9,group=59,index=0,profile=2)
for alternate in (0,1):
    for group in ((17,23,24,25) if not alternate else (17,24,25)):
        for bank in (0,1):case(10,alternate=alternate,group=group,bank=bank,frame=31)
for entry in (0,1,2,5,7,8,9,10):case(entry,offset=0x3FE0)

def fixture(exe,c):
    r,s,_=window_fixture(exe,dict(c,entry=0))
    def put(a,b):a&=0x1FFFFF;r[a:a+len(b)]=b;s[a:a+len(b)]=b
    def sw(a,v):put(a,struct.pack('<I',v&0xFFFFFFFF))
    def sh(a,v):put(a,struct.pack('<H',v&65535))
    def sb(a,v):put(a,bytes((v&255,)))
    def word(a):return struct.unpack_from('<I',r,a&0x1FFFFF)[0]
    def node(kind,id):
        p=word(0x9D154)
        while p:
            if word(p+32)==kind and word(p+36)==id:return p
            p=word(p)
        raise AssertionError((kind,id))
    help_window=execute(r,0x80062D2C,(19,0,0,0))[2]
    sw(help_window+48,0x8004C608)
    group=c.get('group',17)
    selected=node(2,group if group in (17,23,24,25) else 17)
    if c['entry']==9:
        sw(selected+36,group);sw(selected+52,1);sw(selected+68,c.get('index',0));sw(selected+72,0)
    sw(0x9D15C,selected if c.get('focused',1) else 0)
    sw(0x9D108,c.get('bank',0));sw(0x9CF0C,c.get('profile',0))
    sw(0x9D13C,0x1234);sw(0x9D140,132);sw(0x9D144,164)
    for i in range(3):sw(0xA76A4+i*12,c.get('ticks',219660))
    # Independent help table; index 1 exercises the original signed offset.
    sw(0x140810,0x7400);sh(0x147C00,120)
    for i in range(120):
        off=-16 if i==1 else 0x100+i*4
        sh(0x147C02+i*2,off);put(0x147C00+off,bytes((16+i%80,32,255)))
    for i in (115,116,117):
        put(0x140A00+i*0x80,bytes(16+(j+i)%90 for j in range(99))+b'\xff')
    for address in (0xC0DE0,0xC0DF0,0xC20A4,0xC20B4):
        put(address,bytes.fromhex(c.get('name','104830'))+b'\xff')
    record=0x800C0EAC
    kind=c.get('kind',1);sb(record,2);sb(record+4,1);sb(record+5,16 if c.get('renamed') else 0)
    sb(record+6,kind);sb(record+9,10);sh(record+18,c.get('maximum',15)-10)
    sh(record+10,c.get('ammo',15 if c.get('full') else 3))
    if c['entry'] in (8,10) and c.get('record'):
        record=word(0x9D004);sb(record,2);sh(record+10,3)
    args=((c.get('value',0),),(c.get('value',0),),(c.get('value',3661),c.get('small',0)),
          (c.get('index',2),),(c.get('index',0),),(record,0 if c.get('null') else 0x800C20A4),
          (record,),(c.get('width',9),c.get('height',4),c.get('shade',0)),
          (node(1,26),),(help_window,),())[c['entry']]
    for a,n in RANGES:s[a:a+n]=r[a:a+n]
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
        regs=execute(r,ENTRIES[c['entry']],args);result=regs[2] if c['entry'] in (3,4) else 0
        cases.append((c['entry'],first,len(patches),args,result,fingerprint(r),c.get('frame',0)))
        print(k,c,hex(result),hex(fingerprint(r)),flush=True)
        if '--dump' in sys.argv:(ROOT/f'pc_port/build/name-display-oracle-{k}.bin').write_bytes(r)
    out=['/* Generated by pe_name_display_oracle.py --write-header. */']
    for name,rows in (('ranges',RANGES),('common',common),('patches',patches)):
        out.append(f'static const uint32_t NAM8_name_display_{name}[][2]={{')
        out.extend(f'    {{0x{a:X}u,0x{b:X}u}},' for a,b in rows);out.append('};')
    out.append('static const struct { unsigned entry,first,end; uint32_t args[3],result; uint64_t hash; uint32_t frame; } NAM8_name_display_cases[]={')
    for e,a,b,args,result,h,frame in cases:
        params=','.join(f'0x{x:08X}u' for x in args) or '0'
        out.append(f'    {{{e},{a},{b},{{{params}}},0x{result:08X}u,UINT64_C(0x{h:016X}),{frame}u}},')
    out.append('};')
    if '--write-header' in sys.argv:(ROOT/'pc_port/tests/retail_name_display_cases.h').write_text('\n'.join(out)+'\n')
    print(f'PASS: {len(cases)} complete original name display cases')

if __name__=='__main__':main()
