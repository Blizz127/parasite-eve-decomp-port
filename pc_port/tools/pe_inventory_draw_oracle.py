#!/usr/bin/env python3
"""Execute original inventory panels, command lists and numeric call graphs."""
import hashlib
import struct
import sys
from pe_battle_hud_oracle import ROOT,execute
from pe_inventory_menu_oracle import fixture as menu_fixture
from pe_scripted_exit_oracle import words

RANGES=((0,128),(0x9CDB0,0x600),(0x9E068,0xBD0),(0xA1870,0x1A00),
        (0xA76A4,0x50),(0xA8028,0x2C),(0xB00E8,0xD8),(0xB0E38,8),
        (0xB6920,0x38),(0xC0DC0,0x1330),(0x140000,0x20000),
        (0x160000,0x4000),(0x170000,0x4000))
ENTRIES=(0x800602D0,0x80060528,0x8006055C,0x80060590,0x800605C4,0x800605F8,
         0x800536B8,0x80043B0C,0x80043C64,0x8004905C,0x80050748,0x8004F838,
         0x80051E48,0x8005DBF8,0x80021080,0x80052534)
CASES=[]
def case(entry=0,**kw):CASES.append(dict(entry=entry,**kw))
for digits in (0,1,2,3,6,10):
    for value in (-12345,-99,-1,0,1,9,10,99,100,12345,2147483647):case(digits=digits,value=value)
for entry in range(1,6):
    for value in (-1000000,-100,-1,0,9,100,999999,2147483647):case(entry,value=value)
for item in (-32768,-1,0,1,255,256,383,384,511,512,520,521):case(6,item=item)
for kind in (1,9,19):
    for renamed in (0,1):case(6,item=256,kind=kind,renamed=renamed)
for index in (-1,0,1,2):case(6,index=index,item=256)
for level in (0,1,97,98,99,255):
    for armor in (-1,1):case(7,level=level,armor=armor)
case(7,level=0,experience=2000,renamed=1)
for bonus in (-99,-1,0,1,99):
    for fraction in (0,64,128):case(8,bonus=bonus,fraction=fraction)
for value in (-99999,-1,0,99999,2147483647):
    for wide in (0,1):case(9,value=value,wide=wide)
for battle in (0,1):
    for enabled in (0,0x1F,0x1EF,0x1FF):
        for index in (-1,0,3,4,8,9):case(10,battle=battle,enabled=enabled,index=index,selected=4)
for unavailable in (0,1):
    for status in (0,0x10000):case(10,battle=1,index=4,selected=4,unavailable=unavailable,status=status,enabled=0x1F)
for battle in (0,1):
    for bank in (0,1):
        for enabled in (0,0x3D,0x1FF):case(11,battle=battle,bank=bank,enabled=enabled,frame=31)
case(11,battle=0,hud=0);case(11,battle=1,scroll=8);case(11,battle=1,scroll=-8)
case(12);case(13)
for entry in (14,15):
    for battle in (0,1):
        for unavailable in (0,1):
            for status in (0,0x10000):case(entry,battle=battle,unavailable=unavailable,status=status)
for entry in (0,6,7,8,9,10,11):case(entry,offset=0x3FE0,value=-123,battle=1)

def fixture(exe,c):
    r,s,_=menu_fixture(exe,dict(c,entry=0))
    def put(a,b):a&=0x1FFFFF;r[a:a+len(b)]=b;s[a:a+len(b)]=b
    def sw(a,v):put(a,struct.pack('<I',v&0xFFFFFFFF))
    def sh(a,v):put(a,struct.pack('<H',v&65535))
    def sb(a,v):put(a,bytes((v&255,)))
    def word(a):return struct.unpack_from('<I',r,a&0x1FFFFF)[0]
    def find(kind,id):
        p=word(0x9D154)
        while p:
            if word(p+32)==kind and word(p+36)==id:return p
            p=word(p)
        raise AssertionError((kind,id))
    execute(r,0x800438EC)
    node=find(2,0);sw(node+68,c.get('selected',0));sw(node+72,0);sw(node+96,c.get('scroll',0))
    sw(0x9CEF4,node);sw(0x9D15C,node);sw(0x9D1A0,c.get('battle',0)*2)
    sw(0x9CEF8,c.get('hud',1));sw(0x9D108,c.get('bank',0));sb(0x9CE3C,c.get('unavailable',0))
    sw(0x9CDDC,c.get('bank',0));sw(0x956AC,c.get('frame',0))
    sw(0x9CF30,c.get('wide',0));sw(0x9CF40,c.get('fraction',128));sw(0x9CF68,c.get('value',12345))
    for i in range(1,7):sw(0xA1B30+i*4,c.get('bonus',0));sw(0xA18FC+i*4,20-i*7)
    sw(0xA8040,0x15B000-0xA8028)
    for i in range(100):sw(0x15B000+i*4,i*1000)
    sb(0xC0E0A,c.get('level',0));sw(0xC0E00,c.get('experience',100));sb(0xC0E20,0);sb(0xC0E22,c.get('armor',1))
    for i in (57,111,112):put(0x140A00+i*0x80,bytes((32+i%30,255)))
    sw(0xA8034,0x155000-0xA8028)
    sw(0x140808,0x15E000-0x140800);sh(0x15E000,256)
    for i in range(256):sh(0x15E002+i*2,0x204+i*4);put(0x15E204+i*4,bytes((32+i%30,40,255)))
    sw(0x9D048,0x8015C400);sw(0x9D050,2);sw(0x9D03C,10)
    sh(0x15C400,256);sh(0x15C402,257);sh(0x15C400+c.get('index',0)*2,c.get('item',256))
    item=c.get('item',256)
    records=[(0xC0EAC,1),(0xC0ECC,9)]
    if 256<=item<384:records.append((0xC0EAC+(item-256)*32,c.get('kind',1)))
    elif 1<=item<=255:records.append((0x155000+(item-1)*32,c.get('kind',19)))
    elif 512<=item<=520:records.append((0xA1E64+(item-512)*32,c.get('kind',9)))
    for p,kind in records:
        sb(p,2);sb(p+4,1);sb(p+5,16 if c.get('renamed') else 0);sb(p+6,kind)
        sb(p+9,10);sh(p+10,3);sh(p+18,5)
    sw(0x9D254,0x8015C000);sw(0x15C000,0x8015C100);sw(0x9D278,0x8015C100)
    sw(0x15C108,40<<16);sw(0x15C128,80<<16);sh(0x15C110,4500)
    sh(0x15C10C,18);sh(0x15C11C,45);sw(0x15C14C,c.get('status',0))
    for bank in (0,1):sw(0xB0E38+bank*4,0x80170800+bank*0x400)
    sw(0x9D100,0x80160000+c.get('offset',0));sw(0x9D104,0x80160000);sw(0x9D11C,0x80170004)
    args=((c.get('value',123),c.get('digits',3)),*((c.get('value',123),) for _ in range(5)),
          (c.get('index',0),),(find(1,18),),(find(1,45),),(find(1,24),),
          (c.get('index',0),),(node,),(),(),(),())[c['entry']]
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
        regs=execute(r,ENTRIES[c['entry']],args,instruction_budget=750000)
        result=regs[2] if c['entry']>=12 else 0;h=fingerprint(r)
        cases.append((c['entry'],first,len(patches),args,result,h,c.get('frame',0)))
        print(k,c,hex(result),hex(h),flush=True)
        if '--dump' in sys.argv:(ROOT/f'pc_port/build/inventory-draw-oracle-{k}.bin').write_bytes(r)
    out=['/* Generated by pe_inventory_draw_oracle.py --write-header. */']
    for name,rows in (('ranges',RANGES),('common',common),('patches',patches)):
        out.append(f'static const uint32_t INV3_inventory_draw_{name}[][2]={{')
        out.extend(f'    {{0x{a:X}u,0x{b:X}u}},' for a,b in rows);out.append('};')
    out.append('static const struct { unsigned entry,first,end; uint32_t args[2],result; uint64_t hash; uint32_t frame; } INV3_inventory_draw_cases[]={')
    for e,a,b,args,result,h,frame in cases:
        params=','.join(f'0x{x:08X}u' for x in args) or '0'
        out.append(f'    {{{e},{a},{b},{{{params}}},0x{result:08X}u,UINT64_C(0x{h:016X}),{frame}u}},')
    out.append('};')
    if '--write-header' in sys.argv:(ROOT/'pc_port/tests/retail_inventory_draw_cases.h').write_text('\n'.join(out)+'\n')
    print(f'PASS: {len(cases)} original inventory drawing cases')

if __name__=='__main__':main()
