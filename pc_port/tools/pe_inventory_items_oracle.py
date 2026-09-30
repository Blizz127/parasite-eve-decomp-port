#!/usr/bin/env python3
"""Original item availability, rearrangement, construction and drawing graphs.

Runs the authenticated executable without replacing its callees. Item-window
input (44444) is covered separately; list transfer/cancel runs here in full.
"""
import hashlib
import struct
import sys
from pe_battle_hud_oracle import ROOT,execute
from pe_inventory_help_oracle import fixture as help_fixture,RANGES,fingerprint
from pe_scripted_exit_oracle import words

ENTRIES=(0x80055760,0x80050260,0x80055FE0,0x80057C54,0x8005401C,
         0x80054240,0x8005FA3C,0x80063158,0x80062F1C,0x80064C80,
         0x80050804,0x8004F8D0,0x800447F0,0x80044174,0x80064EB4,
         0x80062FEC,0x80063E0C,0x8004E970)
RESULT_ENTRIES={2,3,4,5,16,17}
CASES=[]
def case(entry=0,**kw):CASES.append(dict(entry=entry,**kw))
for profile in (0,1):
    for battle in (0,1):
        for gun in (0,1):
            for armor in (0,1):
                for hp in (18,45):case(profile=profile,battle=battle,gun=gun,armor=armor,hp=hp)
for alternate in (0,1):
    for reuse in (0,1):
        for alt_items in (0,1):case(alternate=alternate,reuse=reuse,alt_items=alt_items,gun=0,armor=0)
for kind in (0,1,7,8,9,10,31,32,33,255):case(kind=kind,gun=0,armor=0)
for capacity in (0,1,31,32,33,49,50,255):
    for bonus in (0,1,2):case(capacity=capacity,bonus=bonus)
for alt_count in (0,1,31,32,33,63,64,65,127,128):case(alternate=1,alt_count=alt_count,reuse=1)
for flags in (0,1,2,3,255):
    for battle in (0,1):case(flags=flags,battle=battle)
case(1,profile=1,battle=1,alternate=1)
for index in (-33,-32,-1,0,1,31,32,33,63,64,127):case(2,index=index,alternate=1,reuse=1)
for first,second in ((0,1),(1,0),(0,3),(3,0),(0,0),(2,2),(1,7)):
    for reuse in (0,1):case(3,first=first,second=second,alternate=1,reuse=reuse)
for capacity in (0,1,10,49,50,255):case(4,capacity=capacity,alternate=1,reuse=1)
for index in (-1,0,1,2,49,255):
    for reuse in (0,1):case(5,index=index,alternate=1,reuse=reuse)
case(5,index=-1,equipped=(-1,-1))
for value in (-2147483648,-100,-1,0,9,10,99,100,2147483647):case(6,value=value)
for absent in (0,1):
    for x,y in ((0,0),(12,-16),(-2147483648,2147483647)):case(7,absent=absent,x=x,y=y)
case(8);case(8,absent=1)
for bank in (0,1):case(9,bank=bank)
for index in (-1,0,1,2,3,4,5,6,7,9,10):
    for dim in (0,1):case(10,index=index,dim=dim)
for capacity in (1,10,31,49,50):
    for saved in (0,1,0x305):case(13,capacity=capacity,saved=saved)
for entry in (11,12,14,15):
    for bank in (0,1):
        for top,scroll in ((0,0),(5,0),(16,-8),(17,0),(17,8)):
            case(entry,capacity=49,bank=bank,top=top,scroll=scroll,frame=8)
for frame in (0,8,31):
    for focused in (0,1):case(14,capacity=50,frame=frame,focused=focused)
case(14,capacity=50,absent=1)
for event in (0x10000,64):
    for selected,saved in ((0,1),(1,0),(0,0),(3,1),(7,0)):
        case(16,event=event,selected=selected,saved_cursor=saved)
for profile in (0,1,2,0xFFFFFFFF):case(17,profile=profile)
for entry in (6,9,10,11,12,14,15):case(entry,capacity=50,offset=0x3FE0)

def fixture(exe,c):
    r,s,_=help_fixture(exe,dict(c,entry=9,group=0,index=0))
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
    owner=find(2,0)
    sw(0x9D018,c.get('bonus',0));sb(0xC0E0C,c.get('capacity',10))
    sh(0xC0E06,45);sh(0xC0E08,c.get('hp',18))
    sb(0xC0E20,c.get('equipped',(0,1))[0]);sb(0xC0E22,c.get('equipped',(0,1))[1])
    put(0xC0E48,bytes(100));put(0x15F800,bytes(256))
    sw(0x9D04C,0x8015F800 if c.get('alternate') else 0);sw(0x9D054,c.get('alt_count',10))
    sw(0x9CF0C,c.get('profile',0));sw(0x9CF98,c.get('saved',0));sw(0x9CEFC,c.get('dim',0))
    sw(0x9D164,124);sw(0x9D168,16)
    records=((c.get('kind',1),1,0),(9,2,0),(10,6,0),(10,12,4),
             (10,13,12),(10,14,2),(10,10,0),(10,15,14))
    for i,(kind,id,subtype) in enumerate(records):
        p=0xC0EAC+i*32;put(p,bytes(32));sb(p,2);sb(p+4,id);sb(p+5,c.get('flags',3));sb(p+6,kind)
        sb(p+9,10);sh(p+10,3);sb(p+14,subtype)
    for i in range(50):
        record=i%10
        value=256+record if record<8 else (0 if record==8 else -1)
        if record==0 and not c.get('gun',1):value=256 if 'kind' in c else 0
        if record==1 and not c.get('armor',1):value=0
        sh(0xC0E48+i*2,value)
    for i in range(128):sh(0x15F800+i*2,256+i%8 if c.get('alt_items',1) else 0)
    execute(r,0x80052E30,(c.get('reuse',0),))
    for a in (0x9D05C,0x9D060,0xA1F80,0xA1F84,0xA1F88,0xA1F8C,0xA1F90):sw(a,0xA5A51234)
    node=owner;window=find(1,18);footer=0;scrollbar=0
    if c['entry'] in (8,11,12,13,14,15,16):
        for id in (45,24,18):execute(r,0x80062F3C,(id,))
        if c['entry']!=13:
            execute(r,0x80044174,(owner,))
            node=find(2,1);window=find(1,1);footer=find(1,27);scrollbar=word(node+128)
            sw(node+68,c.get('selected',0)&1);sw(node+72,c.get('selected',0)//2)
            sw(node+92,c.get('top',0));sw(node+96,c.get('scroll',0))
            if scrollbar:execute(r,0x80065260,(scrollbar,))
            if c['entry']==14:
                assert scrollbar
                sw(0x9D15C,scrollbar if c.get('focused',1) else node)
                if c.get('absent'):sw(scrollbar+52,0)
            if c['entry']==16:
                selected=c.get('saved_cursor',0);sw(node+76,selected&1);sw(node+80,selected//2)
                sw(0x9D0E8,0);sw(0x9D26C,0)
    sw(0x9D100,0x80160000+c.get('offset',0))
    index=c.get('index',0)
    args=((),(),(index,),(1,c.get('first',0),1,c.get('second',1)),(),(index,),
          (c.get('value',10),),(0 if c.get('absent') else window,c.get('x',0),c.get('y',0)),
          (0 if c.get('absent') else window,),(),(index,),(node,),(footer,),(owner,),
          (scrollbar,),(),(node,c.get('event',0)),())[c['entry']]
    for a,n in RANGES:s[a:a+n]=r[a:a+n]
    return r,s,tuple(v&0xFFFFFFFF for v in args)

def main():
    exe=(ROOT/'build/disc1.candidate.exe').read_bytes()
    assert hashlib.sha1(exe).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
    _,s,_=fixture(exe,CASES[0]);base=words(s);common=[(i*4,v) for i,v in enumerate(base) if v]
    patches=[];cases=[]
    for k,c in enumerate(CASES):
        r,s,args=fixture(exe,c);first=len(patches)
        patches.extend((i*4,v) for i,(v,b) in enumerate(zip(words(s),base)) if v!=b)
        regs=execute(r,ENTRIES[c['entry']],args,instruction_budget=900000)
        result=regs[2] if c['entry'] in RESULT_ENTRIES else 0;h=fingerprint(r)
        cases.append((c['entry'],first,len(patches),args,result,h,c.get('frame',0)))
        print(k,c,hex(result),hex(h),flush=True)
        if '--dump' in sys.argv:(ROOT/f'pc_port/build/inventory-items-oracle-{k}.bin').write_bytes(r)
    out=['/* Generated by pe_inventory_items_oracle.py --write-header. */']
    for name,rows in (('ranges',RANGES),('common',common),('patches',patches)):
        out.append(f'static const uint32_t INV6_inventory_items_{name}[][2]={{')
        out.extend(f'    {{0x{a:X}u,0x{b:X}u}},' for a,b in rows);out.append('};')
    out.append('static const struct { unsigned entry,first,end; uint32_t args[4],result; uint64_t hash; uint32_t frame; } INV6_inventory_items_cases[]={')
    for e,a,b,args,result,h,frame in cases:
        params=','.join(f'0x{x:08X}u' for x in args) or '0'
        out.append(f'    {{{e},{a},{b},{{{params}}},0x{result:08X}u,UINT64_C(0x{h:016X}),{frame}u}},')
    out.append('};')
    if '--write-header' in sys.argv:(ROOT/'pc_port/tests/retail_inventory_items_cases.h').write_text('\n'.join(out)+'\n')
    print(f'PASS: {len(cases)} original item availability, transfer and drawing cases')

if __name__=='__main__':main()
