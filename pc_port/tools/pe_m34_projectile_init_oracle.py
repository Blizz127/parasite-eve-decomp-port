#!/usr/bin/env python3
"""Original F434 with explicit retained stack words; no connected-state injection."""
import argparse,hashlib,itertools,struct,subprocess,tempfile
from pathlib import Path
from pe_battle_hud_oracle import ROOT,execute
from pe_btl14_m0005i_publish_oracle import find_disc,read_form1
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
 const int off[6]={-76,-72,-68,-36,-32,-28};int32_t retained[6];
 for(unsigned i=0;i<6;i++)retained[i]=(int32_t)PE_LoadU32(0x801FF000u+off[i]);
 PE_M34BossProjectileInit((pe_addr_t)strtoul(argv[3],0,0),retained);
 if(PE_Port_ShouldStop())return 4;f=fopen(argv[2],"wb");if(!f)return 5;
 size_t n=fwrite(PE_Translate(PE_RAM_BASE,PE_RAM_SIZE),1,PE_RAM_SIZE,f);
 int32_t gte[18];for(unsigned i=0;i<9;i++)gte[i]=g_pe_gte.rt[i/3][i%3];
 for(unsigned i=0;i<3;i++){gte[i+9]=g_pe_gte.tr[i];gte[i+12]=g_pe_gte.ir[i];gte[i+15]=g_pe_gte.mac[i];}
 fwrite(gte,4,18,f);if(fclose(f)||n!=PE_RAM_SIZE)return 6;return 0;
}
'''
OFFSETS=(-76,-72,-68,-36,-32,-28);DATA=0x80140000

def w(r,a,v):struct.pack_into('<I',r,a&0x1fffff,v&0xffffffff)
def h(r,a,v):struct.pack_into('<H',r,a&0x1fffff,v&65535)
def signed(v,bits=32):return (v&((1<<(bits-1))-1))-(v&(1<<(bits-1)))

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--build-dir',type=Path,default=ROOT/'pc_port/build');p.add_argument('--capture',type=Path);p.add_argument('--write-header',action='store_true');args=p.parse_args()
 ex=(ROOT/'build/extracted/disc1/SLUS_006.62').read_bytes();assert hashlib.sha1(ex).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
 ov=read_form1(find_disc(ROOT),16597,100);assert hashlib.sha256(ov).hexdigest()=='0eb2efb10e4779672a00f6da46c2d54f915f1b3e433048513fd08de296eedd5a'
 source=bytearray(0x200000);source[0x10000:0x10000+len(ex)-0x800]=ex[0x800:];source[0x18efe8:0x18efe8+len(ov)]=ov
 print('F434 255 words SHA256',hashlib.sha256(ov[0x44c:0x848]).hexdigest(),flush=True)
 seen_all=set();rows=[]
 with tempfile.TemporaryDirectory(prefix='pe-m34-projectile-') as d:
  temp=Path(d);src=temp/'native.c';src.write_text(NATIVE);binary=temp/'native';inp=temp/'in';out=temp/'out'
  subprocess.run(['cc','-O2',*[f'-I{ROOT}/pc_port/{x}' for x in ('include','platform','bootstrap','src','.')],str(src),str(args.build_dir.resolve()/'libpe_field_runtime.a'),'-o',str(binary)],check=True)
  def check(r,data,retained,label,record=False):
   for off,v in zip(OFFSETS,retained):w(r,0x801ff000+off,v)
   initial=bytes(r);seen=set();gte={}
   execute(r,0x8018f434,(0,0,data),visited_pcs=seen,final_gte=gte,scratchpad=bytearray(1024))
   for pc in seen|{pc+4 for pc in seen}:
    a=pc&0x1fffff;assert initial[a:a+4]==source[a:a+4],('source',hex(pc))
   seen_all.update(seen)
   inp.write_bytes(initial);subprocess.run([str(binary),str(inp),str(out),str(data)],check=True)
   output=out.read_bytes();assert len(output)==0x200000+72;native=output[:0x200000]
   assert native[:0x1fe000]==r[:0x1fe000],(label,[(hex(i),native[i],r[i]) for i in range(0x1fe000) if native[i]!=r[i]][:24])
   expected=[signed(gte['control'][i//2]>>(16*(i%2)),16) for i in range(9)]
   expected += [signed(x) for x in gte['control'][5:8]+gte['data'][9:12]+gte['data'][25:28]]
   got=list(struct.unpack_from('<18i',output,0x200000));assert got==expected,('GTE',label,got,expected)
   if record:
    # Original writes in compared RAM must stay inside the returned record.
    assert all(initial[i]==r[i] or (data&0x1fffff)<=i<(data&0x1fffff)+0x48 for i in range(0x1fe000))
    rows.append((label,struct.unpack_from('<18I',r,data&0x1fffff),tuple(expected)))
  matrices=((4096,0,0,0,4096,0,0,0,4096),(0,0,4096,0,4096,0,-4096,0,0),(32767,-32768,91,-187,10000,-9999,3,7,-17000))
  retained_cases=((0,)*6,(1000,-2000,3000,-4000,5000,-6000),(32767,32768,65535,-32768,-1,0),(0x7fffffff,-0x80000000,0x10001,-0x10001,129,-129))
  for mi,turn,speed,ri in itertools.product(range(3),(-32768,-4096,-1,0,1,1024,32767),(-32768,64,32767),range(4)):
   r=bytearray(source)
   r[0x140000:0x140048]=bytes((i*37+13)&255 for i in range(0x48))
   w(r,0xE2248,0x80150000)
   for i,v in enumerate(matrices[mi]):h(r,0x190028+i*2,v)
   h(r,0x19003A,0xAA55)
   for i,v in enumerate((-12345,257,7890)):w(r,0x19003C+i*4,v)
   w(r,0x150018,turn);w(r,0x150050,16384);w(r,0x150048,speed);w(r,0x150014,0xFF)
   w(r,0x190084,-1234);w(r,0x19008C,2345);h(r,0x942EC,1200)
   check(r,DATA,retained_cases[ri],(mi,turn,speed,ri),True)
   if len(rows)%84==0:print('checked',len(rows),flush=True)
  if args.capture:
   raw=args.capture.read_bytes();assert len(raw)==0x200000
   print('capture',hashlib.sha256(raw).hexdigest(),flush=True)
   for ri,retained in enumerate(retained_cases):check(bytearray(raw),0x801863A0,retained,('capture',ri))
   print('PASS 4 captured-context retained-stack variants',flush=True)
  if args.write_header:
   trig='{'+','.join('{0x%Xu,0x%Xu}'%(0x800966EC+i*4,struct.unpack_from('<I',source,0x966EC+i*4)[0]) for i in (0,1,1024,3072,4095))+'};'
   lines=['/* Original F434 outputs for explicit retained-stack inputs. */',
     'static const uint32_t m34_projectile_trig[][2]='+trig,
     'static const int16_t m34_projectile_matrices[3][9]={'+','.join('{'+','.join(str(x) for x in m)+'}' for m in matrices)+'};',
     'static const int32_t m34_projectile_retained[4][6]={'+','.join('{'+','.join(str(x) if x!=-0x80000000 else 'INT32_MIN' for x in m)+'}' for m in retained_cases)+'};',
     'static const struct {int matrix,turn,speed,retained;uint32_t record[18];int32_t gte[18];} m34_projectile_cases[]={']
   for label,record,gte in rows:lines.append('{%s,{%s},{%s}},'%(','.join(map(str,label)),','.join('0x%Xu'%x for x in record),','.join(str(x) if x!=-0x80000000 else 'INT32_MIN' for x in gte)))
   lines.append('};');(ROOT/'pc_port/tests/retail_m34_projectile_cases.h').write_text('\n'.join(lines)+'\n')
 print('PASS',len(rows),'original/native F434 cases;',len(seen_all),'PCs',flush=True)
if __name__=='__main__':main()
