#!/usr/bin/env python3
"""Complete original Arrange Items menus and sorting; no callee substitutions.

The checksum-pinned executable supplies the actual quicksort, comparisons,
menu callbacks and graphics. Ranges include carried/stored inventory, equipment
indices, availability, menu trees and both GPU banks; CPU stack is excluded.
"""
import hashlib
import random
import struct
import sys
from pe_battle_hud_oracle import ROOT,execute
from pe_inventory_items_oracle import fixture as items_fixture,RANGES
from pe_scripted_exit_oracle import words

ENTRIES=(0x80046DFC,0x80046EAC,0x80047040,0x800471BC,0x800471E4,
         0x8004732C,0x80047354,0x800473E4,0x800474A8,0x800474D0,
         0x80050280,0x80050308,0x8005AFFC,0x8005B124,0x8005B248,
         0x8005B3A4,0x8005B500,0x8005B71C,0x8005B7D0,0x800723A4,
         0x800724F4,0x80043DA4,0x80062FEC,0x80063E0C)
RESULT_ENTRIES={4,6,9,12,13,21,23}
CASES=[]
def case(entry,**kw):CASES.append(dict(entry=entry,**kw))
for mode in (0,1,2):
    case(0,mode=mode)
    for arranged in (0,1):
        for saved in (0,1,0x305,0x80000101):case(1,mode=mode,arranged=arranged,saved=saved)
    for index in range(2 if mode==1 else 3):
        for event in (0,64,0x10000,0x10040):case(4,mode=mode,index=index,event=event)
    for bank in (0,1):
        for page in (58,59,60):case(22,mode=mode,page=page,bank=bank)
    for entry in (2,3,5,7,8,10,11):
        for index in (0,1):case(entry,mode=mode,index=index)
    for entry in (6,9):
        for index in range(2 if entry==6 else 3):
            for event in (0,64,0x10000,0x10040):case(entry,mode=mode,index=index,event=event)
for event in (0,64,0x10000):case(21,event=event)
for page in (58,59,60):
    for event in (1,2,4,8,64,0x10000):case(23,page=page,event=event)
for entry in (16,17,18):
    for kind in range(3 if entry==16 else 2):
        for selection in range(3):
            for seed in (1,9):case(entry,kind=kind,selection=selection,seed=seed)
for entry in (14,15,16,19):
    for count in (0,1,2,3,10,49,50):
        for profile in ('empty','mixed','ties','reverse'):
            case(entry,count=count,capacity=count,profile=profile,kind=2,selection=1)
for entry in (12,13):
    for lookup in (0x800532B4,0x8005332C):
        for first,second in ((0,0),(0,1),(1,0),(0,2),(2,0),(2,2),(4,5),(5,4)):
            for stat in (0,1,2,3):case(entry,lookup=lookup,first=first,second=second,stat=stat)
for size in (0,1,2,3,8):
    for delta in (0,1,2,8):case(20,size=size,delta=delta)
for entry in (3,5,8,22):case(entry,offset=0x3FE0,page=60)

for kind in range(3):
    for selection in range(3):
        for weapon,armor in ((-1,-1),(0,1),(1,2),(49,48)):
            case(16,kind=kind,selection=selection,weapon=weapon,armor=armor,capacity=50)

def fixture(exe,c):
    r,s,_=items_fixture(exe,dict(entry=0,capacity=c.get('capacity',10),bank=c.get('bank',0)))
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
    # Restore all original sort/label tables, rather than synthesizing ranks.
    put(0x922B8,exe[0x922B8-0x10000+0x800:0x92478-0x10000+0x800])
    rng=random.Random(c.get('seed',1));profile=c.get('profile','mixed')
    for i in range(16):
        p=0xC0EAC+i*32;put(p,bytes(32));sb(p,2);sb(p+4,(i%4 if profile=='ties' else i));sb(p+5,3)
        sb(p+6,(1,9,10,5,8,9,19,12)[i%8]);sh(p+10,3)
        for off in (7,8,9):sb(p+off,rng.randrange(256))
        for off in (14,16,18):sh(p+off,0 if profile=='ties' else rng.choice((-32768,-1,0,13,32767)))
    for base,count in ((0xC0E48,50),(0xC1EB8,100),(0xC1F80,82)):
        for i in range(count):
            value=0 if profile=='empty' or i%5==0 else 256+rng.randrange(16)
            if profile=='reverse':value=256+(count-i)%16
            if profile=='ties':value=256+(i%2)*8
            sh(base+i*2,value)
    sb(0xC0E20,c.get('weapon',1));sb(0xC0E22,c.get('armor',-1))
    sw(0x9D0A4,c.get('weapon_stat',0));sw(0x9D0A8,c.get('armor_stat',1));sw(0x9D0A0,c.get('stat',0))
    sw(0x9D0B4,c.get('lookup',0x800532B4));sw(0x9D0B8,0x80092440);sw(0x9D0BC,0x80092458)
    sw(0x9D0AC,0x800C0E48);sw(0x9D0B0,c.get('count',10))
    sw(0x9CF98,c.get('saved',0));sw(0x9CF0C,c.get('mode',0));sw(0x9CFB8,c.get('mode',0))
    sw(0x9CF18,c.get('kind',1));sw(0x9D04C,0);sw(0x9D054,0)
    for i in range(16):sb(0x15FA00+i,i*11)
    sh(0x15FA20,c.get('first',0));sh(0x15FA22,c.get('second',1))
    # Sorting comparators accept item IDs, except the original alternate
    # provider branch which accepts list indices (including a real index 0).
    if c.get('lookup',0x800532B4)==0x800532B4:
        sh(0x15FA20,0 if c.get('first',0)==0 else 256+c['first'])
        sh(0x15FA22,0 if c.get('second',1)==0 else 256+c.get('second',1))
    execute(r,0x80062F9C);sw(0x9CEF0,0x1EF);execute(r,0x800438EC)
    main=find(2,0);window=find(1,0);node=main
    mode=c.get('mode',0);entry=c['entry'];index=c.get('index',0);event=c.get('event',0x10000)
    sw(main+68,0);sw(main+72,5)  # command 6, with field-only Escape masked out
    if mode:
        stored=execute(r,0x80062D2C,(51,main,0,0))[2]
        execute(r,0x8006322C,(51,stored,stored))
    if entry not in (0,12,13,14,15,16,17,18,19,20,21):
        for id in (45,24,18):execute(r,0x80062F3C,(id,))
        execute(r,0x80046DFC,(main,int(mode!=0)))
        window=find(1,59 if mode else 58);node=find(2,59 if mode else 58)
        page=c.get('page',58)
        if entry in (5,6,10) or page==59:
            # mode 1 already owns group 59; recreate it with the secondary
            # callback for direct alternate-branch coverage.
            w=execute(r,0x80062D2C,(59,node,0,0))[2]
            n=execute(r,0x8006322C,(59,w,w))[2]
            sw(w+44,0x80047354);sw(n+48,0x8004732C);execute(r,0x80062CB8,(n,))
            window,node=w,n
        if entry in (8,9,11) or page==60:
            execute(r,0x800473E4,(node,c.get('kind',0)))
            window=find(1,60);node=find(2,60)
        sw(node+68,0);sw(node+72,index)
    assert word(0x9D108)==c.get('bank',0) and word(0x9CDDC)==c.get('bank',0)
    sw(0x9D100,0x80160000+c.get('offset',0));sw(0x9D104,0x80160000)
    args=((main,int(mode!=0)),(c.get('arranged',1),),(index,),(node,),(window,event),
          (node,),(window,event),(node,index),(node,),(window,event),(index,),(index,),
          (0x8015FA20,0x8015FA22),(0x8015FA20,0x8015FA22),(),(),
          (c.get('kind',2),c.get('selection',0)),(c.get('selection',0),),
          (c.get('kind',1),c.get('selection',0)),(0x800C0E48,c.get('count',10),2,0x8005AFFC),
          (0x8015FA00,0x8015FA00+c.get('delta',0),c.get('size',2)),(window,event),(),(node,event))[entry]
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
        regs=execute(r,ENTRIES[c['entry']],args,instruction_budget=2000000)
        result=regs[2] if c['entry'] in RESULT_ENTRIES else 0;h=fingerprint(r)
        cases.append((c['entry'],first,len(patches),args,result,h))
        print(k,c,hex(result),hex(h),flush=True)
        if '--dump' in sys.argv:(ROOT/f'pc_port/build/inventory-sort-oracle-{k}.bin').write_bytes(r)
    out=['/* Generated by pe_inventory_sort_oracle.py --write-header. */']
    for name,rows in (('ranges',RANGES),('common',common),('patches',patches)):
        out.append(f'static const uint32_t DAY1_inventory_sort_{name}[][2]={{')
        out.extend(f'    {{0x{a:X}u,0x{b:X}u}},' for a,b in rows);out.append('};')
    out.append('static const struct { unsigned entry,first,end; uint32_t args[4],result; uint64_t hash; } DAY1_inventory_sort_cases[]={')
    for e,a,b,args,result,h in cases:
        params=','.join(f'0x{x:08X}u' for x in args) or '0'
        out.append(f'    {{{e},{a},{b},{{{params}}},0x{result:08X}u,UINT64_C(0x{h:016X})}},')
    out.append('};')
    if '--write-header' in sys.argv:(ROOT/'pc_port/tests/retail_inventory_sort_cases.h').write_text('\n'.join(out)+'\n')
    print(f'PASS: {len(cases)} original inventory sorting/menu cases')

if __name__=='__main__':main()
