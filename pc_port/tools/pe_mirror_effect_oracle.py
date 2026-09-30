#!/usr/bin/env python3
"""Run the original dressing-room mirror overlay and complete native callees.
Synthetic actor, mirror, mesh and packet fixtures; no replaced MIPS callees.
"""
import hashlib,struct,sys
from pe_battle_hud_oracle import ROOT,execute
from pe_btl14_m0005i_publish_oracle import find_disc,read_form1
from pe_scripted_exit_oracle import words
ENTRIES=(0x8018EFFC,0x8018F020,0x8018F1C4,0x8018F1F8,0x8018F580,0x8018F0F0,0x8006F39C,0x8006F6D4,0x8006F8EC,0x8006F9F0,0x800187C0)
RANGES=((0x140000,0x2000),(0x150000,0xB00),(0x160000,0x2000),(0x170000,0x1000),(0x180000,0x4000),(0xB1644,64),(0xA636C,64))
CASES=[dict(entry=0)]
for mode in (0,1,2):
 for command in (0,1,2,3,4):CASES.append(dict(entry=1,mode=mode,command=command))
for target in (0,1,2,3):CASES.append(dict(entry=1,target=target))
CASES.append(dict(entry=2))
for dx,dz in ((1000,0),(0,1000),(-1000,0),(0,-1000),(1000,1000),(-1000,1000),(1000,-1000),(321,-937)):
 for x,z in ((300,700),(-300,-700),(0,0)):
  CASES.append(dict(entry=3,dx=dx,dz=dz,x=x,z=z))
for bank in (0,1):
 for winding in (0,1,2):
  for depth in (0,1000,16380,16384,32767):CASES.append(dict(entry=4,bank=bank,winding=winding,depth=depth))
for initialized in (0,1):
 for visible in (0,1):
  for bank in (0,1):CASES.append(dict(entry=5,initialized=initialized,visible=visible,bank=bank))
CASES.extend((dict(entry=6),dict(entry=7,command=1),dict(entry=8),dict(entry=9)))

for command in range(5):CASES.append(dict(entry=10,command=command))

def fixture(exe,overlay,c):
 r=bytearray(0x200000);r[0x10000:0x10000+len(exe)-0x800]=exe[0x800:];r[0x18EFE8:0x18EFE8+len(overlay)]=overlay
 s=bytearray(0x200000)
 def put(a,b):r[a:a+len(b)]=b;s[a:a+len(b)]=b
 def sw(a,v):put(a,struct.pack('<I',v&0xffffffff))
 def sh(a,v):put(a,struct.pack('<H',v&65535))
 def sb(a,v):put(a,bytes((v&255,)))
 for a,n in RANGES:put(a,bytes(n))
 for a,n in ((0x9589C,0x804),(0x960BC,0x200),(0x966EC,0x4000),(0x9A6EC,0x804)):
  put(a,exe[a-0x10000+0x800:a-0x10000+0x800+n])
 for a in (0x18EFFC,0x18F000,0x18F1C4,0x18FB3C):put(a,overlay[a-0x18EFE8:a-0x18EFE8+4])
 put(0x18FB60,overlay[0x18FB60-0x18EFE8:0x18FB72-0x18EFE8])
 dest=0x1401b4
 # Source actor and reflected object share a synthetic one-joint model.
 sw(0x1401b0,0x80160400);sh(0x140016,0)
 sw(0x1401fc,c.get('x',300));sw(0x140200,-40);sw(0x140204,c.get('z',700))
 sh(0x1401e0,513);sh(0x1401e2,-713);sh(0x1401e4,201)
 sw(0x140004,0x80140800);sb(0x14000c,0);sb(0x14000d,0)
 sw(0x140804,0x80140c00);sb(0x14080c,7);sb(0x14080d,3);sw(0x140898,0x10 if c.get('target')==2 else 0)
 sb(0x140c0c,7);sb(0x140c0d,3)
 sw(0x9D20C,0 if c.get('target')==3 else 0x80140000)
 sw(0x150008,0x80141000);sw(0x15000c,0x80140000)
 sb(0x150000,1);sb(0x150001,0x45);sb(0x150018,c.get('initialized',0));sb(0x150019,c.get('visible',1))
 for i,v in enumerate((100,200,1100,200)):sh(0x150010+i*2,v)
 dest=0x1411b4
 sw(dest,0x80160000);sw(dest+4,0x80160100);sw(dest+8,0x80160200);sw(dest+16,0x80160300)
 sw(dest+0x18,0x80160600);sw(dest+0x80,0x80160700);sw(dest+0x20,0x80160500);sw(dest+0x54,0x80170000);sw(dest+0x84,0x80161000);sw(dest+0x60,0x80140000)
 sh(dest+0xBC,100);sh(dest+0xBE,200);sh(dest+0xC0,100+c.get('dx',1000));sh(dest+0xC2,200+c.get('dz',0))
 sb(0x160002,1);sh(0x160018,1)
 for i in range(4):sh(0x160008+i*2,1)
 sh(0x160102,6);sb(0x160104,1)
 # wide clip: one frame, one joint, constant XYZ and Euler angles.
 sb(0x160400,2);sb(0x160402,0)
 for i,v in enumerate((0,0,0,0,0,0)):sh(0x16040c+i*4,1);sh(0x16040e+i*4,v)
 for i,(x,y,z) in enumerate(((0,0,0),(100,0,0),(0,100,0),(100,100,0),(0,0,0),(0,0,0))):
  for j,v in enumerate((x,y,z)):sh(0x160200+i*8+j*2,v)
 for i in range(4):
  for j in range(4 if i%2==0 else 3):sh(0x160304+i*12+j*2,j)
 for i in range(0x1000//4):sw(0x170000+i*4,0x0C123456)
 for i in range(4096):sw(0x180000+i*4,0x00ffffff)
 points=((0,0),(0,100),(100,0),(100,100))
 if c.get('winding')==1:points=((0,0),(100,0),(0,100),(100,100))
 if c.get('winding')==2:points=((0,0),)*4
 for i,(x,z) in enumerate(points):sw(0xB1644+i*4,x|(z<<16));sw(0xA636C+i*4,c.get('depth',1000))
 sw(0x9D1A0,0x80)
 bank=c.get('bank',0);sw(0x9CDDC,bank);sw(0xB0E38+bank*4,0x80180000)
 sw(0xBCFA4,0x80161800)
 for i,v in enumerate((4096,0,0,0,4096,0,0,0,4096)):sh(0x161800+i*2,v)
 sw(0x16181c,4096)
 # Scheduler entry/pool is synthetic, but callbacks use original addresses.
 sw(0x942E0,0x80162000);sw(0x942E4,0x80150000);sw(0x162114,0x80163000)
 for i,v in enumerate((0x8018EFF4,0x8018EFFC,0x8018F020,0x8018F0F0,0x8018F1A4,0x8018F1AC,0x8018F1BC)):sw(0x163000+i*4,v)
 if c['entry']==6:sb(0x150000,0)
 for i,v in enumerate((0,c.get('command',0),0 if c.get('command',0)==0 else 713,0 if c.get('command',0)==0 else -201,0)):
  sw(0x161900+i*4,0x80161940+i*4);sw(0x161940+i*4,v)
 args=((0x80150000,), (0x80150000,c.get('mode',0),c.get('command',0),7 if c.get('target',0) else 0,3 if c.get('target',0) else 0),
       (0x801411B4,0x80140000,-713,201,913,-412),(0x801411B4,),(0x801411B4,),(0x80150000,),
       (0x45,0x80141000),(0,0,c.get('command',0),713,201,0),(0,),(0,),(0x80161900,))[c['entry']]
 ctrl={24:160*65536,25:112*65536,26:256}
 return r,s,args,ctrl

def fingerprint(r):
 h=1469598103934665603
 for a,n in RANGES:
  for b in r[a:a+n]:h=((h^b)*1099511628211)&0xffffffffffffffff
 return h

def main():
 exe=(ROOT/'build/disc1.candidate.exe').read_bytes();assert hashlib.sha1(exe).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
 overlay=read_form1(find_disc(ROOT),12868,2)
 assert hashlib.sha256(overlay).hexdigest()=='bff195a272f6b7fc19a4eba453c8ad6b138537b7dfa8c64a0da44c534777a77f'
 print('overlay SHA256',hashlib.sha256(overlay).hexdigest())
 _,s,_,_=fixture(exe,overlay,CASES[0]);base=words(s);common=[(i*4,v) for i,v in enumerate(base) if v];patches=[];cases=[]
 for k,c in enumerate(CASES):
  r,s,args,ctrl=fixture(exe,overlay,c);first=len(patches)
  patches.extend((i*4,v) for i,(v,b) in enumerate(zip(words(s),base)) if v!=b)
  regs=execute(r,ENTRIES[c['entry']],args,initial_cop_control=ctrl)
  result=regs[2] if c['entry'] not in (2,3,4) else 0
  h=fingerprint(r);cases.append((c['entry'],first,len(patches),args,result,h));print(k,c,hex(result),hex(h),flush=True)
  if '--dump' in sys.argv:(ROOT/f'local/live/mirror-oracle-{k}.bin').write_bytes(r)
 out=['/* Generated by pe_mirror_effect_oracle.py; original Disc 1 instructions. */']
 for name,rows in (('ranges',RANGES),('common',common),('patches',patches)):
  out.append(f'static const uint32_t SEW15_{name}[][2]={{');out += [f'    {{0x{a:X}u,0x{b:X}u}},' for a,b in rows];out.append('};')
 out.append('static const struct { unsigned entry,first,end; uint32_t args[6],result; uint64_t hash; } SEW15_cases[]={')
 for e,a,b,args,v,h in cases:
  params=','.join(f'0x{x&0xffffffff:X}u' for x in args);out.append(f'    {{{e},{a},{b},{{{params}}},0x{v:X}u,UINT64_C(0x{h:016X})}},')
 out.append('};')
 if '--write-header' in sys.argv:(ROOT/'pc_port/tests/retail_mirror_effect_cases.h').write_text('\n'.join(out)+'\n')
 print('PASS:',len(cases),'complete original mirror cases')
if __name__=='__main__':main()
