#!/usr/bin/env python3
"""Execute complete original M34 movement/drawing graphs and compare native RAM.

Original instruction words and following words are checked. No callee mocks.
Synthetic fixture changes are confined to isolated calls, never route gameplay.
"""
import argparse,hashlib,itertools,struct,subprocess,tempfile
from pathlib import Path
from pe_battle_hud_oracle import ROOT,execute
from pe_btl14_m0005i_publish_oracle import find_disc,read_form1

RANGES=((0x942E0,16),(0x9CDD8,8),(0x9D1CC,4),(0x9D248,16),
        (0xB0E38,8),(0xB0E58,8),(0xBCFA4,4),(0xE2248,8),(0xE27AC,4),(0xE2844,4),
        (0xF3300,0x200),(0x140000,0x400),(0x141000,0x100),(0x142000,0x40),
        (0x150000,0xC00),(0x160000,0xC000),(0x170000,0x40),
        (0x18EFFC,16),(0x18F014,4),(0x18F020,4),(0x18FF34,0x50),(0x190050,32))
SLOT=0x80150000;REC=SLOT+128;DATA=SLOT+512
CTRL={24:160<<16,25:112<<16,26:256,29:0x155}
NATIVE=r'''
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "pe_sdk.h"
#include "game_port.h"
#include <stdio.h>
#include <stdlib.h>
void Trace_Direct(const char*s){(void)s;}
int main(int argc,char**argv){
 if(argc!=4)return 2;PE_RamInit();FILE*f=fopen(argv[1],"rb");
 if(!f||fread(PE_Translate(PE_RAM_BASE,PE_RAM_SIZE),1,PE_RAM_SIZE,f)!=PE_RAM_SIZE)return 3;fclose(f);
 g_pe_gte.ofx=160*65536;g_pe_gte.ofy=112*65536;g_pe_gte.h=256;g_pe_gte.zsf3=0x155;
 if(!PE_M34BossEffectChild(strtoul(argv[3],0,0),0x80150000,0x80150080,0x80150200)||PE_Port_ShouldStop())return 4;
 f=fopen(argv[2],"wb");if(!f)return 5;
 if(fwrite(PE_Translate(PE_RAM_BASE,PE_RAM_SIZE),1,PE_RAM_SIZE,f)!=PE_RAM_SIZE)return 6;
 if(fwrite(PE_Translate(0x1F800000,1024),1,1024,f)!=1024)return 7;
 return fclose(f)?8:0;
}
'''
def w(r,a,v):struct.pack_into('<I',r,a&0x1FFFFF,v&0xFFFFFFFF)
def h(r,a,v):struct.pack_into('<H',r,a&0x1FFFFF,v&65535)
def b(r,a,v):r[a&0x1FFFFF]=v&255
def u(r,a):return struct.unpack_from('<I',r,a)[0]
def fingerprint(r,scratch):
 value=14695981039346656037
 for a,n in RANGES:
  for x in r[a:a+n]:value=((value^x)*1099511628211)&0xFFFFFFFFFFFFFFFF
 for x in scratch:value=((value^x)*1099511628211)&0xFFFFFFFFFFFFFFFF
 return value

def fixture(source,c):
 r=bytearray(source)
 for a,n in RANGES:
  if a<0x18EFE8 or a>=0x190050:r[a:a+n]=bytes(n)
 for a,n in ((0x150200,0xA00),(0x160000,0x4000),(0x190050,32)):
  r[a:a+n]=bytes((i*17+7)&255 for i in range(n))
 w(r,SLOT+8,0x80140000);w(r,0x140000,0x80141000);w(r,0x141010,100)
 b(r,SLOT,1);b(r,SLOT+1,8);w(r,0x9D254,0x80140300);w(r,0x140300,0x80141020)
 w(r,0xE2248,SLOT+12);w(r,0xF34F4,REC);w(r,0xF3330,DATA)
 b(r,REC,2);b(r,REC+1,1);h(r,REC+4,0);h(r,SLOT+16,c.get('offset',72));b(r,SLOT+18,1)
 b(r,DATA,1);b(r,DATA+2,c.get('time',2));h(r,DATA+4,c.get('size',128));h(r,DATA+6,c.get('fade',0))
 for i,v in enumerate(c.get('position',(100,200,300))):h(r,DATA+8+i*2,v)
 for i,v in enumerate(c.get('velocity',(10,-20,30))):h(r,DATA+24+i*2,v)
 h(r,0x942EC,-1234);w(r,0x9D248,0x80170000)
 polygons=(((-1000,-1000),(1000,-1000),(1000,1000),(-1000,1000)),
           ((-500,-800),(1000,200),(-300,900)),())
 poly=polygons[c.get('poly',0)];h(r,0x9D1CC,len(poly))
 for i,(x,z) in enumerate(poly):w(r,0x170000+i*8,x<<16);w(r,0x170004+i*8,z<<16)
 if c.get('full'):
  for i in range(64):b(r,REC+i*6+1,1)
 kind=c.get('kind',3)
 if kind==0:w(r,SLOT+8,0)
 elif kind==1:w(r,0x140000,0)
 elif kind==2:w(r,0x141010,0)
 elif kind==4:b(r,SLOT+1,7)
 elif kind==5:w(r,0x9D254,0x80140000)
 if c['fn'] in (0x8018FDE4,0x8018F830):
  w(r,0xBCFA4,0x80142000);w(r,0x9CDDC,c.get('bank',0))
  for a,v in ((0xB0E58,0x80160000),(0xB0E5C,0x80162000),(0xB0E38,0x80164000),(0xB0E3C,0x80168000)):w(r,a,v)
  for a in range(0x164000,0x16C000,4):w(r,a,0xCCFFFFFF)
  for i,v in enumerate(c.get('rotation',(4096,0,0,0,4096,0,0,0,4096))):h(r,0x142000+i*2,v)
  w(r,0x14201C,c.get('depth',1024))
  for i,v in enumerate((128,128,128,0,43,2,c.get('flip',0),0)):b(r,0x190060+i,v)
  h(r,0x190068,c.get('bias',0))
 if c['fn']==0x8018F830:
  b(r,DATA,c.get('active',1));h(r,DATA+6,c.get('fade',128))
  for i,v in enumerate(c.get('matrix',(4096,0,0,0,0,4096,0,-4096,0))):h(r,DATA+36+i*2,v)
  h(r,DATA+54,0x1234)
  for i,v in enumerate(c.get('point',(100,200,300))):h(r,u(r,0x9D254)+42+i*4,v)
  for i,v in enumerate((32,32,64,0,32,3,c.get('flip',0),0)):b(r,0x190050+i,v)
  h(r,0x190058,c.get('bias',0));h(r,0x19005A,128)
 return r

def cases():
 for tick in (0,1,2,3,125,126,127,128,129,253,254,255):
  for fade in (-32768,-1,0,128,129,32767):yield dict(fn=0x8018FC54,time=tick,fade=fade)
 for poly,pos,velocity in itertools.product(range(3),((999,32760,999),(1000,-32768,0),(-1000,0,-1000),(32767,32767,-32768)),((1,10,-1),(-32768,-32768,32767))):
  yield dict(fn=0x8018FC54,poly=poly,position=pos,velocity=velocity,size=32767)
 for kind,full,offset in itertools.product(range(6),(False,True),(72,2040,-1)):
  yield dict(fn=0x8018FC54,kind=kind,full=full,offset=offset)
 for bank,level,depth in itertools.product((0,1),(-32768,-1,0,1,128,32767),(-100,0,1024,16384)):
  yield dict(fn=0x8018FDE4,bank=bank,size=level,depth=depth)
 for flip,bias in itertools.product(range(4),(-32768,-1000,4095,32767)):
  yield dict(fn=0x8018FDE4,flip=flip,bias=bias,rotation=(0,0,4096,0,4096,0,-4096,0,0),position=(-32768,32767,-1))
 for kind,point in itertools.product(range(6),((100,200,300),(100,0,301),(110,32767,320),(132,0,300),(133,0,300),(0,0,0),(1000,0,1000))):
  yield dict(fn=0x8018F830,kind=kind,point=point)
 for bank,size,fade in itertools.product((0,1),(-32768,-1,0,1,128,32767),(-1,0,128,32767)):
  yield dict(fn=0x8018F830,bank=bank,size=size,fade=fade,velocity=(-32768,32767,-1),position=(32767,-32768,-100))
 for active in (0,2,127,128,255):yield dict(fn=0x8018F830,active=active)
 for depth,flip in itertools.product((-100,0,16384),range(4)):
  yield dict(fn=0x8018F830,depth=depth,flip=flip)

def main():
 p=argparse.ArgumentParser();p.add_argument('--build-dir',type=Path,default=ROOT/'pc_port/build');p.add_argument('--write-header',action='store_true');args=p.parse_args()
 ex=(ROOT/'build/extracted/disc1/SLUS_006.62').read_bytes();assert hashlib.sha1(ex).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
 ov=read_form1(find_disc(ROOT),16597,100);assert hashlib.sha256(ov).hexdigest()=='0eb2efb10e4779672a00f6da46c2d54f915f1b3e433048513fd08de296eedd5a'
 source=bytearray(0x200000);source[0x10000:0x10000+len(ex)-0x800]=ex[0x800:];source[0x18EFE8:0x18EFE8+len(ov)]=ov
 covered=bytearray(0x1FE000)
 for a,n in RANGES:covered[a:a+n]=bytes([1])*n
 bases={};patches=[];rows=[];seen_all=set();hits=0;packet_counts=set()
 for start,end in ((0x18F830,0x18FC54),(0x18FC54,0x18FDD4),(0x18FDE4,0x18FEE0),(0xC61A8,0xC62DC),(0xC653C,0xC6584)):
  print(hex(start),hex(end),(end-start)//4,'words',hashlib.sha256(source[start:end]).hexdigest(),flush=True)
 with tempfile.TemporaryDirectory(prefix='pe-m34-children-') as d:
  temp=Path(d);src=temp/'native.c';src.write_text(NATIVE);binary=temp/'native';inp=temp/'in';out=temp/'out'
  subprocess.run(['cc','-O2',*[f'-I{ROOT}/pc_port/{x}' for x in ('include','platform','bootstrap','src','.')],str(src),str(args.build_dir.resolve()/'libpe_field_runtime.a'),'-o',str(binary)],check=True)
  for k,c in enumerate(cases()):
   r=fixture(source,c);initial=bytes(r);scratch=bytearray(1024);seen=set()
   execute(r,c['fn'],(SLOT,REC,DATA),scratchpad=scratch,visited_pcs=seen,initial_cop_control=CTRL,instruction_budget=500000)
   for pc in seen|{pc+4 for pc in seen}:
    a=pc&0x1FFFFF;assert initial[a:a+4]==source[a:a+4],('instruction',hex(pc))
   seen_all.update(seen)
   missing=[hex(i) for i in range(0x1FE000) if r[i]!=initial[i] and not covered[i]];assert not missing,('uncovered write',c,missing[:20])
   inp.write_bytes(initial);subprocess.run([str(binary),str(inp),str(out),str(c['fn'])],check=True)
   native=out.read_bytes();assert len(native)==0x200400
   assert native[:0x1FE000]==r[:0x1FE000],(c,[(hex(i),native[i],r[i]) for i in range(0x1FE000) if native[i]!=r[i]][:20])
   assert native[0x200000:]==scratch,('scratch',c,[(hex(i),native[0x200000+i],scratch[i]) for i in range(1024) if native[0x200000+i]!=scratch[i]][:20])
   if c['fn']==0x8018F830:
    hits+=bool(r[(REC+1)&0x1FFFFF]==2);packet_counts.add(u(r,0x9CDD8)//40)
   family={0x8018FC54:0,0x8018FDE4:1,0x8018F830:2}[c['fn']];words={a+i:u(initial,a+i) for a,n in RANGES for i in range(0,n,4)}
   bases.setdefault(family,words);first=len(patches);patches.extend((a,v) for a,v in words.items() if v!=bases[family][a])
   rows.append((c['fn'],family,first,len(patches),fingerprint(r,scratch)))
   if k%40==0:print('PASS through case',k,c,flush=True)
 if args.write_header:
  lines=['/* Complete original M34 movement/draw graphs; generated, not native-derived. */']
  tables=[('ranges',RANGES),('patches',patches)]
  for name,values in tables:
   lines.append(f'static const uint32_t m34_children_{name}[][2]={{');lines.extend('{0x%Xu,0x%Xu},'%(a,v) for a,v in values);lines.append('};')
  for family,base in sorted(bases.items()):
   runs=[]
   for a,v in base.items():
    if not v:continue
    if runs and runs[-1][0]+runs[-1][1]*4==a and runs[-1][2]==v:runs[-1][1]+=1
    else:runs.append([a,1,v])
   lines.append(f'static const uint32_t m34_children_common{family}[][3]={{');lines.extend('{0x%Xu,%du,0x%Xu},'%tuple(x) for x in runs);lines.append('};')
  lines.append('static const struct {uint32_t fn;unsigned family,first,end;uint64_t hash;} m34_children_cases[]={')
  lines.extend('{0x%Xu,%du,%du,%du,UINT64_C(0x%X)},'%row for row in rows);lines.append('};')
  (ROOT/'pc_port/tests/retail_m34_children_cases.h').write_text('\n'.join(lines)+'\n')
 assert hits>0 and {0,5}<=packet_counts,('missing hit/draw coverage',hits,packet_counts)
 print('PASS',len(rows),'original/native cases;',len(seen_all),'instruction PCs; full non-stack RAM and scratchpad;',hits,'hits; packet counts',sorted(packet_counts),flush=True)
if __name__=='__main__':main()
