#!/usr/bin/env python3
"""Original ammunition controls, commit history and complete reload screens.

Runs the authenticated executable's full call graphs without replacing callees.
"""
import hashlib
import struct
import sys
from pe_battle_hud_oracle import ROOT,execute
from pe_inventory_actions_oracle import fixture as actions_fixture,RANGES,fingerprint
from pe_scripted_exit_oracle import words

ENTRIES=(0x8005E120,0x80051098,0x80056B24,0x80057094,0x80056FB8,
         0x800453E8,0x800452C0,0x80044274,0x80062FEC,0x8005E30C,0x800512AC)
RESULT_ENTRIES={0,1,2,6}
CASES=[]
def case(entry=2,**kw):CASES.append(dict(entry=entry,**kw))
for step in (-2147483648,-1,0,1,10,2147483647):case(0,step=step)
for position in (0,36,72,108,144,180):case(1,history=position)
for amount in (-2147483648,-1000,-10,-1,0,1,10,1000,2147483647):
    for counts in ((0,0),(1,9),(9,1),(999,999),(65535,65535)):
        case(amount=amount,counts=counts)
for amount in (-10,0,10):
    for capacity,bonus in ((0,0),(0,-1),(10,-100),(255,744),(255,745),(255,32767)):
        for reserve in (0,37):case(amount=amount,capacity=capacity,bonus=bonus,reserve=(reserve,reserve))
for changed in (0,1,2,3):
    for pending in (0,1):
        for equipped in (-1,0,1):case(3,changed=changed,pending=pending,equipped=equipped)
for reserve in ((0,0),(30,0),(0,30),(30,40)):
    for capacity,bonus in ((10,0),(255,1000),(0,-1)):
        case(3,changed=3,reserve=reserve,extra_capacity=capacity,extra_bonus=bonus)
for first,second in ((0x800A1F94,0x800A1FB4),(0x800A1FA0,0x800A1FB8),
                     (0,0x800C0EAC),(0x800C0EAC,0),(0x800C0EAC,0x800C0EAC)):
    case(3,changed=3,refs=(first,second))
for bank in (0,1):
    for missing in (0,1,2):case(3,changed=3,bank=bank,missing=missing,history=144)
for entry in (4,5,8):
    for bank in (0,1):
        for frame in (0,8,31):case(entry,bank=bank,frame=frame)
for kind in (1,6,9,19):
    for renamed in (0,1):case(4,kind=kind,renamed=renamed)
for event in (0,32,64,0x1000,0x4000,0x10000,0x5000,0x10040,0x1040,0x4040):
    for step in (1,10):case(6,event=event,step=step)
for event in (64,0x10000):
    for changed in (0,3):case(6,event=event,changed=changed)
for matches in (0,1,3):
    for lists in (0,1,2):
        for cursor in (-1,0,1):case(7,matches=matches,lists=lists,cursor=cursor)
for bank in (0,1):
    for matches in (0,1,3):case(7,bank=bank,matches=matches,lists=2,cursor=1)
for event_type,event in ((1,0x1000),(1,0x4000),(1,64),(4,32)):
    for step in (1,10):case(9,event_type=event_type,event=event,step=step,changed=3)
for alias in (0,1):
    for kind in (0,1,5,9,19,255):case(10,src_alias=alias,kind=kind)
for entry in (4,5,8):case(entry,offset=0x3FE0)

def fixture(exe,c):
    r,s,_=actions_fixture(exe,dict(entry=10,index=0,alternate=c.get('bank',0),kind=1))
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
    put(0xC0E48,bytes(100));put(0x15F800,bytes(256))
    for a in (0xC0E48,0x15F800):sh(a,256);sh(a+2,266)
    bank=c.get('bank',0);sw(0x9D04C,0x8015F800 if bank else 0);sw(0x9D054,10)
    sw(0x9D108,bank);sw(0x9CDDC,bank);sw(0x956AC,c.get('frame',0))
    sb(0xC0E20,c.get('equipped',0));sw(0x15C168,0x8015D100)
    sw(0x15D10C,0xA5A55678);sw(0x9D010,c.get('pending',0))
    if c.get('missing')==1:sw(0x9D254,0)
    if c.get('missing')==2:sw(0x15C000,0)
    for i in range(36):sw(0xA1AA0+i*4,0x12000000+i*0x12345)
    sw(0x9D014,0x800A1AA0+c.get('history',0));sw(0x9D0F4,c.get('step',1))
    sw(0x9CF88,bank);sw(0x9CF8C,0);sw(0x9CF90,bank);sw(0x9CF94,1)
    execute(r,0x80052E30,(bank,))
    if c['entry']!=7:
        execute(r,0x800451D0,(node,));window=find(1,62)
        # Constructor orders ammo before weapon. Both mirror counts are
        # independently changed to exercise each commit comparison.
        for i in range(2):
            p=0xA1F94+i*32;reference=word(0x9D070+i*4)
            count=c.get('counts',(7,3))[i]
            sh(reference+10,count);sh(p+10,count+bool(c.get('changed',0)&(1<<i)))
            sb(p+9,c.get('capacity',10));sh(p+18,c.get('bonus',0))
            sh(p+12,c.get('reserve',(0,0))[i]);sb(p+31,i+1)
            extra=0xA1E64+i*32;sb(extra+9,c.get('extra_capacity',100))
            sh(extra+18,c.get('extra_bonus',0));sh(extra+10,80)
        if 'refs' in c:
            for i,a in enumerate(c['refs']):sw(0x9D070+i*4,a)
        if c['entry']==4:
            for p in (0xA1F94,0xA1FB4):
                sb(p+6,c.get('kind',1))
                if c.get('renamed'):sb(p+5,r[p+5]|16)
            put(0xC20A4,bytes((16,32,255)));put(0xC20B4,bytes((48,16,255)))
    else:
        for a in (0xC0E48,0x15F800):
            put(a,bytes(20));sh(a,256)
            for i in range(c.get('matches',1)):sh(a+(i+1)*2,266)
        if bank:put(0xC0E48,bytes(20))
        for id in (13,14)[:c.get('lists',0)]:
            p=execute(r,0x80062D2C,(id,0,0,0))[2]
            n=execute(r,0x8006322C,(id,p,p))[2]
            sw(n+68,c.get('cursor',0));sw(n+72,0)
        sw(0x9D15C,node)
    src=0x8015D000
    if c['entry']==10:
        sb(0xC0EB2,c.get('kind',1));sh(0xC0EB6,3)
        if c.get('src_alias'):src=0x8015D10C
        sw(src,0x800C0EAC)
        # After the ammo bits replace the low ten bits the argument points here.
        sb(0xC0C09,c.get('kind',1))
    if c['entry']==9:
        q=0x80140500;sw(q,0);sw(q+4,c.get('event_type',1));sw(q+8,c.get('event',0x1000))
        sw(0x9D0E0,q);sw(0x9D0E4,q);sw(0x9D0EC,1);sw(0x9D0E8,0)
    sw(0x9D100,0x80160000+c.get('offset',0))
    args=((),(),(c.get('amount',1),),(),(),(window,),(window,c.get('event',0)),
          (0,),(),(),(5,src))[c['entry']]
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
        if '--dump' in sys.argv:(ROOT/f'pc_port/build/inventory-ammo-oracle-{k}.bin').write_bytes(r)
    out=['/* Generated by pe_inventory_ammo_oracle.py --write-header. */']
    for name,rows in (('ranges',RANGES),('common',common),('patches',patches)):
        out.append(f'static const uint32_t INV9_inventory_ammo_{name}[][2]={{')
        out.extend(f'    {{0x{a:X}u,0x{b:X}u}},' for a,b in rows);out.append('};')
    out.append('static const struct { unsigned entry,first,end; uint32_t args[2],result; uint64_t hash; uint32_t frame; } INV9_inventory_ammo_cases[]={')
    for e,a,b,args,result,h,frame in cases:
        params=','.join(f'0x{x:08X}u' for x in args) or '0'
        out.append(f'    {{{e},{a},{b},{{{params}}},0x{result:08X}u,UINT64_C(0x{h:016X}),{frame}u}},')
    out.append('};')
    if '--write-header' in sys.argv:(ROOT/'pc_port/tests/retail_inventory_ammo_cases.h').write_text('\n'.join(out)+'\n')
    print(f'PASS: {len(cases)} original ammunition transfer, confirmation and screen cases')

if __name__=='__main__':main()
