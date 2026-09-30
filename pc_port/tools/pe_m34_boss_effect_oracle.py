#!/usr/bin/env python3
"""Original M34 boss effect calls; complete RAM comparisons, no callee mocks."""
import argparse, hashlib, struct, subprocess, tempfile
from pathlib import Path
from pe_battle_hud_oracle import ROOT, execute
from pe_btl14_m0005i_publish_oracle import find_disc, read_form1
RANGES=((0,0x80),(0x942E0,16),(0x9D254,4),(0x9DF70,32),(0xB0CD8,0x100),
        (0xE2248,4),(0xF32A8,4),(0xF3330,4),(0xF33B0,4),(0xF34F4,4),
        (0x140000,0x3000),(0x150000,0x7800),(0x190020,0x70))
NATIVE=r'''
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "game_port.h"
#include <stdio.h>
#include <stdlib.h>
void Trace_Direct(const char*s){(void)s;}
int main(int argc,char**argv){
 if(argc!=8)return 2;PE_RamInit();FILE*f=fopen(argv[1],"rb");
 if(!f||fread(PE_Translate(PE_RAM_BASE,PE_RAM_SIZE),1,PE_RAM_SIZE,f)!=PE_RAM_SIZE)return 3;fclose(f);
 unsigned fn=strtoul(argv[3],0,0),a[4];for(int i=0;i<4;i++)a[i]=strtoul(argv[4+i],0,0);
 int v=0;switch(fn){
 case 0x8018F00C:v=PE_M34BossEffectInit(a[0]);break;
 case 0x8018F0B8:v=PE_M34BossEffectCommand(a[0],a[1],a[2],a[3],0,0);break;
 case 0x8018F0E4:v=PE_M34BossEffectDraw(a[0]);break;
 case 0x8018F12C:v=PE_M34BossEffectUpdate(a[0]);break;
 case 0x8018F1B8:v=PE_M34BossEffectCleanup(a[0]);break;
 case 0x8006F39C:v=func_8006F39C(a[0],a[1]);break;
 default:if(!PE_M34BossEffectChild(fn,a[0],a[1],a[2]))return 7;
 }
 if(PE_Port_ShouldStop())return 4;
 f=fopen(argv[2],"wb");if(!f)return 5;size_t n=fwrite(PE_Translate(PE_RAM_BASE,PE_RAM_SIZE),1,PE_RAM_SIZE,f);
 if(fclose(f)||n!=PE_RAM_SIZE)return 6;printf("%d\n",v);return 0;
}
'''
def w(r,a,v):struct.pack_into('<I',r,a&0x1fffff,v&0xffffffff)
def h(r,a,v):struct.pack_into('<H',r,a&0x1fffff,v&65535)
def u(r,a):return struct.unpack_from('<I',r,a&0x1fffff)[0]
def fingerprint(r):
 v=14695981039346656037
 for a,n in RANGES:
  for b in r[a:a+n]:v=((v^b)*1099511628211)&0xffffffffffffffff
 return v
SLOT=0x80150000;ACTOR=0x80140000;REC=SLOT+0x200;DATA=SLOT+0x600

def fixture(ex,ov,kind,pattern):
 r=bytearray(0x200000);r[0x10000:0x10000+len(ex)-0x800]=ex[0x800:];r[0x18efe8:0x18efe8+len(ov)]=ov
 for a,n in ((0x140000,0x3000),(0x150000,0x7800),(0x190028,0x68)):
  r[a:a+n]=bytes((i*37+pattern*71)&255 for i in range(n))
 w(r,ACTOR,0x80141000 if kind!=1 else 0);w(r,ACTOR+0x238,0x80142000)
 w(r,0x141000,0xFFFFFFFF if pattern else 0x3456789);w(r,0x141010,100 if kind!=2 else 0);w(r,0x141018,0x80141100)
 w(r,SLOT+8,0 if kind==0 else ACTOR);r[SLOT&0x1fffff]=1;r[(SLOT+1)&0x1fffff]=7 if kind==4 else 8
 w(r,0x9D254,ACTOR if kind==5 else ACTOR+0x280)
 w(r,0xE2248,SLOT+12)
 return r

def main():
 p=argparse.ArgumentParser();p.add_argument('--build-dir',type=Path,default=ROOT/'pc_port/build');p.add_argument('--capture',type=Path);p.add_argument('--write-header',action='store_true');args=p.parse_args()
 ex=(ROOT/'build/extracted/disc1/SLUS_006.62').read_bytes();assert hashlib.sha1(ex).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
 ov=read_form1(find_disc(ROOT),16597,100);assert hashlib.sha256(ov).hexdigest()=='0eb2efb10e4779672a00f6da46c2d54f915f1b3e433048513fd08de296eedd5a'
 source_ram=bytearray(0x200000);source_ram[0x10000:0x10000+len(ex)-0x800]=ex[0x800:];source_ram[0x18efe8:0x18efe8+len(ov)]=ov
 covered=bytearray(0x1fe000)
 for a,n in RANGES:covered[a:a+n]=bytes([1])*n
 seen_all=set();rows=[];patches=[];base=None
 with tempfile.TemporaryDirectory(prefix='pe-m34-core-') as d:
  temp=Path(d);src=temp/'native.c';src.write_text(NATIVE);binary=temp/'native';inp=temp/'in';out=temp/'out'
  subprocess.run(['cc','-O2',*[f'-I{ROOT}/pc_port/{x}' for x in ('include','platform','bootstrap','src','.')],str(src),str(args.build_dir.resolve()/'libpe_field_runtime.a'),'-o',str(binary)],check=True)
  def check(r,fn,a,label,record=True):
   nonlocal base
   a=tuple(a)+(0,)*(4-len(a));initial=bytes(r);seen=set();scratch=bytearray(1024)
   regs=execute(r,fn,a,visited_pcs=seen,scratchpad=scratch,instruction_budget=500000)
   for pc in seen|{p+4 for p in seen}:
    i=pc&0x1fffff;assert initial[i:i+4]==source_ram[i:i+4],('source',hex(pc))
   seen_all.update(seen)
   if record:
    missing=[hex(i) for i in range(0x1fe000) if not covered[i] and r[i]!=initial[i]];assert not missing,('uncovered',label,missing[:20])
   inp.write_bytes(initial);result=int(subprocess.check_output([str(binary),str(inp),str(out),str(fn),*[str(x&0xffffffff) for x in a]]))
   if fn<0x8018F24C:assert result==(regs[2] if regs[2]<0x80000000 else regs[2]-0x100000000),('result',label,result,regs[2])
   native=out.read_bytes();assert len(native)==0x200000
   if native[:0x1fe000]!=r[:0x1fe000]:raise AssertionError((label,[(hex(i),native[i],r[i]) for i in range(0x1fe000) if native[i]!=r[i]][:24]))
   if record:
    initial_words={a+i:u(initial,a+i) for a,n in RANGES for i in range(0,n,4)}
    if base is None:base=initial_words
    first=len(patches);patches.extend((x,v) for x,v in initial_words.items() if v!=base[x]);rows.append((fn,a,first,len(patches),result,fingerprint(r)))
   return r
  for kind in range(6):
   for pattern in range(2):
    for fn in (0x8018F00C,0x8018F1B8,0x8018F0E4,0x8018F12C):
     r=fixture(ex,ov,kind,pattern)
     if fn in (0x8018F0E4,0x8018F12C):
      # Initialized empty children; VM has reached its actual terminator.
      r[SLOT&0x1fffff:(SLOT&0x1fffff)+0xa0c]=bytes(0xa0c)
      w(r,SLOT+8,0 if kind==0 else ACTOR);r[(SLOT+1)&0x1fffff]=7 if kind==4 else 8
      w(r,SLOT+0x78,0x80190020)
     check(r,fn,(SLOT,),('parent',kind,pattern,hex(fn)))
  for mode in (0,1,0xffffffff):
   for index in (0,1,4,8,0xffffffff):
    for value in (0,0xffffffff,0x80000001):
     check(fixture(ex,ov,3,0),0x8018F0B8,(SLOT,mode,index,value),('command',mode,index,value))
  for fn in (0x8018F24C,0x8018F36C,0x8018F374,0x8018F380,0x8018F3BC,0x8018FDD4,0x8018FEE0):
   for variant in range(12):
    r=fixture(ex,ov,5 if variant&1 else 3,variant%2)
    r[(DATA+2)&0x1fffff]=(0,19,20,21,126,127,128,254,255,5,50,100)[variant]
    h(r,DATA+4,(0,8,9,128,0x7fff,0x8000,0xffff,17,1,20,30,40)[variant])
    check(r,fn,(SLOT,REC,DATA),('child',hex(fn),variant))
  if args.capture:
   raw=args.capture.read_bytes();assert len(raw)==0x200000
   print('capture',hashlib.sha256(raw).hexdigest(),flush=True)
   check(bytearray(raw),0x8018F00C,(0x801861A0,),'captured constructor',False)
   print('PASS captured constructor, full original callee',flush=True)
  if args.write_header:
   lines=['/* Original M34 effect core and child callback fixtures. */']
   for name,values in (('ranges',RANGES),('common',[(x,v) for x,v in base.items() if v]),('patches',patches)):
    lines.append(f'static const uint32_t m34_effect_{name}[][2]={{');lines.extend(f'{{0x{x:X}u,0x{v:X}u}},' for x,v in values);lines.append('};')
   lines.append('static const struct {uint32_t fn,args[4];unsigned first,end;int result;uint64_t hash;} m34_effect_cases[]={')
   lines.extend('{0x%Xu,{%s},%d,%d,%d,UINT64_C(0x%X)},'%(fn,','.join('0x%Xu'%x for x in a),first,end,result,hashed) for fn,a,first,end,result,hashed in rows);lines.append('};')
   (ROOT/'pc_port/tests/retail_m34_effect_cases.h').write_text('\n'.join(lines)+'\n')
 print('PASS',len(rows),'original/native cases;',len(seen_all),'instruction PCs',flush=True)
if __name__=='__main__':main()
