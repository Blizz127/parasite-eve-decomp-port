#!/usr/bin/env python3
"""Full retail D1 wrapper/callee comparison; captures are isolated calls only."""
import argparse
import hashlib
import itertools
from pathlib import Path
import struct
import subprocess
import tempfile
from pe_battle_hud_oracle import ROOT, execute

RANGES=((0x9CE98,0x40),(0xBCEA8,224),(0x120F20,4),(0x140000,16))
NATIVE=r'''
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "game_port.h"
#include <stdio.h>
#include <stdlib.h>
void Trace_Direct(const char *s) {(void)s;}
int main(int argc,char **argv) {
 if(argc!=5)return 2;PE_RamInit();FILE *f=fopen(argv[1],"rb");
 if(!f || fread(PE_Translate(PE_RAM_BASE,PE_RAM_SIZE),1,PE_RAM_SIZE,f)!=PE_RAM_SIZE)return 3;
 fclose(f);int result=func_80019D84((pe_addr_t)strtoul(argv[3],0,0));
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

def main():
 parser=argparse.ArgumentParser(description=__doc__)
 parser.add_argument('--build-dir',type=Path,default=ROOT/'pc_port/build')
 parser.add_argument('--capture',type=Path)
 parser.add_argument('--write-header',action='store_true')
 args=parser.parse_args()
 exe=(ROOT/'build/extracted/disc1/SLUS_006.62').read_bytes()
 assert hashlib.sha1(exe).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
 for start,end in ((0x19D84,0x19DB8),(0x375E0,0x37864)):
  print(hex(start),(end-start)//4,'words SHA256',hashlib.sha256(exe[start-0xF800:end-0xF800]).hexdigest(),flush=True)
 covered=bytearray(0x1FE000)
 for a,n in RANGES:covered[a:a+n]=bytes([1])*n
 seen_all=set();rows=[];base=None;patches=[]
 with tempfile.TemporaryDirectory(prefix='pe-message-window-') as directory:
  temp=Path(directory);source=temp/'native.c';source.write_text(NATIVE)
  binary=temp/'native';inp=temp/'in.bin';out=temp/'out.bin'
  subprocess.run(['cc','-O2',*[f'-I{ROOT}/pc_port/{p}' for p in ('include','platform','bootstrap','src','.')],str(source),str(args.build_dir.resolve()/'libpe_field_runtime.a'),'-o',str(binary)],check=True)
  def check(r,arg,label,ranges=True):
   initial=bytes(r);seen=set()
   regs=execute(r,0x80019D84,(arg,),visited_pcs=seen,instruction_budget=100000)
   for pc in seen|{p+4 for p in seen}:
    off=pc-0x8000F800
    assert 0x800<=off<len(exe)-3 and initial[pc&0x1FFFFF:(pc&0x1FFFFF)+4]==exe[off:off+4],('instruction',hex(pc))
   seen_all.update(seen)
   if ranges:assert all(covered[i] or r[i]==initial[i] for i in range(0x1FE000)),('uncovered original write',label)
   inp.write_bytes(initial);got=int(subprocess.check_output([str(binary),str(inp),str(out),str(arg),'0']))
   assert got==regs[2]==1,('return',label,got,regs[2])
   native=out.read_bytes();assert len(native)==0x200000
   if native[:0x1FE000]!=r[:0x1FE000]:
    raise AssertionError((label,[(hex(i),native[i],r[i]) for i in range(0x1FE000) if native[i]!=r[i]][:20]))
  for k,(mid,mask,pattern) in enumerate(itertools.product((0,8,0x7FFF,0x8000,0xFFFF),range(16),range(2))):
   r=bytearray(0x200000);r[0x10000:0x10000+len(exe)-0x800]=exe[0x800:]
   for a,n in RANGES:r[a:a+n]=bytes((i*37+pattern*79)&255 for i in range(n))
   for slot in range(4):
    r[0xBCEA8+slot*56]=(2+slot) if mask&(1<<slot) else 0
    w(r,0xBCEA8+slot*56+12,0xFFFFFFFF if pattern else 0x12345678)
   for i,value in enumerate((8,180,296,24) if not pattern else (0xFFFF,0x8000,0x7FFF,0)):
    h(r,0x9CE98+i*2,value)
   w(r,0x140000,0x80140008);h(r,0x140008,mid)
   # Native guest temporary represents the original stack {-1} list. Seed
   # only these two implementation-storage bytes; include them in comparison.
   h(r,0x120F20,0xFFFF)
   initial={a+i:u(r,a+i) for a,n in RANGES for i in range(0,n,4)}
   if base is None:base=initial
   first=len(patches);patches.extend((a,v) for a,v in initial.items() if v!=base[a])
   check(r,0x80140000,(mid,mask,pattern));rows.append((first,len(patches),fingerprint(r)))
   if k%40==0:print('cases checked',k+1,flush=True)
  if args.capture:
   data=args.capture.read_bytes();assert len(data)==0x200000
   print('capture',hashlib.sha256(data).hexdigest(),flush=True)
   r=bytearray(data);assert u(r,0x120F80)==0x801B5E14 and u(r,0x801B5E14)==8
   h(r,0x120F20,0xFFFF)
   check(r,0x80120F80,'captured D1',False)
   print('PASS captured D1 with full original callee',flush=True)
 if args.write_header:
  lines=['/* Retail D1 wrapper with full message-opening callee. */']
  for name,values in (('ranges',RANGES),('common',list(base.items())),('patches',patches)):
   lines.append(f'static const uint32_t message_window_{name}[][2]={{')
   lines.extend(f'{{0x{a:X}u,0x{v:X}u}},' for a,v in values);lines.append('};')
  lines.append('static const struct { unsigned first,end; uint64_t hash; } message_window_cases[]={')
  lines.extend(f'{{{a},{b},UINT64_C(0x{x:X})}},' for a,b,x in rows);lines.append('};')
  (ROOT/'pc_port/tests/retail_message_window_cases.h').write_text('\n'.join(lines)+'\n')
 print('PASS:',len(rows),'original/native cases;',len(seen_all),'instruction PCs',flush=True)
if __name__=='__main__':main()
