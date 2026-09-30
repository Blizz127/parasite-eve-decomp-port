#!/usr/bin/env python3
"""Compare native model ticks with retail instructions on local RAM captures.

Follows each selected model through 65 ticks with alternating packet banks.
Calls the field loop's lighting setup before each tick. Compares all RAM below
0x1FE000; scratch stack, scratchpad, GTE state and actual frame timing are not
claimed identical. Captures are diagnostic inputs, never route save states.
"""
import argparse,hashlib,struct,subprocess,tempfile
from pathlib import Path
from pe_battle_hud_oracle import ROOT,execute
NATIVE = r'''

#include "psx_compat.h"
#include "pe_port_compat.h"
#include <stdio.h>
#include <stdlib.h>
void Trace_Direct(const char *s) {(void)s;}
int main(int argc,char **argv) {
 if (argc!=4) return 2;
 PE_RamInit(); FILE *f=fopen(argv[1],"rb");
 if (!f || fread(PE_Translate(PE_RAM_BASE,PE_RAM_SIZE),1,PE_RAM_SIZE,f)!=PE_RAM_SIZE) return 3;
 fclose(f);
 unsigned dest=strtoul(argv[3],0,0);
 func_8006698C(dest);func_8003AF14(dest,0x800B89F8u);
 f=fopen(argv[2],"wb");
 if (!f || fwrite(PE_Translate(PE_RAM_BASE,PE_RAM_SIZE),1,PE_RAM_SIZE,f)!=PE_RAM_SIZE) return 4;
 fclose(f);
 return 0;
}
'''

def main():
 parser=argparse.ArgumentParser(description=__doc__)
 parser.add_argument('capture',type=Path)
 parser.add_argument('--actor',type=lambda s:int(s,0),action='append',required=True)
 parser.add_argument('--build-dir',type=Path,default=ROOT/'pc_port/build')
 parser.add_argument('--frames',type=int,default=65)
 args=parser.parse_args()
 exe=(ROOT/'build/disc1.candidate.exe').read_bytes()
 assert hashlib.sha1(exe).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
 capture=args.capture.read_bytes();assert len(capture)==0x200000
 print('capture SHA256',hashlib.sha256(capture).hexdigest(),flush=True)
 total=0
 with tempfile.TemporaryDirectory(prefix='pe-live-model-') as directory:
  temp=Path(directory);source=temp/'native.c';binary=temp/'native';inp=temp/'in.bin';out=temp/'out.bin'
  source.write_text(NATIVE)
  includes=[f'-I{ROOT}/pc_port/{p}' for p in ('include','platform','bootstrap','src','.')]
  subprocess.run(['cc','-O2',*includes,str(source),str(args.build_dir.resolve()/'libpe_field_runtime.a'),'-o',str(binary)],check=True)
  for actor in args.actor:
   assert 0x80000000<=actor<0x801FD000
   d=actor+0x1B4;r=bytearray(capture)
   for frame in range(args.frames):
    struct.pack_into('<I',r,0x9CDDC,frame&1);inp.write_bytes(r)
    subprocess.run([str(binary),str(inp),str(out),hex(d)],check=True)
    native=out.read_bytes();initial=bytes(r);visited=set();gte={};scratch=bytearray(0x400)
    execute(r,0x8006698C,(d,),final_gte=gte,visited_pcs=visited,scratchpad=scratch)
    execute(r,0x8003AF14,(d,0x800B89F8),initial_cop_control=dict(enumerate(gte['control'])),
      initial_cop_data=dict(enumerate(gte['data'])),instruction_budget=1000000,visited_pcs=visited,scratchpad=scratch)
    for pc in visited|{pc+4 for pc in visited}:
     p=pc&0x1FFFFF;off=p-0x10000+0x800
     assert 0x800<=off<len(exe)-3 and initial[p:p+4]==exe[off:off+4],f'non-retail instruction {pc:08X}'
    if native[:0x1FE000]!=r[:0x1FE000]:
     diffs=[f'{i:06X}:{native[i]:02X}/{r[i]:02X}' for i in range(0x1FE000) if native[i]!=r[i]][:20]
     raise AssertionError(f'actor{actor:08X} frame{frame}: {diffs}')
    total+=1
   print(f'PASS actor{actor:08X}: {args.frames} original/native ticks',flush=True)
 print(f'PASS: {total} captured model ticks; RAM below 0x1FE000 identical')
if __name__=='__main__':main()
