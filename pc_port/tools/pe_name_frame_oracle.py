#!/usr/bin/env python3
"""Original opening-name handshake, field menu frames and palette fades.

StoreImage's GPU readback and standalone presentation require hardware tests;
these original instruction cases cover all remaining opening frame paths.
"""
import hashlib
import struct
import sys
from pe_battle_hud_oracle import ROOT,execute
from pe_name_display_oracle import fixture as display_fixture
from pe_scripted_exit_oracle import words

RANGES=((0,128),(0x91694,12),(0x9CDB0,0x600),(0xA1870,0x1840),
        (0xA76A4,0x50),(0xA8028,0x2C),(0xB0CD8,16),(0xBCDC8,0xC0),
        (0xBCF88,4),(0xC0DC0,0x1330),(0x140000,0x18000),
        (0x160000,0x4000),(0x170000,0x4000))
ENTRIES=(0x8004DCA4,0x80016F10,0x80046334,0x8004F464,0x8005C488,
         0x80042B6C,0x800339A0,0x80042D40,0x80042F44,0x8005E788,
         0x8005C498,0x800355F8,0x8005E588)
CASES=[]
def case(entry=0,**kw):CASES.append(dict(entry=entry,**kw))
for alternate in (0,1):
    for existing in (0,13,23,19):case(alternate=alternate,existing=existing)
for overlay in (0,0x1000,0x8000,0xFFFF):
    for task_flags in (0,32,0xFFFF):case(1,overlay=overlay,task_flags=task_flags)
for lock in (0,1,7):
    for loader in (0,1,2,3,4):case(2,lock=lock,loader=loader)
case(3);case(4)
for callback in (0,0x8005C488,0x80062F9C):
    for count in (0,2,3,4,0xFFFFFFFF):case(5,callback=callback,count=count)
for style in (0,1,2,3,0x100,0x103):case(6,style=style)
for frame,step in ((0,-1),(0,0),(0,1),(4,-1),(4,0),(4,1),(11,0),(11,1)):
    for light in (0,72,255):case(7,fade_frame=frame,step=step,light=light)
for height in (-1,0,1,3,32):case(7,height=height)
for state in (0,1,2,4,5,6,7,8,0xFFFFFFFF):case(8,state=state)
case(9)
for group in (17,23,24,25):
    for bank in (0,1):case(10,group=group,bank=bank)
for entry in (10,11):
    for name in ('','104830','1011121314151617','0f0f0f'):
        case(entry,group=25,name=name,confirm=1)
    for timer in (-1,0,1):case(entry,timer=timer)
    case(entry,pending=1);case(entry,callback=0x8005C488,count=3)
    case(entry,lock=1,loader=0);case(entry,lock=1,loader=1)
    for state in (1,2,4,5,6,7):case(entry,state=state)
for bank in (0,1):
    for fill in (0,0x55,0xA7,0xFF):case(12,bank=bank,descriptor_fill=fill)

def fixture(exe,c):
    r,s,_=display_fixture(exe,dict(c,entry=10))
    def put(a,b):a&=0x1FFFFF;r[a:a+len(b)]=b;s[a:a+len(b)]=b
    def sw(a,v):put(a,struct.pack('<I',v&0xFFFFFFFF))
    def sh(a,v):put(a,struct.pack('<H',v&65535))
    def word(a):return struct.unpack_from('<I',r,a&0x1FFFFF)[0]
    for a,n in ((0x10E38,16),):put(a,exe[a-0x10000+0x800:a-0x10000+0x800+n])
    if c['entry'] in (0,1):
        execute(r,0x80062F9C)
        if c.get('existing'):
            execute(r,0x80062D2C,(c['existing'],0,0,0))
    sw(0xB0CD8,c.get('overlay',0));sw(0xBCF88,0x2034)
    sw(0x9D1A0,0x4004);sw(0x9D300,0x80149E40);sh(0x149E48,c.get('task_flags',32))
    sw(0x149E00,0x80149E10);sw(0x149E10,0);sw(0x9CE00,0x80149F10)
    sw(0x9CFB0,c.get('lock',0));put(0xB0CE6,bytes((c.get('loader',0),)))
    sw(0x9D008,0);sw(0x9D030,c.get('timer',0));sw(0x9D034,c.get('pending',0))
    sw(0xA1870,c.get('callback',0));sw(0xA1874,c.get('count',0))
    sw(0x9CED8,c.get('state',0));sw(0x9CEE0,12);sw(0x9CEE8,c.get('fade_frame',4))
    sw(0x9CEE4,c.get('step',1));sw(0x9CEEC,c.get('light',72));sw(0x9CEDC,c.get('height',1))
    put(0xA1878,bytes(i*23 for i in range(12)))
    sw(0xB0E50,0x80150000);sw(0xB0E54,0x80154000)
    if c['entry']==12:
        put(0xA2180,bytes((c['descriptor_fill'],))*240)
        sw(0xB0E38,0x80164000);sw(0xB0E3C,0x80168000)
        sw(0x9D120,0)
    put(0x150000,b''.join(struct.pack('<H',v) for v in (0,0x8000,0x1F,0x3E0,0x7C00,0xFFFF,0x7FFF,0x8421))*1024)
    put(0x154000,bytes((i*11+3)&255 for i in range(0x4000)))
    if c.get('confirm'):
        sw(0x9D0E8,1);sw(0x9D0F0,32);sw(0x9D26C,0)
        sw(word(0x9D15C)+68,0)
    args=((0,),(0x80149E00,),(),(),(),(),(c.get('style',0),),(),(),(1,),
          (0x800A76D8,),(),())[c['entry']]
    for a,n in RANGES:s[a:a+n]=r[a:a+n]
    return r,s,tuple(v&0xFFFFFFFF for v in args)

def run(r,c,args):
    if c['entry']==12:
        execute(r,ENTRIES[12])
        return execute(r,0x8005E6F0)
    if c['entry']==11:
        word=lambda a:struct.unpack_from('<I',r,a&0x1FFFFF)[0]
        return execute(r,ENTRIES[11],stop_at=(0x800356D0,),initial_regs={3:word(0x9D1A0),4:word(0xB0CD8)})
    return execute(r,ENTRIES[c['entry']],args,instruction_budget=750000)

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
        regs=run(r,c,args);result=regs[2] if c['entry'] in (1,10) else 0
        cases.append((c['entry'],first,len(patches),args,result,fingerprint(r)))
        print(k,c,hex(result),hex(fingerprint(r)),flush=True)
        if '--dump' in sys.argv:(ROOT/f'pc_port/build/name-frame-oracle-{k}.bin').write_bytes(r)
    out=['/* Generated by pe_name_frame_oracle.py --write-header. */']
    for name,rows in (('ranges',RANGES),('common',common),('patches',patches)):
        out.append(f'static const uint32_t NAM9_name_frame_{name}[][2]={{')
        out.extend(f'    {{0x{a:X}u,0x{b:X}u}},' for a,b in rows);out.append('};')
    out.append('static const struct { unsigned entry,first,end; uint32_t args[1],result; uint64_t hash; } NAM9_name_frame_cases[]={')
    for e,a,b,args,result,h in cases:
        params=','.join(f'0x{x:08X}u' for x in args) or '0'
        out.append(f'    {{{e},{a},{b},{{{params}}},0x{result:08X}u,UINT64_C(0x{h:016X})}},')
    out.append('};')
    if '--write-header' in sys.argv:(ROOT/'pc_port/tests/retail_name_frame_cases.h').write_text('\n'.join(out)+'\n')
    print(f'PASS: {len(cases)} original name frame cases')

if __name__=='__main__':main()
