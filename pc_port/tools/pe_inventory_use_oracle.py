#!/usr/bin/env python3
"""Original ordinary item use and complete action-menu input graphs.

Tool/rename subtypes have separate menus and are deliberately not substituted
here. Field medicine, battle item commands, Move, Discard and Reload run in full.
"""
import hashlib
import struct
import sys
from pe_battle_hud_oracle import ROOT,execute
from pe_inventory_actions_oracle import fixture as actions_fixture,RANGES as ACTION_RANGES
from pe_inventory_commit_oracle import fixture as commit_fixture
from pe_scripted_exit_oracle import words

RANGES=ACTION_RANGES+((0xB0CB0,0x130),(0xBCF88,0x28),(0x9EC40,56))
ENTRIES=(0x800516B4,0x80057834,0x80044B0C,0x8005E30C,0x80062FEC)
RESULT_ENTRIES={2}
CASES=[]
def case(entry=2,**kw):CASES.append(dict(entry=entry,**kw))
for item in range(6,18):
    for hp in (1,18,45):case(0,item=item,hp=hp)
for equipped in (-1,0,1):
    for bank in (0,1):case(0,item=6,equipped=equipped,bank=bank)
case(0,item=6,no_weapon=1)
for item in range(6,18):
    for battle in (0,1):case(1,item=item,battle=battle)
for subtype in (0,3,7,8,9,10,11,15,255):case(1,subtype=subtype,item=256)
for item in (-1,0,6,256,384,511,520):case(1,item=item,battle=1)
for index in (-1,0,1,9,10):case(1,index=index,battle=1)
for history in (0,108,144):
    for pending in (0,1):case(1,battle=1,history=history,pending=pending)
for profile in (0,1,2):
    for battle in (0,1):
        for enabled in (0,1):case(profile=profile,battle=battle,enabled=enabled)
for profile in (0,1,2):
    for bank in (0,1):case(action=1,profile=profile,bank=bank)
for kind in (1,8,9,10,19):
    for equipped in (-1,0):
        for flags in (3,67):case(action=2,index=0,item=256,kind=kind,equipped=equipped,flags=flags)
for capacity in (8,10):
    for armor_reserve in (0,1,4):case(action=2,index=1,item=257,kind=9,capacity=capacity,armor_reserve=armor_reserve)
for bank in (0,1):
    for matches in (0,1,3):
        for unavailable in (0,1):case(action=3,index=0,item=256,kind=1,bank=bank,matches=matches,unavailable=unavailable)
for event in (0,2,32,64,0x10040,0x1000,0x10000):
    for action in (0,1,2,3):case(event=event,action=action,item=6 if action==0 else 256,index=2 if action==0 else 0)
for kind in (9,10):
    for subtype in (4,6,12,14):case(profile=1,kind=kind,subtype=subtype,item=256,enabled=0 if kind!=10 else 1)
for event_type,event in ((4,32),(1,64),(1,0x4000)):
    for action in (0,1,2,3):case(3,event_type=event_type,event=event,action=action,item=6 if action==0 else 256,index=2 if action==0 else 0)
for action in (0,1,2,3):
    for bank in (0,1):case(4,action=action,item=6 if action==0 else 256,index=2 if action==0 else 0,bank=bank)

def fixture(exe,c):
    r,s,args=actions_fixture(exe,dict(c,entry=10,index=c.get('index',2),alternate=c.get('bank',0),
                                   reuse=c.get('bank',0),equipped=(c.get('equipped',0),1)))
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
    er,_,_=commit_fixture(exe,dict(entry=0,command=5,battle=c.get('battle',0)))
    for a,n in ((0x15C000,0x1400),(0xB0CB0,0x130),(0xBCF88,0x28),(0x150000,0x3000),
                (0xA8038,8),(0xC0E28,14)):
        put(a,er[a:a+n])
    put(0x92234,exe[0x92234-0x10000+0x800:0x92258-0x10000+0x800])
    sw(0xA8034,0x154000-0xA8028)
    for i in range(256):
        p=0x154000+i*32;put(p,bytes(32));sb(p+4,i+1);sb(p+5,3);sb(p+6,10);sb(p+14,1)
    item=c.get('item',6);index=c.get('index',2);bank=c.get('bank',0)
    selected=0x15F800 if bank else 0xC0E48
    sh(selected+index*2,item)
    p=0xC0EAC+(item-256)*32 if 256<=item<384 else 0x154000+(item-1)*32
    if 1<=item<384:
        if 'kind' in c:sb(p+6,c['kind'])
        if 'subtype' in c:sb(p+14,c['subtype'])
        if 'flags' in c:sb(p+5,c['flags'])
    sb(0xC0E20,c.get('equipped',0));sb(0xC0E22,0 if c.get('kind')==9 and index==0 and c.get('equipped')==0 else 1)
    sh(0x15C10C,c.get('hp',18));sh(0x15C11C,45)
    sw(0x15C108,300<<16);sw(0x15C128,400<<16);sw(0x15C14C,0x1055)
    sw(0x15D10C,0xA5A00003)
    if c.get('no_weapon'):sw(0x15C168,0)
    sw(0x9D028,c.get('battle',0));sw(0x9D1A0,c.get('battle',0)*2);sw(0x9CF0C,c.get('profile',0))
    sw(0x9D014,0x800A1AA0+c.get('history',0));sw(0x9D010,c.get('pending',0))
    if 'armor_reserve' in c:
        sb(0xC0ECC+20,1);sb(0xC0ECC+21,8+c['armor_reserve'])
    if c.get('action')==3:
        put(selected,bytes(20));sh(selected,256)
        for i in range(c.get('matches',1)):sh(selected+(i+1)*2,266)
        if bank:put(0xC0E48,bytes(20))
    execute(r,0x80052E30,(bank,));execute(r,0x80055760)
    item_list=find(2,1);sw(item_list+68,index&1);sw(item_list+72,index//2)
    window=0
    if c['entry']>=2:
        execute(r,0x80044924,(item_list,bank,index))
        group=word(0x9CDA8);id=3 if group==2 else 2
        window=find(1,id);node=find(2,id)
        # Select an actual action-table entry. Use explicit groups when the
        # fixture deliberately asks for a disabled or atypical action.
        action=c.get('action',0);group=1 if action==3 else 0
        sw(0x9CDA8,group);sw(node+68,0);sw(node+72,0 if action==3 else action)
        if not c.get('enabled',1):sw(0x9D05C,0);sw(0x9D060,0);sw(0xA1F84,0)
        sw(0x9CF08,c.get('unavailable',0))
        if c['entry']==3:
            q=0x80140500;sw(q,0);sw(q+4,c.get('event_type',4));sw(q+8,c.get('event',32))
            sw(0x9D0E0,q);sw(0x9D0E4,q);sw(0x9D0EC,1);sw(0x9D0E8,0)
        if c['entry']==4:execute(r,0x80044B0C,(window,0x10000))
    sw(0x9D100,0x80160000)
    args=((item,),(index,),(window,c.get('event',0x10000)),(),())[c['entry']]
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
        regs=execute(r,ENTRIES[c['entry']],args,instruction_budget=900000)
        result=regs[2] if c['entry'] in RESULT_ENTRIES else 0;h=fingerprint(r)
        cases.append((c['entry'],first,len(patches),args,result,h))
        print(k,c,hex(result),hex(h),flush=True)
        if '--dump' in sys.argv:(ROOT/f'pc_port/build/inventory-use-oracle-{k}.bin').write_bytes(r)
    out=['/* Generated by pe_inventory_use_oracle.py --write-header. */']
    for name,rows in (('ranges',RANGES),('common',common),('patches',patches)):
        out.append(f'static const uint32_t INV10_inventory_use_{name}[][2]={{')
        out.extend(f'    {{0x{a:X}u,0x{b:X}u}},' for a,b in rows);out.append('};')
    out.append('static const struct { unsigned entry,first,end; uint32_t args[2],result; uint64_t hash; } INV10_inventory_use_cases[]={')
    for e,a,b,args,result,h in cases:
        params=','.join(f'0x{x:08X}u' for x in args) or '0'
        out.append(f'    {{{e},{a},{b},{{{params}}},0x{result:08X}u,UINT64_C(0x{h:016X})}},')
    out.append('};')
    if '--write-header' in sys.argv:(ROOT/'pc_port/tests/retail_inventory_use_cases.h').write_text('\n'.join(out)+'\n')
    print(f'PASS: {len(cases)} original item use and action input cases')

if __name__=='__main__':main()
