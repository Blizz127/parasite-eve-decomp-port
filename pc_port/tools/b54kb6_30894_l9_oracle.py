#!/usr/bin/env python3
"""Independent B54K-B6 oracle for func_80030894's complete L9 group."""
import hashlib, pathlib, struct, sys
SHA1="452fb033f2eaa4b18aa20a5bca60b8125af3a37b"; BASE=0x8000F800
START=0x80031110; END=0x800311EC; WORDS=55
WIN="7d64f3e10e16895bafd1b2fb71ed2757a5513072a35bd9772e04bd84ce56a739"
MODEL="4665016a28d54fdbf145211dc35d553678b9554c7d0a402c350a7f9025442636"
CRIT={0x8003110C:0xA2170006,0x80031110:0x3C10800A,
0x80031114:0x2610E768,0x80031120:0x000288C0,0x80031124:0x02228823,
0x80031128:0x00119080,0x8003112C:0x0C00DC37,0x80031134:0x0000B021,
0x80031138:0x00118900,0x80031170:0xA429E77E,0x8003118C:0x32C200FF,
0x80031190:0x000280C0,0x80031194:0x02028023,0x80031198:0x00108080,
0x800311A4:0x0C00DC37,0x800311D8:0x2C420004,0x800311E4:0x1440FFE9,
0x800311E8:0xA2170006,0x800311EC:0x3C10800A}
def need(x,m):
 if not x: raise SystemExit("FAIL: "+m)
def run():
 p=pathlib.Path(__file__).resolve().parent.parent.parent/"build/disc1.candidate.exe";d=p.read_bytes()
 need(hashlib.sha1(d).hexdigest()==SHA1,"exe SHA");w=lambda a:struct.unpack_from("<I",d,a-BASE)[0]
 win=d[START-BASE:END-BASE];need(len(win)==WORDS*4 and hashlib.sha256(win).hexdigest()==WIN,"window")
 print(f"OK window: {WORDS} words / {len(win):#x} bytes, SHA-256 exact")
 for a,v in CRIT.items():need(w(a)==v,f"word {a:#x}")
 calls=[];branches=[]
 for a in range(START,END,4):
  x=w(a);op=x>>26
  if op==3:calls.append((a,0x80000000|((x&0x3ffffff)<<2)))
  if op in (1,2,4,5,6,7,20,21,22,23):
   q=x&0xffff;q=q-0x10000 if q&0x8000 else q;branches.append((a,a+4+q*4))
 need(calls==[(0x8003112C,0x800370DC),(0x800311A4,0x800370DC)],"calls")
 need(branches==[(0x800311E4,0x8003118C)],"branch")
 print("OK control flow: two native jal sites; one four-item back-edge")
 m={}
 def p8(a,v):m[a]=v&255
 def p16(a,v):p8(a,v);p8(a+1,v>>8)
 def spr(h,wd,ht,u=None):
  for o,v in {3:6,4:0x34,5:2,6:0,7:0xe1,8:0,9:0,10:0,11:0,12:0x80,13:0x80,14:0x80,15:0x64}.items():p8(h+o,v)
  if u is not None:p8(h+20,u);p8(h+21,0xef)
  p16(h+22,0x7e13);p16(h+24,wd);p16(h+26,ht)
 spr(0x8009E768,36,5,0x58)
 for j in range(4):spr(0x8009E7A0+j*28,6,6)
 packed=b"".join(struct.pack("<IB",a,v) for a,v in sorted(m.items()));dig=hashlib.sha256(packed).hexdigest()
 print(f"model_unique_written_bytes={len(m)}\nmodel_write_map_sha256={dig}")
 need(len(m)==97 and dig==MODEL,"model");need(0x8009E733 not in m,"next group")
 print("OK independent model: one compound sprite plus four-item array")
 return 0
if __name__=="__main__":sys.exit(run())
