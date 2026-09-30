#!/usr/bin/env python3
"""Complete original player tick, damage acknowledgement and status recovery."""
import hashlib
import struct
import sys
from pe_battle_hud_oracle import ROOT,execute
from pe_enemy_tick_oracle import fixture as enemy_fixture,RANGES as ENEMY_RANGES
from pe_scripted_exit_oracle import words

RANGES=ENEMY_RANGES+((0x9E068,0xBD0),(0xA22E0,0xD80),(0xBCF88,4))
ENTRIES=(0x8001D340,0x8001F4D4,0x8001F814,0x8001F9C4,0x800201DC,
         0x80020288,0x80020CE4)
CASES=[]
def case(entry=0,**kw):CASES.append(dict(entry=entry,**kw))
for mode in (0,1,2,256,257):
    for flags in (0,0x40,0x80,0x100,0x140):case(mode=mode,flags=flags)
for bank in (0,1):
    for phase in range(4):
        for timer in (0,89,90):case(bank=bank,phase=phase,flags=0x2000,pe=0,exhaust=timer)
        for shield in (0,1,2):case(bank=bank,phase=phase,shield=shield)
for pe in (-1,0,1,79<<16,80<<16,(80<<16)-50):case(pe=pe)
for rate in (0,0x1999,0x10000):case(pe=10<<16,rate=rate,regen=1)
for cmd in range(20):case(cmd=cmd,mode=1)
for timer in (0,1,2,90,255):case(flags=0x800000,timer=timer)
case(flags=0x180000);case(flags=0x4000,battle=0x102)
for pending in (0,1):
    for state in range(5):case(flags=0x4000,pending=pending,action=state)
case(flags=0x4000,pending=1,frames=2)
case(flags=0x4000,pending=1,frames=3)
for damage in (0,1,3,15,45,80):case(flags=0x4000,pending=1,damage=damage)
for kind in (0,1,2,3,4,5):case(flags=0x4000,pending=1,kind=kind,element=0)
for contact in (0,1):
    for flags in (0,0x200,0x1000000):case(contact=contact,flags=flags)
for angle in (0,1024,2048,3072):case(knock=3,angle=angle)
case(knock=3,actor_flags=0xC0000);case(knock=1,speed=0)
for hp,previous in ((50,45),(42,45),(45,40),(0,45),(-5,45)):case(hp=hp,previous=previous)
case(hp=0,flags=0x10000,viewport=0xE000)
for mode in (0,1):case(battle_mode=mode,flags=0x4000,pending=1)
for flags in (0,0x100,0x1000,0x1100,0x200):
    for defense in (0,25,100):case(1,flags=flags,defense=defense)
case(1,critical=100);case(1,critical=255,crit_resist=255)
case(1,flags=0x200,attribute=10);case(1,shield=1,damage=0)
for cmd in range(20):case(2,cmd=cmd)
case(2,actor_flags=0x100);case(2,flags=0x12000)
for flags in (1,4,16,64,128,256,512,1024,0x1000000,0x7FF):
    for timer in (0,1,90):case(3,flags=flags,status_timer=timer,timer=timer,pe=2<<16,cmd=17)
for tick in (0,4,5,255):case(4,poison_tick=tick)
for attr in range(18):
    for flags in (0,0x3FF):case(5,attribute=attr,flags=flags)
for attr in (1,3,4,5,6):
    for seed in (1,3,10):case(5,attribute=attr,armor_flags=15,seed=seed)
case(6,flags=0x10000)

def fixture(exe,c):
    r,s=enemy_fixture(exe,{'entry':0})
    def put(a,b):r[a:a+len(b)]=b;s[a:a+len(b)]=b
    def sw(a,v):put(a,struct.pack('<I',v&0xFFFFFFFF))
    def sh(a,v):put(a,struct.pack('<H',v&65535))
    def sb(a,v):put(a,bytes((v&255,)))
    p=0x144000;b=0xA5D5C;action=b+0x1C
    sh(p+4,7);sw(p+8,c.get('pe',80<<16));sh(p+12,c.get('hp',45));sh(p+14,c.get('previous',45))
    sh(p+16,8990);sb(p+18,4);sh(p+28,45);sh(p+32,c.get('defense',25));sh(p+36,17)
    sw(p+40,80<<16);sw(p+44,c.get('rate',0x10000));sw(p+48,c.get('regen',100));sw(p+52,c.get('shield',0))
    sh(p+56,2);sb(p+58,c.get('poison_tick',0));sb(p+59,5);sh(p+60,5);sh(p+62,3)
    for a in (64,66,68,70):sh(p+a,c.get('status_timer',10))
    sb(p+72,c.get('knock',0));sb(p+73,c.get('speed',6));sh(p+74,c.get('angle',0));sw(p+76,c.get('flags',0))
    sw(p+108,0x80147100);sw(0x147100,c.get('crit_resist',0)<<20);sw(0x147104,c.get('armor_flags',0))
    sb(0x14000E,c.get('cmd',4));sb(0x14000F,8);sw(0x140014,0x70000);sw(0x140098,c.get('actor_flags',0))
    sh(0x140210,160);sh(0x140212,120);sw(0x140040,100<<16);sw(0x140048,200<<16)
    sw(b,(c.get('kind',3)<<21)|(0x80000000 if c.get('pending') else 0));sw(b+24,0x80000000+action)
    sb(action,c.get('action',1));sb(action+1,c.get('attribute',0));sh(action+12,c.get('damage',8));sb(action+14,c.get('element',1))
    sb(b+144,c.get('critical',0));sb(b+146,3);sb(b+147,12);sb(b+148,2);sb(b+149,5)
    sw(0x141098,0x2000000 if c.get('contact') else 0);sw(0xA5E50,0)
    sw(0x9D1A0,c.get('battle',0x82));sw(0x9D28C,c.get('battle_mode',0));sw(0x9CDDC,c.get('bank',0))
    sw(0x9D1E8,c.get('phase',0));sb(0x9CE30,c.get('exhaust',0));sb(0x9CE34,c.get('timer',90));sb(0x9D234,c.get('timer',0))
    sh(0x9D228,c.get('status_timer',2));sw(0xBCF88,c.get('viewport',0));sw(0x9D200,0xFFFFFFFF);sw(0x9D2FC,0xFFFFFFFF)
    put(0x9E068,bytes((i*37+19)&255 for i in range(0xBD0)))
    args=((c.get('mode',2),),(0x80141000,),(0x80141000,),(),(),(0x800A5D5C,),())[c['entry']]
    return r,s,args

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
        for _ in range(c.get('frames',1)):regs=execute(r,ENTRIES[c['entry']],args,bios_seed=c.get('seed',1))
        result=regs[2] if c['entry']==2 else 0
        cases.append((c['entry'],c.get('frames',1),c.get('seed',1),first,len(patches),args,result,fingerprint(r)))
        print(k,c,hex(result),hex(fingerprint(r)),flush=True)
        if '--dump' in sys.argv:(ROOT/f'pc_port/build/player-tick-oracle-{k}.bin').write_bytes(r)
    out=['/* Generated by pe_player_tick_oracle.py --write-header. */']
    for name,rows in (('ranges',RANGES),('common',common),('patches',patches)):
        out.append(f'static const uint32_t ATK31_player_{name}[][2]={{')
        out.extend(f'    {{0x{a:X}u,0x{b:X}u}},' for a,b in rows);out.append('};')
    out.append('static const struct { unsigned entry,frames,seed,first,end; uint32_t args[1],result; uint64_t hash; } ATK31_player_cases[]={')
    for e,f,seed,a,b,args,v,h in cases:
        params=','.join(f'0x{x:08X}u' for x in args) or '0'
        out.append(f'    {{{e},{f},{seed},{a},{b},{{{params}}},0x{v:08X}u,UINT64_C(0x{h:016X})}},')
    out.append('};')
    if '--write-header' in sys.argv:(ROOT/'pc_port/tests/retail_player_tick_cases.h').write_text('\n'.join(out)+'\n')
    print(f'PASS: {len(cases)} complete original player tick / damage / status call graphs')

if __name__=='__main__':main()
