#!/usr/bin/env python3
"""Original reward setup, score confirmation, level commit and loot windows.

All calls execute original MIPS instructions, including their delay slots.
--compare-native compares full RAM below the original execution stack (1FE000).
No capture or fixture is injected into a connected route.
"""
import hashlib
import struct
import subprocess
import sys
import tempfile
from pathlib import Path
from pe_battle_hud_oracle import ROOT, execute
from pe_inventory_entry_oracle import fixture as menu_fixture, RANGES
from pe_scripted_exit_oracle import words

ENTRIES=(0x8004B70C,0x8004B90C,0x8004B970,0x8004BB80,0x8004BC80,0x8004BCB4,
         0x8004C4B4,0x8004C520,0x80055668,0x8004C1E0,0x8005382C,0x80048654,
         0x80063D78,0x8005B8A8,0x80051DF8,0x80057ECC,0x80052764,0x8004BE4C,0x8004BF08)
RESULT_ENTRIES={3,8,9,10,13,14,15}
CASES=[]
def case(entry,**kw):CASES.append(dict(entry=entry,**kw))
for xp,gain in ((0,0),(0,5),(95,5),(99,302),(9700,700),(0x7FFFFFFF,1)):
    for bonus in (0,9,65536):
        for items in (0,2):case(0,xp=xp,gain=gain,bonus=bonus,items=items)
for bp in (99998,99999,0x7FFFFFFF,0xFFFFFFFF):case(0,xp=99,gain=1,bp=bp,bonus=3)
for entry in (1,4,5,7,11,17,18):
    for bank in (0,1):case(entry,bank=bank,items=2)
for xp,gain in ((0,5),(99,1),(100,0),(999999,0),(0,0)):
    for timer in (0,1,60):case(2,xp=xp,gain=gain,timer=timer)
for xp,gain in ((0,5),(99,1),(99,302),(9700,500)):
    for event in (0,32,0x10000):
        for snap in (0,1):
            for items in (0,2):case(3,xp=xp,gain=gain,event=event,snap=snap,items=items)
for existing in (0,1):
    for level in (0,1,3,19,20,255):case(6,level=level,existing=existing);case(8,level=level,existing=existing)
for busy in (0,1):
    for event in (0,0x10000):
        for gain in (0,1,302):
            for items in (0,2):case(9,xp=99,gain=gain,event=event,busy=busy,items=items)
for count in (-1,0,1,2,3,8,10,11):
    for pattern in (0,1,2,3):case(10,count=count,pattern=pattern)
for column,row in ((-1,-1),(0,0),(2,1),(999,999),(1,6)):
    for top in (0,2,7):case(12,column=column,row=row,top=top)
for xp in (-2147483648,-1,0,99,100,199,200,9700,9800,9900,2147483647):case(13,xp=xp)
for index in (0,1,40,98):
    for bonus in (0,19,0x7FFFFFFF):case(14,index=index,bonus=bonus)
for items in (0,1,10,0xFFFFFFFF):case(15,items=items)
case(16)


def fixture(exe,c):
    r,s,_=menu_fixture(exe,dict(entry=3,bank=c.get('bank',0)))
    def put(a,b):a&=0x1FFFFF;r[a:a+len(b)]=b;s[a:a+len(b)]=b
    def sw(a,v):put(a,struct.pack('<I',v&0xFFFFFFFF))
    def sh(a,v):put(a,struct.pack('<H',v&65535))
    def sb(a,v):put(a,bytes((v&255,)))
    def word(a):return struct.unpack_from('<I',r,a&0x1FFFFF)[0]
    # Deliberate threshold fixtures; the code executed remains the original EXE.
    sw(0xA8040,0x15B000-0xA8028)
    for i in range(128):sw(0x15B000+i*4,i*100)
    sw(0xA8048,0x142900-0xA8028)
    for i in range(20):sh(0x142900+i*4,i*2+1);sh(0x142902+i*4,3)
    xp=c.get('xp',95);gain=c.get('gain',5)
    sw(0xC0E00,xp);sb(0xC0E0A,min(98,max(0,xp//100)))
    sh(0xC0E1E,65534);sw(0xC0E10,c.get('bp',90))
    sw(0x9D01C,0);sw(0x9D02C,0);sw(0x9CF78,c.get('timer',0))
    count=c.get('items',0)
    for i in range(min(count,10)):sh(0x142800+i*4,6+i%2);sh(0x142802+i*4,i+1)
    sh(0x142800+min(count,10)*4,0)
    args=(gain,c.get('bonus',2),0x80142800 if count else 0)
    entry=c['entry']
    window=0
    if entry in (2,3,9,11):
        execute(r,0x8004B70C,args,instruction_budget=900000)
        window=word(0x9D15C)
    if entry==3 and c.get('snap'):sw(0x9CFE8,word(0x9CFEC))
    if entry==9:
        execute(r,0x80062F9C)
        execute(r,0x8004BE4C)
        window=word(0x9D15C);sw(0x9CF80,c.get('busy',0))
        if not c.get('busy'):
            sw(0x9CF60,word(0x9CF6C));sw(0x9CF64,word(0x9CF70));sw(0x9CF68,word(0x9CF74))
    if entry in (6,8) and c.get('existing'):execute(r,0x8004C4B4,(7,))
    if entry in (7,):sw(0x9CFF4,3)
    if entry==10:
        patterns=((0,)*10,(6,)*10,(6,0,0,6,0,0,0,0,0,0),(0,6,0,6,0,6,0,6,0,6))
        for i,v in enumerate(patterns[c['pattern']]):sh(0xC0E48+i*2,v)
    if entry==12:
        window=0x80142980
        for offset,value in ((84,2),(88,8),(56,3),(92,c['top'])):sw(window+offset,value)
    if entry==14:sw(0xA1B30,c['bonus'])
    if entry==15:sw(0x9D078,count)
    args=(args,(),(),(window,c.get('event',0x10000)),(),(),(c.get('level',0),),(),
          (c.get('level',0),),(window,c.get('event',0x10000)),(c.get('count',1),),(),
          (window,c.get('column',0),c.get('row',0)),(xp,0x8015B000),(c.get('index',0),),(),(),(),())[entry]
    for a,n in RANGES:s[a:a+n]=r[a:a+n]
    return r,s,tuple(a&0xFFFFFFFF for a in args)

SWITCH='''
case 0:func_8004B70C(a,b,c);break;
case 1:func_8004B90C();break;
case 2:func_8004B970();break;
case 3:result=func_8004BB80(a,b);break;
case 4:func_8004BC80();break;
case 5:func_8004BCB4();break;
case 6:func_8004C4B4(a);break;
case 7:func_8004C520();break;
case 8:result=func_80055668(a);break;
case 9:result=func_8004C1E0(a,b);break;
case 10:result=func_8005382C((int32_t)a);break;
case 11:func_80048654();break;
case 12:func_80063D78(a,(int32_t)b,(int32_t)c);break;
case 13:result=func_8005B8A8((int32_t)a,b);break;
case 14:result=func_80051DF8((int32_t)a);break;
case 15:result=func_80057ECC();break;
case 16:func_80052764();break;
case 17:func_8004BE4C();break;
case 18:func_8004BF08();break;
'''
NATIVE='''#include "psx_compat.h"
#include "pe_port_compat.h"
#include "game_port.h"
#include <stdio.h>
#include <stdlib.h>
void Trace_Direct(const char *s) {(void)s;}
int main(int argc,char **argv) {
 if(argc!=7)return 2;PE_RamInit();FILE *f=fopen(argv[1],"rb");
 if(!f || fread(PE_Translate(PE_RAM_BASE,PE_RAM_SIZE),1,PE_RAM_SIZE,f)!=PE_RAM_SIZE)return 3;
 fclose(f);D_8009D1A0=PE_LoadU32(0x8009D1A0u);
 unsigned result=0,a=strtoul(argv[4],0,0),b=strtoul(argv[5],0,0),c=strtoul(argv[6],0,0);
 switch(atoi(argv[3])) {'''+SWITCH+'''}
 if(PE_Port_ShouldStop())return 4;
 PE_StoreU32(0x8009D1A0u,D_8009D1A0);
 f=fopen(argv[2],"wb");if(!f)return 5;
 fwrite(PE_Translate(PE_RAM_BASE,PE_RAM_SIZE),1,PE_RAM_SIZE,f);fclose(f);
 printf("%u\\n",result);return 0;
}'''


def fingerprint(r):
    h=14695981039346656037
    for a,n in RANGES:
        for b in r[a:a+n]:h=((h^b)*1099511628211)&0xFFFFFFFFFFFFFFFF
    return h


def main():
    exe=(ROOT/'build/extracted/disc1/SLUS_006.62').read_bytes()
    assert hashlib.sha1(exe).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
    _,s,_=fixture(exe,CASES[0]);base=words(s);common=[(i*4,v) for i,v in enumerate(base) if v]
    covered=bytearray(0x1FE000)
    for a,n in RANGES:covered[a:a+n]=bytes([1])*n
    patches=[];cases=[];all_seen=set()
    with tempfile.TemporaryDirectory(prefix='pe-reward-oracle-') as directory:
        temp=Path(directory);binary=temp/'native';inp=temp/'in.bin';output=temp/'out.bin'
        compare='--compare-native' in sys.argv
        if compare:
            build=Path(sys.argv[sys.argv.index('--build-dir')+1]) if '--build-dir' in sys.argv else ROOT/'pc_port/build'
            source=temp/'native.c';source.write_text(NATIVE)
            subprocess.run(['cc','-O2',*[f'-I{ROOT}/pc_port/{p}' for p in ('include','platform','bootstrap','src','.')],str(source),str(build.resolve()/'libpe_field_runtime.a'),'-o',str(binary)],check=True)
        for k,c in enumerate(CASES):
            r,s,args=fixture(exe,c);initial=bytes(r);seen=set();first=len(patches)
            patches.extend((i*4,v) for i,(v,b) in enumerate(zip(words(s),base)) if v!=b)
            regs=execute(r,ENTRIES[c['entry']],args,instruction_budget=900000,visited_pcs=seen)
            for pc in seen|{p+4 for p in seen}:
                offset=pc-0x8000F800
                assert 0x800<=offset<len(exe)-3 and initial[pc&0x1FFFFF:(pc&0x1FFFFF)+4]==exe[offset:offset+4],('instruction',hex(pc))
            all_seen.update(seen)
            assert all(covered[i] or r[i]==initial[i] for i in range(0x1FE000)),('uncovered write',c)
            result=regs[2] if c['entry'] in RESULT_ENTRIES else 0
            if compare:
                inp.write_bytes(initial)
                got=int(subprocess.check_output([str(binary),str(inp),str(output),str(c['entry']),*[str(v) for v in (*args,0,0,0)[:3]]]))
                assert got==result,('return',c,hex(got),hex(result))
                native=output.read_bytes()
                if native[:0x1FE000]!=r[:0x1FE000]:
                    Path('/tmp/pe-reward-mismatch-original.bin').write_bytes(r);Path('/tmp/pe-reward-mismatch-native.bin').write_bytes(native)
                    raise AssertionError((c,[(hex(i),native[i],r[i]) for i in range(0x1FE000) if native[i]!=r[i]][:30]))
            h=fingerprint(r);cases.append((c['entry'],first,len(patches),args,result,h))
            print(k,c,hex(result),hex(h),flush=True)
    out=['/* Generated by pe_battle_reward_oracle.py --write-header; original MIPS execution. */']
    for name,rows in (('ranges',RANGES),('common',common),('patches',patches)):
        out.append(f'static const uint32_t REWARD_{name}[][2]={{')
        out.extend(f'{{0x{a:X}u,0x{b:X}u}},' for a,b in rows);out.append('};')
    out.append('static const struct { unsigned entry,first,end; uint32_t args[3],result; uint64_t hash; } REWARD_cases[]={')
    for e,a,b,args,result,h in cases:
        params=','.join(f'0x{x:08X}u' for x in args) or '0'
        out.append(f'{{{e},{a},{b},{{{params}}},0x{result:08X}u,UINT64_C(0x{h:016X})}},')
    out.append('};')
    if '--write-header' in sys.argv:(ROOT/'pc_port/tests/retail_battle_reward_cases.h').write_text('\n'.join(out)+'\n')
    print('PASS:',len(cases),'complete original reward entry cases;',len(all_seen),'original instruction PCs; native comparison',compare)

if __name__=='__main__':main()
