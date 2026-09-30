#!/usr/bin/env python3
"""Execute the original level animation, stat interpolation and bar drawing.

Native comparisons cover all RAM below 1FE000 (original stack excluded).
--capture PATH additionally compares the reached level drawing graph in isolation from an actual connected RAM snapshot.
"""
import hashlib
import struct
import subprocess
import sys
import tempfile
from pathlib import Path
from pe_battle_hud_oracle import ROOT,execute
from pe_battle_reward_oracle import fixture as reward_fixture,RANGES
from pe_scripted_exit_oracle import words
RANGES=RANGES+((0,64),)

def fingerprint(r):
    value=0xCBF29CE484222325
    for a,n in RANGES:
        for byte in r[a:a+n]:value=((value^byte)*0x100000001B3)&0xFFFFFFFFFFFFFFFF
    return value

ENTRIES=(0x8005BA78,0x800437B4,0x8006062C,0x80050D20,0x8004FFF8,0x8004BF40,0x80062FEC,0x8004C1E0)
RESULT_ENTRIES={7}
CASES=[]
def case(entry,**kw):CASES.append(dict(entry=entry,**kw))
for key in (-2147483648,-1,0,1,99,100,101,9700,9799,9800,9900,2147483647):
    for outputs in (0,1,2,3,4):case(0,key=key,outputs=outputs)
for fraction in (-1,0,1,2,3,24,45,46,47,48,49):
    for bank in (0,1):case(2,fraction=fraction,bank=bank)
for bar,timer in ((0,0),(20,0),(48,4),(48,2),(48,0),(48,-1),(49,1)):
    for key in (0,50,99,100,9800):case(1,bar=bar,timer=timer,key=key)
for entry in (3,4,5,6):
    for phase in (0,64,126,128,130):
        for bank in (0,1):case(entry,phase=phase,bank=bank)
for timer in (-1,0,29,30,31):
    for done in (0,1):case(5,timer=timer,done=done)
for fraction in (0,2,24,48):
    for space in (0,8,80,160):case(2,fraction=fraction,space=space)


def fixture(exe,c):
    r,s,_=reward_fixture(exe,dict(entry=9,xp=99,gain=302,busy=1,bank=c.get('bank',0)))
    def put(a,b):a&=0x1FFFFF;r[a:a+len(b)]=b;s[a:a+len(b)]=b
    def sw(a,v):put(a,struct.pack('<I',v&0xFFFFFFFF))
    def word(a):return struct.unpack_from('<I',r,a&0x1FFFFF)[0]
    # Seven separate monotone lookup tables with deliberate threshold boundaries.
    sw(0xA8038,0x156000-0xA8028)
    for table in range(7):
        for i in range(128):sw(0x156000+table*512+i*4,i*100)
    for i in range(7):
        sw(0xA18D8+i*4,c.get('key',75));sw(0xA18FC+i*4,30)
        sw(0xA1920+i*4,c.get('bar',0));sw(0xA1940+i*4,c.get('timer',0))
    sw(0x9CF40,c.get('phase',0));sw(0x9CF78,c.get('timer',30));sw(0x9CF7C,c.get('timer',0))
    if c.get('done'):
        sw(0x9CF60,word(0x9CF6C));sw(0x9CF64,word(0x9CF70));sw(0x9CF68,word(0x9CF74))
    if 'space' in c:sw(0x9D100,word(0x9D104)+0x4000-c['space'])
    node=word(0x9D154)
    while not (word(node+32)==2 and word(node+36)==47):node=word(node)
    outputs=c.get('outputs',3)
    a=0x80142980 if outputs in (1,3,4) else 0
    b=0x80142980 if outputs==4 else 0x80142984 if outputs in (2,3) else 0
    args=((1,c.get('key',75),a,b),(1,),(123,c.get('fraction',24)),(0,),(node,),(),())[c['entry']]
    for a,n in RANGES:s[a:a+n]=r[a:a+n]
    return r,s,tuple(a&0xFFFFFFFF for a in args)

SWITCH='''case 0:func_8005BA78((int32_t)a,(int32_t)b,c,d);break;
case 1:func_800437B4(a);break;
case 2:func_8006062C((int32_t)a,(int32_t)b);break;
case 3:func_80050D20(a);break;
case 4:func_8004FFF8(a);break;
case 5:func_8004BF40();break;
case 6:func_80062FEC();break;
case 7:result=func_8004C1E0(a,b);break;
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
    with tempfile.TemporaryDirectory(prefix='pe-level-oracle-') as directory:
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
                    Path('/tmp/pe-level-mismatch-original.bin').write_bytes(r);Path('/tmp/pe-level-mismatch-native.bin').write_bytes(native)
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
            for e,args in ((0,(1,75,0x80142980,0x80142984)),(1,(1,)),(2,(123,24)),(3,(0,)),(4,(find(bytearray(initial),2,47),)),(5,()),(6,())):
                r=bytearray(initial)
                check(r,e,args,('capture draw',e),False);print('PASS captured draw',e,flush=True)
            r=bytearray(initial)
            for frame in range(80):
                # An isolated draw-frame context, never a connected checkpoint.
                for address,value in ((0x9D100,word(r,0x9D104)),(0x9D12C,0x800A2270),(0x9D124,0),(0x9D128,0)):
                    struct.pack_into('<I',r,address,value)
                check(r,6,(),('capture animation frame',frame),False)
            assert word(r,0x9CF80)==0, 'animation must finish'
            check(r,7,(find(r,1,21),0x10000),('capture level commit',),False)
            print('PASS captured 80-frame animation and level commit',
                  'level',r[0xC0E0A],'maxHP',struct.unpack_from('<H',r,0xC0E06)[0],
                  'BP',word(r,0xC0E10),flush=True)
    out=['/* Generated by pe_battle_level_oracle.py --write-header; original MIPS execution. */']
    for name,rows in (('ranges',RANGES),('common',common),('patches',patches)):
        out.append(f'static const uint32_t LEVEL_{name}[][2]={{')
        out.extend(f'{{0x{a:X}u,0x{b:X}u}},' for a,b in rows);out.append('};')
    out.append('static const struct { unsigned entry,first,end; uint32_t args[4],result; uint64_t hash; } LEVEL_cases[]={')
    for e,a,b,args,result,h in cases:
        params=','.join(f'0x{x:08X}u' for x in args) or '0'
        out.append(f'{{{e},{a},{b},{{{params}}},0x{result:08X}u,UINT64_C(0x{h:016X})}},')
    out.append('};')
    if '--write-header' in sys.argv:
        assert selected
        (ROOT/'pc_port/tests/retail_battle_level_cases.h').write_text('\n'.join(out)+'\n')
    print('PASS:',len(cases),'original level cases;',len(all_seen),'instruction PCs; native comparison',compare)

if __name__=='__main__':main()
