#!/usr/bin/env python3
"""Check retail actor retirement and parent poses against native RAM.

Exercises list removal, parent retirement, Aya, model allocations and both
owned-effect pools, plus ordered and overlapping parent pose copies. All RAM below 1FE000 is compared; the original execution
stack is excluded. Executed instructions and delay slots must be retail bytes.
Captures are diagnostic inputs only and never injected into a connected route.
"""
import argparse
import hashlib
import itertools
from pathlib import Path
import struct
import subprocess
import tempfile
from pe_battle_hud_oracle import ROOT, execute

RANGES=((0,0x100),(0x942E0,12),(0x9D20C,4),(0x9D254,4),(0x9D2A4,12),
        (0x9D2E8,4),(0xA7624,128),(0x150000,0x10000))
ACTORS=tuple(0x80150000+i*0x280 for i in range(4))
NATIVE=r'''
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "game_port.h"
#include <stdio.h>
#include <stdlib.h>
void Trace_Direct(const char *s) {(void)s;}
int main(int argc,char **argv) {
 if(argc!=4)return 2;
 PE_RamInit();FILE *f=fopen(argv[1],"rb");
 if(!f || fread(PE_Translate(PE_RAM_BASE,PE_RAM_SIZE),1,PE_RAM_SIZE,f)!=PE_RAM_SIZE)return 3;
 fclose(f);if(atoi(argv[3])==2)func_8003601C();
 else if(atoi(argv[3]))func_80036448();else func_800360B4();
 if(PE_Port_ShouldStop())return 4;
 f=fopen(argv[2],"wb");if(!f)return 5;
 fwrite(PE_Translate(PE_RAM_BASE,PE_RAM_SIZE),1,PE_RAM_SIZE,f);return fclose(f)!=0;
}
'''

def w(r,a,v):struct.pack_into('<I',r,a&0x1FFFFF,v&0xFFFFFFFF)
def h(r,a,v):struct.pack_into('<H',r,a&0x1FFFFF,v&65535)
def u(r,a):return struct.unpack_from('<I',r,a&0x1FFFFF)[0]
def fingerprint(r):
    value=14695981039346656037
    for a,n in RANGES:
        for v in r[a:a+n]:value=((value^v)*1099511628211)&0xFFFFFFFFFFFFFFFF
    return value

def fixture(exe,count=4,mask=0,parent=0,aya=0,model=1,effect=0):
    r=bytearray(0x200000);r[0x10000:0x10000+len(exe)-0x800]=exe[0x800:]
    for a,n in RANGES:r[a:a+n]=bytes(n)
    w(r,0x9D20C,ACTORS[0] if count else 0);w(r,0x9D254,ACTORS[aya])
    w(r,0x9D2AC,0x80150F00);h(r,0x9D2A6,count);w(r,0x9D2E8,0xA5A5FFFF)
    for i,a in enumerate(ACTORS):
        w(r,a+4,ACTORS[i+1] if i+1<count else 0);w(r,a+8,ACTORS[i-1] if i else 0)
        w(r,a+0x98,0x10800000|(16 if mask&(1<<i) else 0))
        if parent and i: w(r,a+0x18C,ACTORS[0 if parent==1 else i-1])
        w(r,a+0x1AC,0x80153000 if model else 0);w(r,a+0x278,0x80154000+i*32)
        for j in (i,15-i):w(r,0xA7624+j*8,0x80154000+i*32);w(r,0xA7628+j*8,0xABCDEF00+j)
    w(r,0x942E0,0x80151000);w(r,0x942E4,0x80156000);w(r,0x942E8,0x8015E000)
    w(r,0x151000+4*8,0x80151200);w(r,0x151214,0x800C7DC4)
    if effect:
        for i in range(22):
            p=0x156000+i*0xA0C if i<11 else 0x15E000+(i-11)*0x10C
            r[p:p+4]=bytes((effect-1,8,3,9));w(r,p+8,ACTORS[i%4]);w(r,p+4,0xAABBCCDD)
    return r

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build-dir',type=Path,default=ROOT/'pc_port/build')
    parser.add_argument('--write-header',action='store_true')
    parser.add_argument('--capture',type=Path,action='append',default=[])
    args=parser.parse_args()
    exe=(ROOT/'build/extracted/disc1/SLUS_006.62').read_bytes()
    assert hashlib.sha1(exe).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
    common=None;patches=[];rows=[];total=0
    with tempfile.TemporaryDirectory(prefix='pe-retire-') as d:
        temp=Path(d);source=temp/'native.c';binary=temp/'native';inp=temp/'in.bin';out=temp/'out.bin'
        source.write_text(NATIVE)
        subprocess.run(['cc','-O2',*[f'-I{ROOT}/pc_port/{p}' for p in ('include','platform','bootstrap','src','.')],
                        str(source),str(args.build_dir.resolve()/'libpe_field_runtime.a'),'-o',str(binary)],check=True)
        def compare(r,label,generate=True,contact=False,parent=False):
            nonlocal common,total
            kind=2 if parent else int(contact)
            initial=bytes(r);inp.write_bytes(r)
            subprocess.run([str(binary),str(inp),str(out),str(kind)],check=True)
            native=out.read_bytes();seen=set()
            execute(r,(0x800360B4,0x80036448,0x8003601C)[kind],(),instruction_budget=1000000,visited_pcs=seen,scratchpad=bytearray(0x400))
            if generate:
                cursor=0
                for a,n in RANGES:
                    assert r[cursor:a]==initial[cursor:a],('uncovered write',label,hex(cursor),hex(a))
                    cursor=a+n
                assert r[cursor:0x1FE000]==initial[cursor:0x1FE000],('uncovered write',label)
            for pc in seen|{pc+4 for pc in seen}:
                offset=pc-0x80010000+0x800
                assert 0x800<=offset<len(exe)-3 and initial[pc&0x1FFFFF:(pc&0x1FFFFF)+4]==exe[offset:offset+4],('instruction',hex(pc))
            if native[:0x1FE000]!=r[:0x1FE000]:
                differences=[(hex(i),native[i],r[i]) for i in range(0x1FE000) if native[i]!=r[i]][:30]
                raise AssertionError((label,differences))
            if args.write_header and generate:
                words={a+i:u(initial,a+i) for a,n in RANGES for i in range(0,n,4)}
                if common is None:common=words
                first=len(patches);patches.extend((a,v) for a,v in words.items() if v!=common[a])
                rows.append((kind,first,len(patches),fingerprint(r)))
            total+=1
        for count in range(5):
            for mask in range(1<<count):compare(fixture(exe,count=count,mask=mask),('list',count,mask))
        for mask,parent,aya,model in itertools.product(range(16),range(3),range(4),range(2)):
            compare(fixture(exe,mask=mask,parent=parent,aya=aya,model=model),('parent/Aya/model',mask,parent,aya,model))
        for mask,effect in itertools.product((0,1,2,4,8,15),range(1,8)):
            compare(fixture(exe,mask=mask,effect=effect),('effects',mask,effect))
        r=fixture(exe,count=1,mask=1);h(r,0x9D2A6,0);compare(r,'count underflow')
        for large,small in itertools.product((0,0x156000,0x80156000),(0,0x15E000,0x8015E000)):
            r=fixture(exe,mask=15);w(r,0x942E4,large);w(r,0x942E8,small)
            compare(r,('physical/uninitialized pool',large,small))
        for count,flags,relation in itertools.product(range(5),(0,0x400000,0xC00010),range(4)):
            r=fixture(exe,count=count)
            for i,a in enumerate(ACTORS):
                w(r,a+0x98,flags)
                owner=(0,ACTORS[i],ACTORS[(i-1)%4],ACTORS[(i+1)%4])[relation]
                w(r,a+0x18C,owner)
                for j in range(3):w(r,a+40+4*j,(i+1)*0x12345678+j*0x7FFFFFFF)
                for j in range(3):h(r,a+56+2*j,i*17009+j*32767)
            compare(r,('parent pose',count,flags,relation),parent=True)
        for offset in (-12,-8,-4,4,8,12):
            r=fixture(exe,count=1);a=ACTORS[0]
            w(r,a+0x98,0x400000);w(r,a+0x18C,a+offset)
            for i in range(12):w(r,a+28+4*i,i*0x23456789)
            compare(r,('overlapping parent pose',offset),parent=True)
        for capture in args.capture:
            r=bytearray(capture.read_bytes());assert len(r)==0x200000
            print('capture',capture,'SHA256',hashlib.sha256(r).hexdigest(),flush=True)
            initial=bytes(r)
            compare(r,str(capture),generate=False)
            retired=bytes(r)
            for dx,dz in ((5,0),(-5,0),(0,5),(0,-5),(5,5),(-5,5),(5,-5),(-5,-5)):
                for state,base in (('before',initial),('after',retired)):
                    r=bytearray(base);aya=u(r,0x9D254)
                    assert 0x80000000<=aya<=0x801FFD80
                    x,z=u(r,aya+40),u(r,aya+48)
                    for new,old in ((40,64),(44,68),(48,72)):w(r,aya+old,u(r,aya+new))
                    w(r,aya+40,x+dx*65536);w(r,aya+48,z+dz*65536)
                    compare(r,('captured contact',state,dx,dz),generate=False,contact=True)
                    signed=lambda v: (v&0x7FFFFFFF)-(v&0x80000000)
                    print('contact',state,'requested',dx,dz,'actual',
                          signed((u(r,aya+40)-x)&0xFFFFFFFF)/65536,
                          signed((u(r,aya+48)-z)&0xFFFFFFFF)/65536,flush=True)
    if args.write_header:
        lines=['/* Generated by pe_actor_retirement_oracle.py; original MIPS execution. */']
        for name,data in (('ranges',RANGES),('common',[(a,v) for a,v in common.items() if v]),('patches',patches)):
            lines.append(f'static const uint32_t retirement_{name}[][2]={{')
            lines.extend(f' {{0x{a:X}u,0x{v:X}u}},' for a,v in data);lines.append('};')
        lines.append('static const struct { unsigned kind,first,end; uint64_t hash; } retirement_cases[]={')
        lines.extend(f' {{{kind},{a},{b},UINT64_C(0x{v:X})}},' for kind,a,b,v in rows);lines.append('};')
        (ROOT/'pc_port/tests/retail_actor_retirement_cases.h').write_text('\n'.join(lines)+'\n')
    print('PASS:',total,'original/native actor retirement cases',flush=True)

if __name__=='__main__':main()
