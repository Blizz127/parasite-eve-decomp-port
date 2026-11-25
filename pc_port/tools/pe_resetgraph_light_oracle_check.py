#!/usr/bin/env python3
"""Compare native ResetGraph light dispatch with the original executor."""
import argparse, ctypes, hashlib, pathlib, random, struct
parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--exe',required=True,type=pathlib.Path)
parser.add_argument('--library',required=True,type=pathlib.Path)
args=parser.parse_args()
import pe_cd_leaf_vectors as c
exe=args.exe.read_bytes();assert hashlib.sha1(exe).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
base=bytes(c.v.fixture(exe,0,0,0)); rng=random.Random(0x74a4401)
L=ctypes.CFUNCTYPE(ctypes.c_uint32,ctypes.c_uint32,ctypes.c_uint32)
H=ctypes.CFUNCTYPE(ctypes.c_uint32,ctypes.c_uint32,ctypes.c_uint32,ctypes.c_uint32)
lib=ctypes.CDLL(str(args.library.resolve()));lib.set_callbacks.argtypes=[L,H];lib.pe_plat_resetgraph_light_host.argtypes=[ctypes.c_int];lib.pe_plat_resetgraph_light_host.restype=ctypes.c_int
for i in range(600):
 ram,_,_,_=c.prep(base,1); root1=0x80140040;root2=0x80140080; cb1=0x80170000;cb2=0x80170040;cb3=0x80170080
 mode=(rng.getrandbits(29)<<3)|rng.choice((1,2,4,6,7)); debug=rng.randrange(4); mutate=rng.randrange(2);ret=rng.getrandbits(32)
 struct.pack_into('<I',ram,0x95744,root1);struct.pack_into('<I',ram,0x95748,cb1);struct.pack_into('<I',ram,0x140074,cb2);struct.pack_into('<I',ram,0x1400b4,cb3);ram[0x9574e]=debug
 native=bytearray(ram);calls1=[];calls2=[]
 def act(mem,fn,a0,a1,calls):
  calls.append((fn,a0,a1 if fn==cb1 else 0))
  if fn==cb1 and mutate:struct.pack_into('<I',mem,0x95744,root2)
  return ret
 def original(fn):
  def cb(mem,regs):return act(mem,fn,regs[4]&0xffffffff,regs[5]&0xffffffff,calls1)
  return cb
 c.UNFAITHFUL[0]=False
 out=c.run_with_callees(ram,0x80074a44,[mode],{}, {fn:original(fn) for fn in (cb1,cb2,cb3)})
 assert not c.UNFAITHFUL[0],i
 def load(a,n):return int.from_bytes(native[(a&0x1fffff):(a&0x1fffff)+n],'little')
 def call(fn,a0,a1):return act(native,fn,a0,a1,calls2)
 lc=L(load);hc=H(call);lib.set_callbacks(lc,hc);got=lib.pe_plat_resetgraph_light_host(mode)&0xffffffff
 assert got==out[0] and calls1==calls2 and native[0x95700:0x95820]==ram[0x95700:0x95820],(i,got,out[0],calls1,calls2)
print('PASS 600 original-executor vs native light-path cases; callbacks scripted, including root mutation')
