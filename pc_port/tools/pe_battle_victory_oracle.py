#!/usr/bin/env python3
"""Execute the original complete encounter-victory handler and its callees.

Native comparisons cover all RAM below 1FE000 (original stack excluded).
--capture PATH additionally compares the reached victory handler in isolation from an actual connected RAM snapshot.
"""
import hashlib
import struct
import subprocess
import sys
import tempfile
from pathlib import Path
from pe_battle_hud_oracle import ROOT,execute
from pe_battle_reward_oracle import fixture as reward_fixture,RANGES as REWARD_RANGES
from pe_scripted_exit_oracle import words,RANGES as EXIT_RANGES

def union_ranges(ranges):
    out=[]
    for a,n in sorted(ranges):
        if out and a<=out[-1][0]+out[-1][1]:
            x,y=out[-1];out[-1]=(x,max(x+y,a+n)-x)
        else:out.append((a,n))
    return tuple(out)
RANGES=union_ranges(REWARD_RANGES+EXIT_RANGES+((0,512),(0x942E0,12),(0x180000,0x12000)))
def fingerprint(r):
    value=0xCBF29CE484222325
    for a,n in RANGES:
        for byte in r[a:a+n]:value=((value^byte)*0x100000001B3)&0xFFFFFFFFFFFFFFFF
    return value

ENTRIES=(0x8002B0E8,)
RESULT_ENTRIES=set()
CASES=[]
def case(**kw):CASES.append(dict(entry=0,**kw))
for command in (0,10,266):
    for flags in (0,2,0x802):
        for pool in (0,1,2,3):case(phase=0,command=command,flags=flags,pool=pool)
for value in (0,999,1000,1001,65535):case(phase=1,value=value)
for frame in (0,1,255,256,257,65535):
    for flags in (0,0x802,0x1002,0x1802):case(phase=2,frame=frame,flags=flags)
for media in (64,65):
    for flags in (0,2,0x1902):
        for busy in (0,4):case(phase=3,media=media,flags=flags,busy=busy)
for phase in (4,255):case(phase=phase)
for phase in (0,1,2,3):case(phase=phase,physical=1,command=10,value=1000,frame=1)

def fixture(exe,c):
    r,s,_=reward_fixture(exe,dict(entry=0,xp=99,gain=3,items=2))
    def put(a,b):a&=0x1FFFFF;r[a:a+len(b)]=b;s[a:a+len(b)]=b
    def sw(a,v):put(a,struct.pack('<I',v&0xFFFFFFFF))
    def sh(a,v):put(a,struct.pack('<H',v&65535))
    def sb(a,v):put(a,bytes((v&255,)))
    aya=0 if c.get('physical') else 0x15C000
    if not aya:put(0,bytes(512))
    sw(0x9D254,aya if not aya else aya|0x80000000)
    sb(aya+12,0);sb(aya+15,1);sh(aya+22,c.get('command',10));sh(aya+26,c.get('frame',1));sw(aya+152,0xABCD0100)
    sb(0x9CE74,c['phase']);sh(0x9D2A4,c.get('value',1000));sw(0x9D1A0,c.get('flags',2));sw(0x9D28C,2)
    sw(0x9D304,3);sh(0x9D21C,2);put(0xA7FF0,r[0x142800:0x142810])
    sw(0xB0CD8,c.get('busy',0)|0x18000);sb(0xB0DCA,c.get('media',65));sh(0xB0DC0,-1)
    sw(0x9D190,0);sw(0x915E0,0x80012340);sw(0xBCF88,0xA5A58020)
    for command in (21,24):
        sw(0xB0E98+command*4,0x80142A00+command*4);sb(0x142A02+command*4,8)
    put(0x180000,bytes(0x12000))
    sw(0x942E0,0x8018C000);sw(0x942E4,0x80180000);sw(0x942E8,0x80190000)
    for code in (2,4,0x55):
        sw(0x18C000+code*4,0x8018D000+code*24);sw(0x18D000+code*24+20,0x800D4850)
    for i in range(22):
        p=0x180000+i*0xA0C if i<11 else 0x190000+(i-11)*0x10C
        sb(p,i%7 if c.get('pool') else 0);sb(p+1,(2,4,0x72)[i%3]);sw(p+4,123+i);sw(p+8,0x8015C000)
    if c.get('pool')==2:sb(0x180000,1);sb(0x180001,255)
    if c.get('pool')==3:sb(0x190000,1);sb(0x190001,255)
    for a,n in RANGES:s[a:a+n]=r[a:a+n]
    return r,s,()

SWITCH='''case 0:func_8002B0E8();break;'''
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
    with tempfile.TemporaryDirectory(prefix='pe-victory-oracle-') as directory:
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
                    Path('/tmp/pe-victory-mismatch-original.bin').write_bytes(r);Path('/tmp/pe-victory-mismatch-native.bin').write_bytes(native)
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
            r=bytearray(initial)
            check(r,0,(),('captured current victory phase',),False)
            print('PASS captured current phase',flush=True)
    out=['/* Generated by pe_battle_victory_oracle.py --write-header; original MIPS execution. */']
    for name,rows in (('ranges',RANGES),('common',common),('patches',patches)):
        out.append(f'static const uint32_t VICTORY_{name}[][2]={{')
        out.extend(f'{{0x{a:X}u,0x{b:X}u}},' for a,b in rows);out.append('};')
    out.append('static const struct { unsigned entry,first,end; uint32_t args[4],result; uint64_t hash; } VICTORY_cases[]={')
    for e,a,b,args,result,h in cases:
        params=','.join(f'0x{x:08X}u' for x in args) or '0'
        out.append(f'{{{e},{a},{b},{{{params}}},0x{result:08X}u,UINT64_C(0x{h:016X})}},')
    out.append('};')
    if '--write-header' in sys.argv:
        assert selected
        (ROOT/'pc_port/tests/retail_battle_victory_cases.h').write_text('\n'.join(out)+'\n')
    print('PASS:',len(cases),'original victory cases;',len(all_seen),'instruction PCs; native comparison',compare)

if __name__=='__main__':main()
