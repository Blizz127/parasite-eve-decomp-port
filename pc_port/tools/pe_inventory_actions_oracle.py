#!/usr/bin/env python3
"""Original item-window input, action constructors and compatibility helpers.

The selected action's subsequent 44B0C callback is a separate test group.
All call graphs exercised here execute their original instructions in full.
"""
import hashlib
import struct
import sys
from pe_battle_hud_oracle import ROOT,execute
from pe_inventory_items_oracle import fixture as items_fixture,RANGES as ITEM_RANGES
from pe_scripted_exit_oracle import words

RANGES=((0x9CDA8,4),)+ITEM_RANGES
ENTRIES=(0x800210D4,0x80052558,0x80055724,0x80055FB4,0x800562A4,0x8005600C,
         0x80056C40,0x80057D18,0x8005833C,0x80057654,0x80044924,0x800451D0,0x80044444)
RESULT_ENTRIES={0,1,4,7,8,9,12}
CASES=[]
def case(entry=12,**kw):CASES.append(dict(entry=entry,**kw))
for entry in (0,1):
    for battle in (0,1):
        for unavailable in (0,1):
            for status in (0,0x10000):case(entry,battle=battle,unavailable=unavailable,status=status)
for count in (-1,0,1,2,4):case(2,bit_words=count,alternate=1,reuse=1)
for index in (-33,-32,-1,0,1,31,32,33,63,64,127):case(3,index=index,alternate=1,reuse=1)
for kind in (0,1,4,5,6,7,8,9,10,16,18,19,20,21,22,255):
    for reuse in (0,1):
        for alternate in (0,1):case(4,kind=kind,index=0,reuse=reuse,alternate=alternate)
for index in (-1,0,1,2,7,9,10):case(4,index=index,alternate=1,alt_count=128)
for capacity in (0,1,31,32,33,49,50):case(4,capacity=capacity,index=0)
for bank in (0,1,2):
    for selected in (-1,0,1,2,31):
        for bits in (0,1,3):case(5,selected_bank=bank,selected=selected,bits=bits,alternate=1,reuse=1)
case(5,selected_bank=0,selected=0,bits=1,alias=1)
for first_bank in (0,1):
    for second_bank in (0,1):
        for first,second in ((-1,-1),(0,1),(1,0),(2,2),(9,10)):
            case(6,first_bank=first_bank,second_bank=second_bank,first=first,second=second,alternate=1)
for alias in (0,1,2):case(6,record_alias=alias,first=0,second=1,alternate=1)
for index in (-1,0,1,9,10):case(7,index=index,alternate=1,reuse=1)
for kind in (0,1,5,7,8,9,16,17,18,19,255):
    for full in (0,1):case(8,kind=kind,index=0,full=full)
for index in (-1,0,1,3,4):case(8,index=index)
for kind in (1,8,9,10):
    for flags in (0,64):
        for equipped in ((0,1),(-1,-1)):
            case(9,kind=kind,flags=flags,equipped=equipped,index=0)
case(9,kind=8,duplicate_baton=1,index=0);case(9,index=9)
for kind in (1,8,9,10,12,13,14,15,19):
    for profile in (0,1):
        for battle in (0,1):case(10,kind=kind,profile=profile,battle=battle,index=0)
for unavailable in (0,1):
    for status in (0,0x10000):case(10,kind=1,index=0,battle=1,unavailable=unavailable,status=status)
for kind in (1,19,21,22):
    for first_bank in (0,1):case(11,kind=kind,first_bank=first_bank,second_bank=1-first_bank,alternate=1)
case(11,first=-1,second=-1)
for event in (0,32,64,0x10000):
    for index in (0,1,2,8):case(event=event,index=index)
for group in (13,14,51,52):
    for event in (64,0x10000):case(group=group,event=event,alternate=1,index=2)
for dim in (0,1):
    for index in (0,2,8):case(dim=dim,index=index,event=0x10000)
for pending in (0,1):
    for event in (0,64,0x10000):
        for usable in (0,1):case(pending=pending,event=event,usable=usable,index=2)
for kind in (1,9,10):case(pending=1,event=0x10000,kind=kind,index=0,equipped=(-1,-1))
for event in (64,0x10000):
    for first in (0,2):case(pair=1,first=first,index=1,event=event)
case(event=64,saved_cursor=1)
for entry in (10,11,12):case(entry,kind=19,event=0x10000,alternate=1,reuse=1)

def fixture(exe,c):
    r,s,_=items_fixture(exe,dict(c,entry=11,saved=0,selected=0))
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
    node=find(2,1);window=find(1,1)
    sb(0xC0EAC+6,c.get('kind',1));sb(0xC0EAC+14,c.get('subtype',1))
    # Supply ammunition groups 19/20/21 plus gun categories 1/5/6.
    for i,kind in enumerate((19,20,21,1,5,6),10):
        p=0xC0EAC+i*32;put(p,bytes(32));sb(p,2);sb(p+4,i);sb(p+5,3);sb(p+6,kind)
        sb(p+9,100);sh(p+10,7)
    for i in range(3,8):sh(0xC0E48+i*2,266+i-3);sh(0x15F800+i*2,266+(i-2)%6)
    if c.get('duplicate_baton'):sb(0xC0ECC+6,8)
    if c.get('full'):
        for i in range(50):sh(0xC0E48+i*2,256)
    else:sh(0xC0E48+16,0)
    sw(0x9D078,4)
    for i,id in enumerate((256,257,0,255)):sh(0xA1FD4+i*2,id)
    for i in range(3):p=0xA1E64+i*32;sb(p+9,100);sh(p+10,12)
    for a,offset in ((0x9CF88,'first_bank'),(0x9CF8C,'first'),(0x9CF90,'second_bank'),(0x9CF94,'second')):
        sw(a,c.get(offset,0 if offset in ('first_bank','first') else 1))
    if not c.get('pair') and c['entry']!=11:sw(0x9CF8C,-1)
    sw(0x9CF00,c.get('pending',0));sw(0x9CF5C,0);sw(0x9D010,0)
    sw(0x9CEFC,c.get('dim',0));sb(0x9CE3C,c.get('unavailable',0));sw(0x15C14C,c.get('status',0))
    sw(0x9D028,c.get('battle',0));sw(0x9D1A0,c.get('battle',0)*2)
    execute(r,0x80052E30,(c.get('reuse',0),))
    execute(r,0x80055760)
    if c['entry'] in (2,3,5):
        for a in (0x9D05C,0x9D060,0xA1F80,0xA1F84,0xA1F88,0xA1F8C,0xA1F90):sw(a,c.get('bits',0xA5A51234))
    if c['entry']==2:sw(0x9D064,c.get('bit_words',2))
    if not c.get('usable',1):sw(0x9D05C,0);sw(0x9D060,0)
    sw(0x15FA00,c.get('selected_bank',0));sw(0x15FA04,c.get('selected',0))
    if c['entry']==6 and c.get('record_alias'):
        sw(0xA8034,0xA1F94-0xA8028);sw(0xA8038,0xA1FD4-0xA8028)
        sh(0xC0E48,1);sh(0xC0E4A,2 if c['record_alias']==1 else 1)
        for i in range(64):sb(0xA1F94+i,(i*7+3)&255)
    if c['entry']==12:
        group=c.get('group',1)
        if group in (13,14,51,52):
            sw(window+36,group);sw(node+36,group)
            owner_id=12 if group in (13,14) else 50
            parent=execute(r,0x80062D2C,(owner_id,0,0,0))[2]
            other=execute(r,0x8006322C,(owner_id,parent,parent))[2]
            sw(other+68,1);sw(0x9D15C,node)
        index=c.get('index',0);sw(node+68,index&1);sw(node+72,index//2)
        if c.get('saved_cursor'):sw(node+76,0);sw(node+80,0)
    args=((),(),(),(c.get('index',0),),(c.get('index',0),),
          (0x8015FA00,0x8015FA00 if c.get('alias') else 0x8015FA04),
          (c.get('first_bank',0),c.get('first',0),c.get('second_bank',0),c.get('second',1)),
          (c.get('index',0),),(c.get('index',0),),(c.get('index',0),),
          (node,c.get('reuse',0),c.get('index',0)),(node,),(window,c.get('event',0)))[c['entry']]
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
        if '--dump' in sys.argv:(ROOT/f'pc_port/build/inventory-actions-oracle-{k}.bin').write_bytes(r)
    out=['/* Generated by pe_inventory_actions_oracle.py --write-header. */']
    for name,rows in (('ranges',RANGES),('common',common),('patches',patches)):
        out.append(f'static const uint32_t INV7_inventory_actions_{name}[][2]={{')
        out.extend(f'    {{0x{a:X}u,0x{b:X}u}},' for a,b in rows);out.append('};')
    out.append('static const struct { unsigned entry,first,end; uint32_t args[4],result; uint64_t hash; } INV7_inventory_actions_cases[]={')
    for e,a,b,args,result,h in cases:
        params=','.join(f'0x{x:08X}u' for x in args) or '0'
        out.append(f'    {{{e},{a},{b},{{{params}}},0x{result:08X}u,UINT64_C(0x{h:016X})}},')
    out.append('};')
    if '--write-header' in sys.argv:(ROOT/'pc_port/tests/retail_inventory_actions_cases.h').write_text('\n'.join(out)+'\n')
    print(f'PASS: {len(cases)} original inventory input, action setup and compatibility cases')

if __name__=='__main__':main()
