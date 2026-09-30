#!/usr/bin/env python3
"""Original menu input FIFO/repeat timing and name-entry callbacks."""
import hashlib
import struct
import sys
from pe_battle_hud_oracle import ROOT,execute
from pe_name_entry_oracle import fixture as name_fixture
from pe_scripted_exit_oracle import words

RANGES=((0,128),(0x91694,12),(0x9CF00,0x278),(0xA2090,0x1040),(0xB0CD8,4),
        (0xC0DC0,0x1310),(0x140000,0x8000))
ENTRIES=(0x8005E038,0x8005E12C,0x8005E114,0x8004E074,0x8004E2E4)
CASES=[]
def case(entry=0,**kw):CASES.append(dict(entry=entry,**kw))
case()
for bit in range(32):case(pad=1<<bit)
case(pad=0xFFFFFFFF)
for armed in (0,1):
    for buttons in (0,8,0x20000000,0x40000000,0x60000028):
        for previous in (0,0x20,0x4000):case(1,armed=armed,pad=buttons,previous=previous)
for timer in (-302,-300,-298,-94,-92,-90,-6,-4,-2,0,2,16):
    for fast in (0,1):case(1,armed=1,pad=8,previous=0x1000,timer=timer,fast=fast)
case(1,armed=1,pad=0x40000000,previous=64,timer=-6)
case(1,armed=1,pad=8,previous=0x20,full=1)
case(1,armed=1,pad=8,previous=0x20,slots=1)
case(1,armed=1,pad=8,previous=0x20,queued=2)
for value in (-1,0,1,17):case(2,value=value)
for alternate in (0,1):
    for row in (-1,0,1,2,3,4,5,6):case(3,event=0x8000,row=row,alternate=alternate)
    for row,column in ((0,0),(1,2),(4,3)):case(3,event=0x10000,row=row,column=column,alternate=alternate)
for name in ('','10','fa10','104830','1011121314151617'):
    case(3,event=64,name=name)
case(3,event=0);case(3,event=0x800)
for alternate in (0,1):
    for group in ((23,24,25) if not alternate else (24,25)):
        for event in (0,0x10000,0x2000,0x1000,0x4000,0x800):
            case(4,event=event,group=group,alternate=alternate)
for name in ('','0f0f0f','10','104830','fa10'):
    case(4,event=0x10000,group=25,name=name)
    case(4,event=0x800,group=25,name=name)
for row in (0,1):case(4,event=0x10000,group=24,row=row,name='123456')
for event in (0x40,0x800,0x10000):case(4,event=event,group=25,record=1)
for event in (0x3000,0x5000,0x1800,0x12000):case(4,event=event,group=23)

def fixture(exe,c):
    r,s,_=name_fixture(exe,dict(c,entry=0))
    def put(a,b):r[a:a+len(b)]=b;s[a:a+len(b)]=b
    def sw(a,v):put(a,struct.pack('<I',v&0xFFFFFFFF))
    def sh(a,v):put(a,struct.pack('<H',v&65535))
    # Original table layout, distinct deterministic glyph pages, and defaults.
    sh(0x140900,120)
    for i in range(120):
        offset=0x100+i*0x80
        sh(0x140902+i*2,offset)
        put(0x140900+offset,bytes((16+(j+i)%90) for j in range(70))+b'\xff')
    put(0x140900+0x100+30*0x80,b'\x10\x48\x30\xff')
    # Keep the independent item-name table away from the menu string pages.
    sw(0x140808,0x7000);sh(0x147800,1);sh(0x147802,16);put(0x147810,b'\x40\x41\x42\xff')
    execute(r,0x8004DD64,((c.get('record',0)-1)&0xFFFFFFFF,))
    for a,n in RANGES:s[a:a+n]=r[a:a+n]
    def word(a):return struct.unpack_from('<I',r,a&0x1fffff)[0]
    def node(kind,id):
        p=word(0x9D154)
        while p:
            if word(p+32)==kind and word(p+36)==id:return p
            p=word(p)
        raise AssertionError((kind,id))
    selected=node(2,c.get('group',17) if c['entry']==4 else 17)
    sw((selected&0x1fffff)+72,c.get('row',0));sw((selected&0x1fffff)+68,c.get('column',0))
    sw(0x9D15C,selected)
    p=word(0x9D0C0)&0x1fffff
    put(p,bytes.fromhex(c.get('name','104830'))+b'\xff')
    for i in range(20):sw(0xA2090+i*12,0x800A2090+(i+1)*12 if i<19 else 0)
    slots=c.get('slots',20)
    if slots<20:sw(0xA2090+(slots-1)*12,0)
    sw(0x9D0DC,0 if c.get('full') else 0x800A2090)
    queued=c.get('queued',0)
    sw(0x9D0E0,0x80140500 if queued else 0);sw(0x9D0E4,0x80140500+(queued-1)*12 if queued else 0)
    for i in range(queued):
        a=0x140500+i*12;sw(a,0x80140500+(i+1)*12 if i+1<queued else 0);sw(a+4,4);sw(a+8,0x80+i)
    sw(0x9D0E8,c.get('armed',0));sw(0x9D0F0,c.get('previous',0));sw(0x9D0F8,c.get('timer',0))
    sw(0x9D26C,c.get('pad',0));sw(0xB0E08,0)
    args=((),(c.get('fast',0),),(c.get('value',0),),
          (node(1,17),c.get('event',0)),(node(1,23),c.get('event',0)))[c['entry']]
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
        result=0 if c['entry'] in (1,2) else regs[2]
        cases.append((c['entry'],first,len(patches),args,result,fingerprint(r)))
        print(k,c,hex(result),hex(fingerprint(r)),flush=True)
        if '--dump' in sys.argv:(ROOT/f'pc_port/build/name-input-oracle-{k}.bin').write_bytes(r)
    out=['/* Generated by pe_name_input_oracle.py --write-header. */']
    for name,rows in (('ranges',RANGES),('common',common),('patches',patches)):
        out.append(f'static const uint32_t NAM3_input_{name}[][2]={{')
        out.extend(f'    {{0x{a:X}u,0x{b:X}u}},' for a,b in rows);out.append('};')
    out.append('static const struct { unsigned entry,first,end; uint32_t args[2],result; uint64_t hash; } NAM3_input_cases[]={')
    for e,a,b,args,v,h in cases:
        params=','.join(f'0x{x:08X}u' for x in args) or '0'
        out.append(f'    {{{e},{a},{b},{{{params}}},0x{v:08X}u,UINT64_C(0x{h:016X})}},')
    out.append('};')
    if '--write-header' in sys.argv:(ROOT/'pc_port/tests/retail_name_input_cases.h').write_text('\n'.join(out)+'\n')
    print(f'PASS: {len(cases)} complete original name input cases')

if __name__=='__main__':main()
