#!/usr/bin/env python3
"""Original-instruction model fades, packet modes and color restoration.

Synthetic geometry covers four primitive types, both GPU banks, tagged and
untagged color triangles, signed countdowns and full fade histories. Scratch
stack and scratchpad are excluded; every other changed RAM byte is covered.
"""
import hashlib,itertools,struct,sys
from pathlib import Path
from pe_battle_hud_oracle import ROOT,execute
from pe_scripted_exit_oracle import words
ENTRIES=(0x8003C638,0x8003C818,0x8003CCB0,0x8003CEF8,0x8003B708,0x8003C0B4,0x8003AF14)
RANGES=((0x9CDA0,4),(0xA6360,128),(0xB1638,128),(0xBEA40,64),(0x140000,0x5000),(0xB0CEC,0xC0))

def cases():
 for entry,bank,phase in itertools.product((0,1),(0,1),(-128,-1,0,1,2,59,60,127)):
  yield dict(entry=entry,bank=bank,phase=phase)
 for entry,bank,mode in itertools.product((2,3),(0,1),(0,1,2,3,0x10000)):
  yield dict(entry=entry,bank=bank,mode=mode)
 for entry,bank,tag in itertools.product((4,5),(0,1),(0,1,2,3)):
  yield dict(entry=entry,bank=bank,tag=tag)
 for flags,bank in itertools.product((0,1,2,4,8,9,10,12,0x800,0x20,0x40,0x824,0x42),(0,1)):
  yield dict(entry=6,bank=bank,flags=flags,phase=-1)
 for entry,gate in itertools.product(range(7),('obj','active')):
  if entry!=4:yield dict(entry=entry,gate=gate)
 for fade,bank in itertools.product((2,4),(0,1)):
  yield dict(entry=6,bank=bank,flags=fade,phase=-1,frames=65)
 for level in (-32768,-21,0,21,32767):yield dict(entry=5,level=level,tag=3)
 for scale in (1,2,15,30,60,127,-128,-1):yield dict(entry=1,phase=-1,scale=scale)
 for bank in (0,1):yield dict(entry=4,bank=bank,weapon=True)

def fixture(exe,c):
 r=bytearray(0x200000);r[0x10000:0x10000+len(exe)-0x800]=exe[0x800:]
 s=bytearray(0x200000)
 def put(a,b):r[a:a+len(b)]=b;s[a:a+len(b)]=b
 def w(a,v):put(a,struct.pack('<I',v&0xFFFFFFFF))
 def h(a,v):put(a,struct.pack('<H',v&65535))
 def b(a,v):put(a,bytes((v&255,)))
 def matrix(a):
  put(a,bytes(32))
  for off in (0,8,16):h(a+off,4096)
 for a,n in ((0x91A38,0x800),(0x921D8,32)):
  put(a,bytes(r[a:a+n]))
 # The restoration decoder uses only zero angles; retain its sine table.
 put(0x966EC,bytes(r[0x966EC:0x966EC+4]))
 d=0xB0CEC if c.get('weapon') else 0x140200
 obj=0x141000;rec=0x141100;verts=0x141200;colors=0x141400;geo=0x141500;bounds=0x141600;mats=0x141700;packets=0x142000
 put(0x140000,bytes(0x5000));put(d,bytes(0xC0))
 w(d,0x80000000|obj);w(d+4,0x80000000|rec);w(d+8,0x80000000|verts);w(d+12,0x80000000|colors)
 w(d+16,0x80000000|geo);w(d+24,0x80000000|bounds);w(d+32,0x80141800)
 w(d+0x54,0x80000000|packets);w(d+0x84,0x80000000|mats)
 matrix(d+0x34);matrix(mats);matrix(0x143000)
 w(d+0x58,0x80000000|mats);w(d+0x80,0x80141810);w(d+0xB0,0x80141900);w(d-4,0x80141900)
 b(0x141900,2);b(0x141901,0);b(0x141902,1)
 for i in range(6):h(0x14190C+i*4,1);h(0x14190E+i*4,0)
 b(obj+2,1);h(obj+0x18,1);h(obj+0x1A,12)
 h(rec,0);h(rec+2,12);b(rec+4,1)
 h(bounds+6,20)
 for i in range(12):
  for j,v in enumerate((i*10-60,(i%3)*20-20,i*3)):h(verts+i*8+j*2,v)
  h(verts+i*8+6,i)
  w(colors+i*4,0x00706050|((c.get('tag',0)==i%3+1)<<24))
 for off,v in ((0x88,55),(0x89,65),(0x8A,35),(0x8C,c.get('phase',-1)),(0x8D,c.get('scale',60)),(0x8E,2),(0x8F,3),(0x93,5),(0x90,255),(0x91,128),(0x92,63),(0x94,10),(0x95,130),(0x96,254),(0x97,19),(0x98,39),(0x99,59),(0x9E,0),(0x9F,1-c.get('bank',0))):b(d+off,v)
 h(d+0x9C,c.get('flags',0));h(d+0x9A,c.get('level',0));h(d+0xBA,1)
 cursor=packets;g=geo
 for kind,size in enumerate((52,40,36,28)):
  h(obj+8+kind*2,2)
  for i in range(2):
   b(g+3,(11,16,21,26)[kind] if i else 0)
   for j in range(4):h(g+4+j*2,j)
   for bank in (0,1):
    p=cursor+bank*size;put(p,bytes((x*17+bank*29+kind*11)&255 for x in range(size)))
    w(p,0x12345678 if i else 0);b(p+7,0x3F);h(p+26,0x186+bank*32)
   cursor+=size*2;g+=12
 for a,n in ((0xA6360,128),(0xB1638,128)):put(a,bytes((i*37+19)&255 for i in range(n)))
 w(0x9CDDC,c.get('bank',0));w(0x9CDA0,0x808080)
 for i in range(3):b(0xBD025+i,200+i*20)
 if c.get('gate')=='obj':w(d,0)
 if c.get('gate')=='active':h(d+0xBA,0)
 if c['entry']==4 and c.get('gate'):raise AssertionError('restoration requires constructed dest')
 # Shared tick intentionally starts with invisible packets. Chain tests obtain
 # packet tags from the original submission, retaining normal fade semantics.
 w(0xB0E38,0x80143000);w(0xB0E3C,0x80143000)
 for a,n in RANGES:s[a:a+n]=r[a:a+n]
 args=(0x80000000|d,c.get('mode',c.get('bank',0)))
 if c['entry']==5:args=(args[0],c.get('level',0)&0xFFFFFFFF,19,39,59)
 if c['entry']==6:args=(args[0],0x80143000)
 return r,s,args

def fingerprint(r):
 h=14695981039346656037
 for a,n in RANGES:
  for b in r[a:a+n]:h=((h^b)*1099511628211)&0xFFFFFFFFFFFFFFFF
 return h

def main():
 exe=(ROOT/'build/disc1.candidate.exe').read_bytes()
 assert hashlib.sha1(exe).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
 base=None;patches=[];rows=[];points={a+i for a,n in RANGES for i in range(n)}
 for k,c in enumerate(cases()):
  r,s,args=fixture(exe,c);initial=words(s)
  if base is None:base=initial
  first=len(patches);patches.extend((i*4,v) for i,(v,b) in enumerate(zip(initial,base)) if v!=b)
  for frame in range(c.get('frames',1)):
   bank=(c.get('bank',0)+frame)&1;struct.pack_into('<I',r,0x9CDDC,bank)
   before=bytes(r);gte={};scratch=bytearray(0x400)
   execute(r,0x8006698C,(args[0],),final_gte=gte,scratchpad=scratch)
   regs=execute(r,ENTRIES[c['entry']],args,instruction_budget=1000000,scratchpad=scratch,
      initial_cop_control=dict(enumerate(gte['control'])),initial_cop_data=dict(enumerate(gte['data'])))
   for a,(old,new) in enumerate(zip(before[:0x1FE000],r[:0x1FE000])):
    if old!=new:assert a in points,('uncompared original write',k,hex(a))
   result=regs[2] if c['entry'] in (0,1) else 0
   rows.append((c['entry'],first,len(patches),args,bank,frame,result,fingerprint(r)))
   if '--dump' in sys.argv:Path(f'/tmp/pe-model-fade-original-{len(rows)-1}.bin').write_bytes(r)
  print(k,c,'hash',hex(rows[-1][-1]),flush=True)
 out=['/* Generated by pe_model_fade_oracle.py --write-header. */']
 for name,data in (('ranges',RANGES),('patches',patches),('common',[(i*4,v) for i,v in enumerate(base) if v])):
  out.append(f'static const uint32_t model_fade_{name}[][2]={{')
  out.extend(f' {{0x{a:X}u,0x{v:X}u}},' for a,v in data);out.append('};')
 out.append('static const struct { unsigned entry,first,end; uint32_t args[5]; unsigned bank,frame,result; uint64_t hash; } model_fade_cases[]={')
 for e,a,b,args,bank,frame,result,h in rows:
  params=','.join(f'0x{x:X}u' for x in (*args,*([0]*(5-len(args)))))
  out.append(f' {{{e},{a},{b},{{{params}}},{bank},{frame},0x{result:X}u,UINT64_C(0x{h:X})}},')
 out.append('};')
 if '--write-header' in sys.argv:(ROOT/'pc_port/tests/retail_model_fade_cases.h').write_text('\n'.join(out)+'\n')
 print('PASS:',len(rows),'original model fade frames')
if __name__=='__main__':main()
