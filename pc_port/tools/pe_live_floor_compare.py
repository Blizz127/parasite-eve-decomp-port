#!/usr/bin/env python3
"""Compare native 1AE40 with original instructions on local room captures.

Captures are diagnostic inputs, never replayed into the connected route.
Exercises eight five-unit steps with cached-wall state retained or cleared.
Compares RAM below 0x1FE000, excluding the oracle's scratch stack, and checks
that every executed instruction came from the SHA-1-verified retail EXE.
Does not establish rendering, timing, GTE-state or whole-route fidelity.
"""
import argparse
import hashlib
import itertools
from pathlib import Path
import struct
import subprocess
import tempfile
from pe_battle_hud_oracle import ROOT, execute

NATIVE = r'''
#include "psx_compat.h"
#include "pe_port_compat.h"
#include <stdio.h>
void Trace_Direct(const char *s) {(void)s;}
int main(int argc,char **argv) {
    if (argc!=3) return 2;
    PE_RamInit();
    FILE *f=fopen(argv[1],"rb");
    if (!f) return 3;
    if (fread(PE_Translate(PE_RAM_BASE,PE_RAM_SIZE),1,PE_RAM_SIZE,f)!=PE_RAM_SIZE) return 4;
    fclose(f);
    func_8001AE40(PE_LoadU32(0x8009D254u));
    f=fopen(argv[2],"wb");
    if (!f) return 5;
    if (fwrite(PE_Translate(PE_RAM_BASE,PE_RAM_SIZE),1,PE_RAM_SIZE,f)!=PE_RAM_SIZE) return 6;
    return fclose(f)!=0;
}
'''
DIRECTIONS=((5,0),(-5,0),(0,5),(0,-5),(5,5),(-5,5),(5,-5),(-5,-5))


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('captures',type=Path,nargs='+')
    parser.add_argument('--build-dir',type=Path,default=ROOT/'pc_port/build')
    parser.add_argument('--cc',default='cc')
    args=parser.parse_args()
    exe=(ROOT/'build/extracted/disc1/SLUS_006.62').read_bytes()
    assert hashlib.sha1(exe).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
    total=0
    with tempfile.TemporaryDirectory(prefix='pe-live-floor-') as directory:
        temp=Path(directory)
        source=temp/'native.c';source.write_text(NATIVE)
        binary=temp/'native';input_path=temp/'input.bin';output_path=temp/'output.bin'
        include=[str(ROOT/'pc_port'/p) for p in ('include','platform','bootstrap','src','.')]
        subprocess.run([args.cc,'-O2',*[f'-I{p}' for p in include],str(source),
                        str(args.build_dir.resolve()/'libpe_field_runtime.a'),'-o',str(binary)],check=True)
        for capture in args.captures:
            original=capture.read_bytes()
            assert len(original)==0x200000,f'{capture}: expected 2 MiB capture'
            print(f'capture {capture}: SHA256 {hashlib.sha256(original).hexdigest()}',flush=True)
            for (dx,dz),cached in itertools.product(DIRECTIONS,(False,True)):
                ram=bytearray(original)
                def word(a):return struct.unpack_from('<I',ram,a&0x1FFFFF)[0]
                def store(a,v):struct.pack_into('<I',ram,a&0x1FFFFF,v&0xFFFFFFFF)
                actor=word(0x9D254)
                assert 0x80000000<=actor<=0x801FFD80,f'{capture}: invalid Aya pointer'
                for new,old in ((0x28,0x40),(0x2C,0x44),(0x30,0x48)):
                    store(actor+old,word(actor+new))
                store(actor+0x28,word(actor+0x28)+dx*65536)
                store(actor+0x30,word(actor+0x30)+dz*65536)
                if not cached:store(0x9D2E8,word(0x9D2E8)&~8)
                initial=bytes(ram);input_path.write_bytes(initial)
                subprocess.run([str(binary),str(input_path),str(output_path)],check=True)
                native=output_path.read_bytes()
                visited=set()
                execute(ram,0x8001AE40,(actor,),instruction_budget=1000000,visited_pcs=visited)
                # The runner's PC set omits branch delay slots. Include each
                # successor word so those original instructions are checked.
                for pc in visited | {pc+4 for pc in visited}:
                    off=(pc&0x1FFFFF)-0x10000+0x800
                    assert 0x800<=off<=len(exe)-4 and initial[pc&0x1FFFFF:(pc&0x1FFFFF)+4]==exe[off:off+4],f'non-retail instruction at {pc:08X}'
                if native[:0x1FE000]!=ram[:0x1FE000]:
                    differences=[f'{i:06X}:{native[i]:02X}/{ram[i]:02X}' for i in range(0x1FE000) if native[i]!=ram[i]][:20]
                    raise AssertionError(f'{capture} delta=({dx},{dz}) cached={cached}: native/original {differences}')
                total+=1
    print(f'PASS: {total} captured-state native/original floor cases; RAM below 0x1FE000 identical')


if __name__=='__main__':main()
