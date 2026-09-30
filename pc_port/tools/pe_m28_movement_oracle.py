#!/usr/bin/env python3
"""Compare complete original m28 movement calls and histories with native RAM."""
import argparse
import hashlib
import itertools
import struct
import subprocess
import tempfile
from pathlib import Path
from pe_battle_hud_oracle import ROOT, execute
from pe_btl14_m0005i_publish_oracle import find_disc, read_form1

NATIVE = r'''
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "game_port.h"
#include <stdio.h>
#include <stdlib.h>
void Trace_Direct(const char *s) {(void)s;}
int main(int argc,char **argv) {
 if(argc<4)return 2;
 PE_RamInit();FILE *f=fopen(argv[1],"rb");
 if(!f || fread(PE_Translate(PE_RAM_BASE,PE_RAM_SIZE),1,PE_RAM_SIZE,f)!=PE_RAM_SIZE)return 3;
 fclose(f);unsigned a[6]={0};for(int i=4;i<argc && i<10;i++)a[i-4]=strtoul(argv[i],0,0);
 int result=0;
 switch(atoi(argv[3])) {
 case 0:result=PE_M28MovementInit(a[0]);break;
 case 1:result=PE_M28MovementCommand(a[0],a[1],a[2],a[3],a[4],a[5]);break;
 case 2:result=PE_M28MovementUpdate(a[0]);break;
 case 3:result=PE_M28MovementCleanup(a[0]);break;
 case 4:result=func_8006F39C(a[0],a[1]);break;
 case 5:result=func_8006F6D4(a[0],a[1],a[2],a[3],a[4],a[5]);break;
 case 6:result=func_80069660();break;
 case 7:func_80017018();break;
 case 8:PE_EffectCallback_SetExtra1(a[3]);result=PE_EffectCallback(0x80193148u,a[0],a[1],a[2]);break;
 case 9:result=func_8006FC18(a[0],a[1],a[2]);break;
 }
 if(PE_Port_ShouldStop())return 5;
 f=fopen(argv[2],"wb");if(!f)return 4;
 fwrite(&result,4,1,f);fwrite(PE_Translate(PE_RAM_BASE,PE_RAM_SIZE),1,PE_RAM_SIZE,f);fclose(f);
 return 0;
}
'''
ENTRIES = (0x8019151C,0x801915AC,0x801917FC,0x801920A0,0x8006F39C,0x8006F6D4,0x80069660,0x80017018,0x80193148,0x8006FC18)
SLOT, ACTOR = 0x80160000, 0x80140000
RANGES = ((0x910A0,0x400),(0x942E0,12),(0x966EC,0x4000),(0x9A6EC,0x804),
          (0x9CE00,4),(0x9D1A0,4),(0x9D20C,4),(0x9D254,4),(0x9D2F0,4),(0x9D300,4),
          (0x9DF70,32),(0xA77F0,32),(0xB6A80,32),(0xB0CD8,0x100),
          (0x140000,0x1000),(0x150000,0x17000),(0x170000,0x1000),
          (0x19151C,4),(0x1915AC,4),(0x191894,4),(0x1920D8,4),(0x1931DC,28),(0x193288,12))


def fingerprint(r):
    value=14695981039346656037
    for a,n in RANGES:
        for v in r[a:a+n]:value=((value^v)*1099511628211)&0xFFFFFFFFFFFFFFFF
    return value


def w(r,a,v):
    struct.pack_into('<I',r,a&0x1FFFFF,v&0xFFFFFFFF)


def h(r,a,v):
    struct.pack_into('<H',r,a&0x1FFFFF,v&65535)


def b(r,a,v):
    r[a&0x1FFFFF]=v&255


def u(r,a):
    return struct.unpack_from('<I',r,a&0x1FFFFF)[0]


def fixture(exe,overlay,descriptor=0x801931DC):
    r=bytearray(0x200000);r[0x10000:0x10000+len(exe)-0x800]=exe[0x800:]
    r[0x18EFE8:0x18EFE8+len(overlay)]=overlay
    for a,n in ((0x140000,0x1000),(0x150000,0x17000),(0x170000,0x1000)):
        r[a:a+n]=bytes(n)
    w(r,0x942E0,0x80170000);w(r,0x942E4,0x80150000);w(r,0x942E8,SLOT)
    for code in (0x45,0x46,0x49,0x54):w(r,0x170000+4*code,descriptor)
    w(r,0x9D1A0,0x80);w(r,0xB0CD8,0);b(r,0xB0DC7,0)
    w(r,0x9D254,ACTOR+0x280);w(r,0x9D20C,ACTOR);w(r,0x9D2F0,ACTOR)
    w(r,ACTOR,ACTOR+0x800);w(r,ACTOR+0x818,ACTOR+0x900)
    w(r,ACTOR+4,ACTOR+0x280);b(r,ACTOR+12,8);b(r,ACTOR+14,4)
    b(r,ACTOR+0x28C,0);b(r,ACTOR+0x28D,1)
    for a,pos in ((ACTOR,(-167,1153,-2150)),(ACTOR+0x280,(2,1153,-3034))):
        for i,v in enumerate(pos):w(r,a+0x28+4*i,v<<16)
    b(r,SLOT,1);b(r,SLOT+1,0x49);w(r,SLOT+8,ACTOR)
    h(r,ACTOR+0x16,10);h(r,ACTOR+0x1A,3);b(r,ACTOR+15,3)
    return r


def verify_m32_relocation(old_overlay,new_overlay):
    start,end=0x80191514,0x80192104
    references={0x80191550,0x80191714,0x80191758,0x80191884,0x80191C24}
    jumps=refs=0
    for pc in range(start,end,4):
        old=struct.unpack_from('<I',old_overlay,pc-0x8018EFE8)[0]
        new=struct.unpack_from('<I',new_overlay,pc-16-0x8018EFE8)[0]
        if old==new:continue
        if old>>26 in (2,3):
            target=0x80000000|((old&0x3FFFFFF)<<2)
            assert start<=target<end
            assert new==(old&0xFC000000)|(((target-16)>>2)&0x3FFFFFF)
            jumps+=1
        else:
            assert pc in references and new==old-16
            refs+=1
    assert jumps==21 and refs==5
    print('PASS: complete764-word M28/M32 relocation:21internal jumps and5callback references',flush=True)


def main():
    global RANGES
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build-dir',type=Path,default=ROOT/'pc_port/build')
    parser.add_argument('--write-header',action='store_true')
    parser.add_argument('--room',choices=('m0028i','m0032i'),default='m0028i')
    args=parser.parse_args()
    exe=(ROOT/'build/disc1.candidate.exe').read_bytes()
    assert hashlib.sha1(exe).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
    overlay=read_form1(find_disc(ROOT),15016,69)
    assert hashlib.sha256(overlay).hexdigest()=='c15d03313a578f7c7ba07f255345df8e17734e38dd807162426d0bc3a7fd7e94'
    relocation=0;descriptor=0x801931DC;namespace='m28_movement';entries=ENTRIES;native_source=NATIVE
    if args.room=='m0032i':
        original_overlay=overlay
        overlay=read_form1(find_disc(ROOT),15937,48)
        assert hashlib.sha256(overlay).hexdigest()=='cad2f1feb66352c54806de4291c6a7fb28fd610039860a6631aebc93047fffa8'
        verify_m32_relocation(original_overlay,overlay)
        relocation=-16;descriptor=0x80192558;namespace='m32_movement'
        entries=tuple(e+relocation if i<4 else e for i,e in enumerate(ENTRIES))
        for name in ('Init','Command','Update'):
            native_source=native_source.replace('PE_M28Movement'+name,'PE_M32Movement'+name)
        moved={0x19151C:0x19150C,0x1915AC:0x19159C,0x191894:0x191884,
               0x1920D8:0x1920C8,0x1931DC:0x192558}
        RANGES=tuple((moved.get(a,a),n) for a,n in RANGES if a!=0x193288)
    make_fixture=lambda:fixture(exe,overlay,descriptor)
    total=0
    common=None;patches=[];rows=[]
    with tempfile.TemporaryDirectory(prefix='pe-m28-movement-') as directory:
        temp=Path(directory);source=temp/'native.c';binary=temp/'native';inp=temp/'in.bin';out=temp/'out.bin'
        source.write_text(native_source)
        subprocess.run(['cc','-O2',*[f'-I{ROOT}/pc_port/{p}' for p in ('include','platform','bootstrap','src','.')],
                        str(source),str(args.build_dir.resolve()/'libpe_field_runtime.a'),'-o',str(binary)],check=True)

        def compare(r,kind,params,label):
            nonlocal total,common
            if args.write_header:
                words={a+i:u(r,a+i) for a,n in RANGES for i in range(0,n,4)}
                if common is None:common=words
                first=len(patches);patches.extend((a,v) for a,v in words.items() if v!=common[a])
            inp.write_bytes(r)
            subprocess.run([str(binary),str(inp),str(out),str(kind),*[hex(x&0xFFFFFFFF) for x in params]],check=True)
            data=out.read_bytes();result=struct.unpack_from('<I',data)[0];native=bytearray(data[4:])
            initial=bytes(r);seen=set()
            regs=execute(r,entries[kind],params,visited_pcs=seen,instruction_budget=1000000,scratchpad=bytearray(0x400))
            cursor=0
            for start,size in sorted(RANGES):
                assert r[cursor:start]==initial[cursor:start],('uncovered original write',label,hex(cursor),hex(start))
                cursor=start+size
            assert r[cursor:0x1FE000]==initial[cursor:0x1FE000],('uncovered original write',label,hex(cursor))
            for pc in seen | {pc+4 for pc in seen}:
                offset=pc-0x8018EFE8
                original=overlay[offset:offset+4] if 0<=offset<len(overlay)-3 else exe[pc-0x80010000+0x800:pc-0x80010000+0x804]
                assert initial[pc&0x1FFFFF:(pc&0x1FFFFF)+4]==original,('instruction source',hex(pc))
            # The VM compatibility frame has host-selected guest storage.
            if kind==7:
                native[0x120F80:0x120FC0]=r[0x120F80:0x120FC0]
            if native[:0x1FE000]!=r[:0x1FE000] or (kind!=7 and result!=regs[2]):
                diffs=[(hex(i),r[i],native[i]) for i in range(0x1FE000) if r[i]!=native[i]]
                Path('/tmp/pe-movement-fail-original.bin').write_bytes(r)
                Path('/tmp/pe-movement-fail-native.bin').write_bytes(native)
                Path('/tmp/pe-movement-fail-input.bin').write_bytes(initial)
                raise AssertionError((label,kind,params,hex(regs[2]),hex(result),diffs[:30]))
            total+=1
            if args.write_header:rows.append((kind,params,first,len(patches),result,fingerprint(r)))
            return r

        base=make_fixture()
        compare(base,0,(SLOT,),'init')
        for mode,value in (() if relocation else itertools.product((0,1,2,0xFFFFFFFF),(0,0x80000000,0xFFFFFFFF))):
            compare(bytearray(base),8,(mode,value,value^0xDEADBEEF,value^0x12345678),'room command callback')
        for state,owner,force,count in itertools.product(range(7),(ACTOR,ACTOR+0x280),(0,1),(0,1)):
            r=bytearray(base);b(r,SLOT,state);h(r,SLOT+0x14,count);w(r,SLOT+0x10,0x80140F00)
            w(r,0x140F00,1)
            compare(r,9,(11,owner,force),f'destroy {state}/{owner}/{force}/{count}')
        for state,mode,cmd in itertools.product((0,1,2,3),(0,1,2),range(28)):
            r=bytearray(base);b(r,SLOT+3,state)
            a=0x80140F00 if (cmd==19 and mode or cmd==25) else 8
            compare(r,1,(SLOT,mode,cmd,a,0,64),f'command {state}/{mode}/{cmd}')
        for code,index in itertools.product((0x45,0x46,0x49,0x54),range(12)):
            r=make_fixture();r[0x150000:0x170000]=bytes(0x20000)
            pool,stride=(0x150000,0xA0C) if code==0x45 else (0x160000,0x10C)
            for i in range(index):b(r,pool+i*stride,1)
            compare(r,4,(code,ACTOR),f'allocation {code}/{index}')
        for flags in (0,0x80,0x84,0x180,0x184):
            r=bytearray(base);w(r,0x9D1A0,flags)
            compare(r,6,(),f'pool pump {flags}')
        for mode,cmd in itertools.product((0,1,2),(0,19,25,26)):
            r=bytearray(base)
            compare(r,5,(11,mode,cmd,0x80140F00,0x80140F04,0x80140F08),'event dispatcher')
        for mode in range(5):
            r=bytearray(base);w(r,0x9D300,0x80140C00);w(r,0x140C00,0x80140D00)
            w(r,0x140C08,0x80);w(r,0x140C10,1)
            word=0x6C|(5<<13)
            for i,v in enumerate((11,25,0x12345678,0x23456789,0x3456789A)):
                word|=mode<<(17+3*i)
                addr=0x140D08+4*i if mode==0 else (0x1400AC,0xA77F0,0x9DF70,0xB6A80)[mode-1]+4*i
                w(r,0x140D08+4*i,i);w(r,addr,v)
            w(r,0x140D00,word);w(r,0x140D1C,2|(1<<13));w(r,0x140D20,0);w(r,0x140D24,3)
            compare(r,7,(),f'opcode 6C mode{mode}')
        for variant in range(24):
            r=bytearray(base)
            compare(r,1,(SLOT,0,17,0x200000,0xFFFFFFFF,0xF6000000),'target')
            compare(r,1,(SLOT,0,25,0x80140F00,0,0),'query in non-query mode')
            compare(r,1,(SLOT,1,25,0x80140F00,0,0),'query')
            if variant&1:compare(r,1,(SLOT,0,16,1,0,0),'pause at apex')
            if variant&2:compare(r,1,(SLOT,0,6,0x18000,0,-0x21000),'target offset')
            if variant&4:compare(r,1,(SLOT,0,0,0,1,0),'follow actor')
            if variant&8:compare(r,1,(SLOT,0,22,0x30000,0xF7000000,64),'turn')
            if variant&16:compare(r,1,(SLOT,0,10,4,3,0),'animation gate')
            for frame in range(42):
                if frame==4 and variant%3==0:w(r,ACTOR+0x98,u(r,ACTOR+0x98)|0x80000)
                if frame==12 and variant&1:compare(r,1,(SLOT,0,18,0x18000,0,0),'resume from apex')
                if u(r,SLOT+12)==0x80191D10+relocation:continue
                compare(r,2,(SLOT,),f'history {variant}/{frame}')
                if r[(SLOT+3)&0x1FFFFF]==0:break
        print('PASS:',total,'original/native movement calls and history frames',flush=True)
        if args.write_header:
            lines=['/* Generated original movement effect cases; do not edit. */']
            for name,data in (('ranges',RANGES),('common',[(a,v) for a,v in common.items() if v]),('patches',patches)):
                lines.append(f'static const uint32_t {namespace}_{name}[][2]={{')
                lines.extend(f' {{0x{a:X}u,0x{v:X}u}},' for a,v in data);lines.append('};')
            lines.append('static const struct { unsigned kind,first,end; uint32_t args[6],result; uint64_t hash; } '+namespace+'_cases[]={')
            for kind,params,first,end,result,hash_value in rows:
                values=','.join(f'0x{x&0xFFFFFFFF:X}u' for x in list(params)+[0]*(6-len(params)))
                lines.append(f' {{{kind},{first},{end},{{{values}}},0x{result:X}u,UINT64_C(0x{hash_value:X})}},')
            lines.append('};')
            (ROOT/f'pc_port/tests/retail_{namespace}_cases.h').write_text('\n'.join(lines)+'\n')


if __name__=='__main__':main()
