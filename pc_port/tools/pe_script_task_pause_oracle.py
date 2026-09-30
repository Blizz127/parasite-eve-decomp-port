#!/usr/bin/env python3
"""Compare original task suspension/resumption leaves and full VM dispatch.

Captures supply isolated call contexts only, never connected route state.
All RAM below 1FE000 is compared; only the original stack is excluded.
"""
import argparse
import hashlib
import itertools
from pathlib import Path
import struct
import subprocess
import tempfile
from pe_battle_hud_oracle import ROOT, execute

RANGES=((0,0x200),(0x910A0,0x400),(0x9CE00,4),(0x9D1A0,4),
        (0x9D254,4),(0x9D2F0,0x14),(0x120F80,64),(0x140000,0x4000))
ENTRIES=(0x80018300,0x80018364,0x80017018)
NATIVE=r'''
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "game_port.h"
#include <stdio.h>
#include <stdlib.h>
void Trace_Direct(const char *s) {(void)s;}
int main(int argc,char **argv) {
 if(argc!=4)return 2;PE_RamInit();FILE *f=fopen(argv[1],"rb");
 if(!f || fread(PE_Translate(PE_RAM_BASE,PE_RAM_SIZE),1,PE_RAM_SIZE,f)!=PE_RAM_SIZE)return 3;
 fclose(f);D_8009D1A0=PE_LoadU32(0x8009D1A0u);int result=0;
 switch(atoi(argv[3])) {
 case 0:result=func_80018300(0);break;
 case 1:result=func_80018364(0);break;
 case 2:func_80017018();break;
 }
 if(PE_Port_ShouldStop())return 4;
 f=fopen(argv[2],"wb");if(!f)return 5;
 size_t n=fwrite(PE_Translate(PE_RAM_BASE,PE_RAM_SIZE),1,PE_RAM_SIZE,f);
 if(fclose(f)||n!=PE_RAM_SIZE)return 6;printf("%d\n",result);return 0;
}
'''

def w(r,a,v):struct.pack_into('<I',r,a&0x1FFFFF,v&0xFFFFFFFF)
def h(r,a,v):struct.pack_into('<H',r,a&0x1FFFFF,v&65535)
def u(r,a):return struct.unpack_from('<I',r,a&0x1FFFFF)[0]
def fingerprint(r):
 value=14695981039346656037
 for a,n in RANGES:
  for b in r[a:a+n]:value=((value^b)*1099511628211)&0xFFFFFFFFFFFFFFFF
 return value

def fixture(exe,entry,layout,flags,saved,current,physical=False,vm_op=0):
 r=bytearray(0x200000);r[0x10000:0x10000+len(exe)-0x800]=exe[0x800:]
 for a,n in RANGES:
  if a!=0x910A0:r[a:a+n]=bytes(n)
 actor=0 if physical else 0x80140000
 w(r,0x9D2F0,actor);w(r,0x9D254,actor)
 nodes=[0x80141000+i*0x40 for i in range(9)]
 for chain,count in enumerate(((0,0,0),(1,0,0),(0,2,0),(0,0,3),(3,3,3),(2,1,3))[layout]):
  w(r,actor+0xA0+chain*4,nodes[chain*3] if count else 0)
  for j in range(count):
   p=nodes[chain*3+j]
   w(r,p,0x80143000);w(r,p+4,0x80143200+j*8 if saved else 0)
   h(r,p+8,flags);h(r,p+10,0xC000+j);w(r,p+16,17+j)
   w(r,p+0x24,nodes[chain*3+j+1] if j+1<count else 0)
 w(r,0x9D300,nodes[current] if current>=0 else 0)
 if entry==2:
  task=nodes[current];w(r,task,0x80143000);w(r,task+0x24,0)
  h(r,task+8,0x80);w(r,task+16,1)
  w(r,0x143000,0x60 if vm_op else 0x5F);w(r,0x143004,0)
  w(r,0x143008,0x2002);w(r,0x14300C,0);w(r,0x143010,3)
  # Native VM's host argument bank is implementation storage. Preseed its
  # final value so full RAM comparison includes it without an exclusion.
  w(r,0x120F80,0x80143010)
 return r

def main():
 parser=argparse.ArgumentParser(description=__doc__)
 parser.add_argument('--build-dir',type=Path,default=ROOT/'pc_port/build')
 parser.add_argument('--capture',type=Path)
 parser.add_argument('--write-header',action='store_true')
 args=parser.parse_args()
 exe=(ROOT/'build/extracted/disc1/SLUS_006.62').read_bytes()
 assert hashlib.sha1(exe).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
 print('original58 words SHA256',hashlib.sha256(exe[0x8B00:0x8BE8]).hexdigest(),flush=True)
 covered=bytearray(0x1FE000)
 for a,n in RANGES:covered[a:a+n]=bytes([1])*n
 seen_all=set();rows=[];base=None;patches=[]
 with tempfile.TemporaryDirectory(prefix='pe-script-pause-') as directory:
  temp=Path(directory);source=temp/'native.c';source.write_text(NATIVE)
  binary=temp/'native';inp=temp/'in.bin';out=temp/'out.bin'
  subprocess.run(['cc','-O2',*[f'-I{ROOT}/pc_port/{p}' for p in ('include','platform','bootstrap','src','.')],str(source),str(args.build_dir.resolve()/'libpe_field_runtime.a'),'-o',str(binary)],check=True)
  def check(r,entry,label,ranges=True):
   initial=bytes(r);seen=set()
   regs=execute(r,ENTRIES[entry],(0,),visited_pcs=seen,instruction_budget=100000)
   for pc in seen|{p+4 for p in seen}:
    off=pc-0x8000F800
    assert 0x800<=off<len(exe)-3 and initial[pc&0x1FFFFF:(pc&0x1FFFFF)+4]==exe[off:off+4],('instruction',hex(pc))
   seen_all.update(seen)
   if ranges:assert all(covered[i] or r[i]==initial[i] for i in range(0x1FE000)),('uncovered original write',label)
   inp.write_bytes(initial);got=int(subprocess.check_output([str(binary),str(inp),str(out),str(entry)]))
   result=regs[2] if entry<2 else 0
   assert got==result,('return',label,got,result)
   native=out.read_bytes()
   assert len(native)==0x200000
   if native[:0x1FE000]!=r[:0x1FE000]:
    raise AssertionError((label,[(hex(i),native[i],r[i]) for i in range(0x1FE000) if native[i]!=r[i]][:20]))
   return result
  cases=[]
  for entry,layout,flags,saved,current in itertools.product(range(2),range(6),(0,0x20,0x40,0x60,0xFFFF),(0,1),(-1,0,4,8)):
   cases.append((entry,layout,flags,saved,current,False,0))
  for entry in range(2):cases.append((entry,4,0xFFFF,1,4,True,0))
  for op,flags,saved,current in itertools.product(range(2),(0,0x40,0x60),(0,1),(0,4,8)):
   cases.append((2,4,flags,saved,current,False,op))
  for k,case in enumerate(cases):
   r=fixture(exe,*case);initial={a+i:u(r,a+i) for a,n in RANGES for i in range(0,n,4)}
   if base is None:base=initial
   first=len(patches);patches.extend((a,v) for a,v in initial.items() if v!=base[a])
   result=check(r,case[0],case);rows.append((case[0],first,len(patches),result,fingerprint(r)))
   if k%100==0:print('cases checked',k+1,flush=True)
  if args.capture:
   data=args.capture.read_bytes();assert len(data)==0x200000
   print('capture',hashlib.sha256(data).hexdigest(),flush=True)
   r=bytearray(data);actor=u(r,0x9D254);task=0
   for chain in range(3):
    node=u(r,actor+0xA0+chain*4)
    while node:
     if u(r,node)==0x801CDAFC:task=node
     node=u(r,node+0x24)
   assert task, 'captured opcode5F task not found in actor chains'
   w(r,0x9D2F0,actor);w(r,0x9D300,task)
   check(r,0,'captured suspend',False);check(r,1,'captured resume',False)
   print('PASS captured suspend/resume sequence',flush=True)
 if args.write_header:
  lines=['/* Original MIPS task pause/resume and full-VM cases. */']
  for name,values in (('ranges',RANGES),('common',[(a,v) for a,v in base.items() if v]),('patches',patches)):
   lines.append(f'static const uint32_t task_pause_{name}[][2]={{')
   lines.extend(f'{{0x{a:X}u,0x{v:X}u}},' for a,v in values);lines.append('};')
  lines.append('static const struct { unsigned entry,first,end,result; uint64_t hash; } task_pause_cases[]={')
  lines.extend(f'{{{e},{a},{b},{v},UINT64_C(0x{x:X})}},' for e,a,b,v,x in rows);lines.append('};')
  (ROOT/'pc_port/tests/retail_task_pause_cases.h').write_text('\n'.join(lines)+'\n')
 print('PASS:',len(rows),'original/native cases;',len(seen_all),'instruction PCs',flush=True)

if __name__=='__main__':main()
