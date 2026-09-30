#!/usr/bin/env python3
"""Original item-action drawing, notices and discard confirmations.

Tests full original draw trees and queued Yes/No input. Notice callbacks are
covered for zero, 62F9C and 5C488; unrelated dialog callbacks are not replaced.
"""
import hashlib
import struct
import sys
from pe_battle_hud_oracle import ROOT,execute
from pe_inventory_actions_oracle import fixture as actions_fixture,RANGES,fingerprint
from pe_scripted_exit_oracle import words

ENTRIES=(0x8005F354,0x8005F594,0x80064C30,0x80064C54,0x80062A7C,0x80053068,
         0x80052BCC,0x80052C08,0x80050878,0x8004F910,0x800509A8,0x8004F950,
         0x8004FFA8,0x8004CDD4,0x8004CC50,0x8004D030,0x80044E14,0x80044F8C,
         0x80062CE4,0x80058C4C,0x80045110,0x80044E98,0x80062FEC,0x80062FEC,
         0x80062FEC,0x800631AC,0x8004D024,0x8005E30C)
RESULT_ENTRIES={5,15,19,21}
CASES=[]
def case(entry=22,**kw):CASES.append(dict(entry=entry,**kw))
for entry in (0,1,2):
    for width in (-2147483648,0,1,20,120,320):
        for text in (0,1,2):case(entry,width=width,text=text)
for entry in (3,4):
    for id in (0,1,5,6,8,29):case(entry,id=id)
for id in (-32768,-1,0,1,255,256,383,384,511,512,520,521):case(5,id=id)
for kind in (1,9):case(5,id=256,kind=kind,renamed=1)
for entry in (6,7):
    for text in (0,1,2):
        for alias in (0,1):case(entry,text=text,alias=alias)
for group in (0,1,2):
    for index in (0,1,2):
        for usable in (0,1):case(8,group=group,index=index,usable=usable)
for kind in (1,8,9,10,19):
    for profile in (0,1):
        for entry in (9,22):case(entry,kind=kind,profile=profile,item_index=0)
for entry in (10,11,12,13,16):
    for bank in (0,1):
        for index in (0,1):case(entry,bank=bank,index=index,extra=29)
for entry in (14,17):
    for length in (1,15,24):
        for nested in (0,1):case(entry,length=length,nested=nested)
for event in (0,32,64,0x10000):
    for callback in (0,0x80062F9C,0x8005C488):case(15,event=event,callback=callback)
for saved in (0,1,2):case(18,saved_focus=saved)
for mask in (0,1,2,0x3803FE,0x80000000,0xFFFFFFFF):
    for capacity in (0,10,49,50):case(19,mask=mask,capacity=capacity)
for confirmed in (0,1):
    for item_index in (0,1,2):case(20,confirmed=confirmed,item_index=item_index,equipped=(-1,-1))
for event in (0,32,64,0x10000):
    for index in (-1,0,1,2):
        for callback in (0,0x80045110):case(21,event=event,index=index,callback=callback)
for entry in (22,23,24):
    for bank in (0,1):
        for frame in (0,8,31):case(entry,bank=bank,frame=frame)
case(25);case(25,absent=1)
for callback in (0,0x8005C488,0xFFFFFFFF):case(26,callback=callback)
for event_type,event,index in ((4,32,0),(4,32,1),(1,64,0),(1,64,1),(1,0x2000,0)):
    case(27,event_type=event_type,event=event,index=index)
for entry in (0,8,9,11,12,13,16,22,23,24):case(entry,offset=0x3FE0)

def fixture(exe,c):
    r,s,args=actions_fixture(exe,dict(c,entry=10,index=c.get('item_index',2)))
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
    put(0x92234,exe[0x92234-0x10000+0x800:0x92258-0x10000+0x800])
    execute(r,0x80044924,args)
    item_list=find(2,1);group=word(0x9CDA8);id=3 if group==2 else 2
    action_list=find(2,id);action_window=find(1,id);window=action_window;node=action_list
    sw(0x9D108,c.get('bank',0));sw(0x9CDDC,c.get('bank',0));sw(0x956AC,c.get('frame',0))
    sw(0x9D138,c.get('width',140));sw(0x9D164,c.get('width',124));sw(0x9D0D8,0)
    sample=(bytes((255,)),bytes((1,15,32,48,255)),bytes((251,16,32,252,16,255)))[c.get('text',1)]
    put(0x15FC00,sample);put(0x15FB80,bytes((48,16,255)))
    for id in (4,29):put(0x140A00+id*0x80,bytes((32,))*c.get('length',2)+bytes((255,)))
    if c['entry']==5:
        sh(word(0x9D048),c.get('id',256))
        if c.get('renamed'):sb(0xC0EB1,r[0xC0EB1]|16)
        put(0xC20A4,bytes((16,32,255)));put(0xC20B4,bytes((48,16,255)))
    if c['entry']==8:
        sw(0x9CDA8,c.get('group',0));sw(0x9CF08,not c.get('usable',1))
        if not c.get('usable',1):sw(0x9D05C,0);sw(0x9D060,0)
    if c.get('nested'):execute(r,0x8004CC50,(29,0))
    if c['entry'] in (12,13,15,24):
        execute(r,0x8004CC50,(29,0));id=61 if c.get('nested') else 40
        window=find(1,id);node=find(2,id);sw(0x9CFFC,c.get('callback',0))
    if c['entry'] in (10,11,16,20,21,23,27):
        execute(r,0x80044F8C);window=find(1,41);node=find(2,41)
        sw(node+68,c.get('index',0));sw(node+72,0)
        sw(0x9CFA0,c.get('extra',0));sw(0x9CFA8,c.get('callback',0x80045110))
        if c['entry']==20:execute(r,0x80062F1C,(window,))
    if c['entry']==18:
        sw(0x9D160,(0,action_list,0x8014FF00)[c.get('saved_focus',0)])
        sw(0x9D15C,item_list)
    if c['entry']==27:
        q=0x80140500;sw(q,0);sw(q+4,c.get('event_type',4));sw(q+8,c.get('event',32))
        sw(0x9D0E0,q);sw(0x9D0E4,q);sw(0x9D0EC,1);sw(0x9D0E8,0)
    sw(0x9D100,0x80160000+c.get('offset',0))
    args=((0x8015FC00,c.get('width',140)),(0x8015FC00,),(0x8015FC00,),
          (c.get('id',5),),(c.get('id',5),),(0,),
          (0x8015FC00 if c.get('alias') else 0x8015FB80,0x8015FC00),
          (0x8015FB80,0x8015FB82 if c.get('alias') else 0x8015FC00),
          (c.get('index',0),),(action_list,),(c.get('index',0),),(node,),(node,),(window,),
          (29,0),(window,c.get('event',0)),(window,),(),(),(c.get('mask',0x3803FE),),
          (window,c.get('confirmed',0)),(window,c.get('event',0)),(),(),(),
          (0 if c.get('absent') else window,),(c.get('callback',0),),())[c['entry']]
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
        if '--dump' in sys.argv:(ROOT/f'pc_port/build/inventory-dialogs-oracle-{k}.bin').write_bytes(r)
    out=['/* Generated by pe_inventory_dialogs_oracle.py --write-header. */']
    for name,rows in (('ranges',RANGES),('common',common),('patches',patches)):
        out.append(f'static const uint32_t INV8_inventory_dialogs_{name}[][2]={{')
        out.extend(f'    {{0x{a:X}u,0x{b:X}u}},' for a,b in rows);out.append('};')
    out.append('static const struct { unsigned entry,first,end; uint32_t args[2],result; uint64_t hash; uint32_t frame; } INV8_inventory_dialogs_cases[]={')
    for e,a,b,args,result,h,frame in cases:
        params=','.join(f'0x{x:08X}u' for x in args) or '0'
        out.append(f'    {{{e},{a},{b},{{{params}}},0x{result:08X}u,UINT64_C(0x{h:016X}),{frame}u}},')
    out.append('};')
    if '--write-header' in sys.argv:(ROOT/'pc_port/tests/retail_inventory_dialogs_cases.h').write_text('\n'.join(out)+'\n')
    print(f'PASS: {len(cases)} original inventory dialog drawing, confirmation and queued-input cases')

if __name__=='__main__':main()
