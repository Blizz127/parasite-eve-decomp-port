#!/usr/bin/env python3
"""Execute the retail loot render, transfer, bulk collection and finalization.

Native comparisons cover all RAM below 1FE000 (original stack excluded).
--capture PATH additionally compares the reached loot renderers and the
Take All/Done sequence in isolation from an actual connected RAM snapshot.
"""
import hashlib
import struct
import subprocess
import sys
import tempfile
from pathlib import Path
from pe_battle_hud_oracle import ROOT,execute
from pe_battle_reward_oracle import fixture as reward_fixture,RANGES,fingerprint
from pe_scripted_exit_oracle import words

ENTRIES=(0x8004FCF8,0x8004FD68,0x80050B94,0x80050BE8,0x80057F14,
         0x80058030,0x8005833C,0x80058454,0x80058670,0x80048838,0x80062FEC,0x8005E30C,0x80052C6C)
RESULT_ENTRIES={5,6,9}
CASES=[]
def case(entry,**kw):CASES.append(dict(entry=entry,**kw))
for entry in (0,1,2,3,4,10):
    for item in (6,256,512,519):
        for bank in (0,1):case(entry,item=item,bank=bank)
for entry in (3,4):
    for item in (0,255,383,384,511,520):case(entry,item=item)
for index in (-1,0,1,3):
    for full in (0,1):
        for item in (1,6,512,519):case(6,index=index,full=full,item=item)
for full in (0,1):
    for item in (0,1,6,256,512,519):case(7,item=item,full=full)
for pattern in (0,1,2):
    for pool in (0,1,2):case(8,pattern=pattern,pool=pool)
for first_list,second_list in ((13,13),(14,14),(13,14),(14,13),(99,13)):
    for first,second in ((0,0),(0,1),(1,0),(1,1)):
        for item in (6,519):case(5,first_list=first_list,second_list=second_list,first=first,second=second,item=item)
for first_list,second_list in ((13,14),(14,13)):
    for protected in (0,64):
        for equipped in (-1,0,1):case(5,first_list=first_list,second_list=second_list,first=0,second=1,item=6,protected=protected,equipped=equipped)
for entry in (9,11):
    for row in (-1,0,1,2,3):
        for event in (0,32,0x10000):case(entry,row=row,event=event,item=519)

for first in (0,25,252):case(12,first=first)

def fixture(exe,c):
    r,s,_=reward_fixture(exe,dict(entry=11,items=2,bank=c.get('bank',0)))
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
    item=c.get('item',6)
    # The loot graph uses only item records; provide the true bounded table end.
    sw(0xA8038,0x156000-0xA8028)
    for i in range(9):
        p=0xA1E64+i*32;put(p,r[0x154000:0x154020]);sb(p+4,i+1);sb(p+5,3);sb(p+6,16+i%3);sb(p+9,90);sh(p+10,13);sh(p+18,9)
    # Distinct names and quantities, including a named temporary weapon.
    sb(0xA1F44+4,1);sb(0xA1F44+6,16)
    sb(0xC0EAC+5,19);sb(0xC0EAC+6,1)
    sb(0xC0EAC+4,1);sb(0xC0EAC,1)
    sb(0xC0E20,c.get('equipped',-1));sb(0xC0E22,-1)
    sh(0xC0E48,6);sh(0xC0E4A,7)
    if c.get('full'):
        for i in range(10):sh(0xC0E48+i*2,6)
    sb(0x154000+6*32+5,c.get('protected',3))
    sh(0xA1FD4,item);sh(0xA1FD6,6);sw(0x9D078,2)
    execute(r,0x80048654,instruction_budget=900000)
    window=find(1,12);command=find(2,12);loot=find(2,13)
    sw(command+72,c.get('row',0));sw(command+68,0)
    if c['entry']==8:
        ids=((1,6,256,0,519,257,2,0,0,0),(1,2,3,4,5,6,7,8,9,10),(256,257,258,259,260,261,262,263,264,265))[c['pattern']]
        for i,id in enumerate(ids):sh(0xC0E48+i*2,id)
        for i in range(10):
            p=0x154000+i*32;sb(p,1);sb(p+6,i+1)
        for i in range(128):sb(0xC0EAC+i*32,1 if c['pool']==2 or (c['pool']==1 and i<127) else 0)
    if c['entry']==11:
        q=0x80142500;sw(q,0);sw(q+4,1);sw(q+8,c['event'])
        sw(0x9D0E0,q);sw(0x9D0E4,q);sw(0x9D0EC,1)
    if c['entry']==12:
        for i in range(256):sb(0x154000+i*32+6,10)
        for i in range(3):sb(0x154000+(c['first']+i)*32+6,19+i)
        sw(0x9D03C,0)
    sw(0x9CEF4,command)
    args=((command,),(loot,),(c.get('index',1),),(0,),(0,),
          (c.get('first_list',13),c.get('first',0),c.get('second_list',14),c.get('second',1)),
          (c.get('index',0),),(),(),(window,c.get('event',0x10000)),(),(),())[c['entry']]
    for a,n in RANGES:s[a:a+n]=r[a:a+n]
    return r,s,tuple(a&0xFFFFFFFF for a in args)

SWITCH='''
case 0:func_8004FCF8(a);break;
case 1:func_8004FD68(a);break;
case 2:func_80050B94(a);break;
case 3:func_80050BE8(a);break;
case 4:func_80057F14(a);break;
case 5:result=func_80058030(a,(int32_t)b,c,(int32_t)d);break;
case 6:result=func_8005833C((int32_t)a);break;
case 7:func_80058454();break;
case 8:func_80058670();break;
case 9:result=func_80048838(a,b);break;
case 10:func_80062FEC();break;
case 11:func_8005E30C();break;
case 12:func_80052C6C();break;
'''
NATIVE='''#include "psx_compat.h"
#include "pe_port_compat.h"
#include "game_port.h"
#include <stdio.h>
#include <stdlib.h>
void Trace_Direct(const char *s) {(void)s;}
int main(int argc,char **argv) {
 if(argc!=8)return 2;PE_RamInit();FILE *f=fopen(argv[1],"rb");
 if(!f || fread(PE_Translate(PE_RAM_BASE,PE_RAM_SIZE),1,PE_RAM_SIZE,f)!=PE_RAM_SIZE)return 3;
 fclose(f);D_8009D1A0=PE_LoadU32(0x8009D1A0u);
 unsigned result=0,a=strtoul(argv[4],0,0),b=strtoul(argv[5],0,0),c=strtoul(argv[6],0,0),d=strtoul(argv[7],0,0);
 switch(atoi(argv[3])) {'''+SWITCH+'''}
 if(PE_Port_ShouldStop())return 4;
 PE_StoreU32(0x8009D1A0u,D_8009D1A0);
 f=fopen(argv[2],"wb");if(!f)return 5;
 fwrite(PE_Translate(PE_RAM_BASE,PE_RAM_SIZE),1,PE_RAM_SIZE,f);fclose(f);
 printf("%u\\n",result);return 0;
}'''


def main():
    exe=(ROOT/'build/extracted/disc1/SLUS_006.62').read_bytes()
    assert hashlib.sha1(exe).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
    _,s,_=fixture(exe,CASES[0]);base=words(s);common=[(i*4,v) for i,v in enumerate(base) if v]
    covered=bytearray(0x1FE000)
    for a,n in RANGES:covered[a:a+n]=bytes([1])*n
    patches=[];cases=[];all_seen=set()
    with tempfile.TemporaryDirectory(prefix='pe-loot-oracle-') as directory:
        temp=Path(directory);binary=temp/'native';inp=temp/'in.bin';output=temp/'out.bin'
        compare='--compare-native' in sys.argv
        if compare:
            build=Path(sys.argv[sys.argv.index('--build-dir')+1]) if '--build-dir' in sys.argv else ROOT/'pc_port/build'
            source=temp/'native.c';source.write_text(NATIVE)
            subprocess.run(['cc','-O2',*[f'-I{ROOT}/pc_port/{p}' for p in ('include','platform','bootstrap','src','.')],str(source),str(build.resolve()/'libpe_field_runtime.a'),'-o',str(binary)],check=True)
        def check(r,entry,args,label,check_ranges=True):
            initial=bytes(r);seen=set()
            regs=execute(r,ENTRIES[entry],args,instruction_budget=900000,visited_pcs=seen)
            for pc in seen|{p+4 for p in seen}:
                offset=pc-0x8000F800
                assert 0x800<=offset<len(exe)-3 and initial[pc&0x1FFFFF:(pc&0x1FFFFF)+4]==exe[offset:offset+4],('instruction',hex(pc))
            all_seen.update(seen)
            if check_ranges:assert all(covered[i] or r[i]==initial[i] for i in range(0x1FE000)),('uncovered write',label)
            result=regs[2] if entry in RESULT_ENTRIES else 0
            if compare:
                inp.write_bytes(initial)
                got=int(subprocess.check_output([str(binary),str(inp),str(output),str(entry),*[str(v) for v in (*args,0,0,0,0)[:4]]]))
                assert got==result,('return',label,hex(got),hex(result))
                native=output.read_bytes()
                if native[:0x1FE000]!=r[:0x1FE000]:
                    Path('/tmp/pe-loot-mismatch-original.bin').write_bytes(r);Path('/tmp/pe-loot-mismatch-native.bin').write_bytes(native)
                    raise AssertionError((label,[(hex(i),native[i],r[i]) for i in range(0x1FE000) if native[i]!=r[i]][:30]))
            return result
        selected=[] if '--capture-only' in sys.argv else CASES
        for k,c in enumerate(selected):
            r,s,args=fixture(exe,c);first=len(patches)
            patches.extend((i*4,v) for i,(v,b) in enumerate(zip(words(s),base)) if v!=b)
            result=check(r,c['entry'],args,c);h=fingerprint(r)
            cases.append((c['entry'],first,len(patches),args,result,h))
            print(k,c,hex(result),hex(h),flush=True)
        if '--capture' in sys.argv:
            assert compare
            capture=Path(sys.argv[sys.argv.index('--capture')+1]);initial=capture.read_bytes()
            print('CAPTURE',capture,hashlib.sha256(initial).hexdigest(),flush=True)
            word=lambda r,a:struct.unpack_from('<I',r,a&0x1FFFFF)[0]
            def find(r,kind,id):
                p=word(r,0x9D154)
                while p:
                    if word(r,p+32)==kind and word(r,p+36)==id:return p
                    p=word(r,p)
                raise AssertionError((kind,id))
            for e,kind,id in ((0,2,12),(1,2,13),(10,0,0),(12,0,0)):
                r=bytearray(initial);args=(find(r,kind,id),) if kind else ()
                check(r,e,args,('capture draw',e),False);print('PASS captured draw',e,flush=True)
            r=bytearray(initial);window=find(r,1,12)
            check(r,9,(window,0x10000),('capture Take All',),False)
            check(r,9,(window,0x10000),('capture Done',),False)
            print('PASS captured Take All / Done sequence',flush=True)
    out=['/* Generated by pe_battle_loot_oracle.py --write-header; original MIPS execution. */']
    for name,rows in (('ranges',RANGES),('common',common),('patches',patches)):
        out.append(f'static const uint32_t LOOT_{name}[][2]={{')
        out.extend(f'{{0x{a:X}u,0x{b:X}u}},' for a,b in rows);out.append('};')
    out.append('static const struct { unsigned entry,first,end; uint32_t args[4],result; uint64_t hash; } LOOT_cases[]={')
    for e,a,b,args,result,h in cases:
        params=','.join(f'0x{x:08X}u' for x in args) or '0'
        out.append(f'{{{e},{a},{b},{{{params}}},0x{result:08X}u,UINT64_C(0x{h:016X})}},')
    out.append('};')
    if '--write-header' in sys.argv:
        assert selected
        (ROOT/'pc_port/tests/retail_battle_loot_cases.h').write_text('\n'.join(out)+'\n')
    print('PASS:',len(cases),'original loot cases;',len(all_seen),'instruction PCs; native comparison',compare)

if __name__=='__main__':main()
