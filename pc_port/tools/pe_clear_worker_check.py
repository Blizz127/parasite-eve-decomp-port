#!/usr/bin/env python3
"""Compare test-only clear packet model with original code and scripted GPU calls."""
from pathlib import Path
import ctypes,hashlib,json,random,struct,sys
w=Path(__file__).resolve().parents[2];sys.path.insert(0,str(w/'pc_port/tools'))
from pe_battle_hud_oracle import execute
exe=(w/'build/disc1.candidate.exe').read_bytes();assert hashlib.sha1(exe).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
lib=ctypes.CDLL(sys.argv[1]);L=ctypes.CFUNCTYPE(ctypes.c_uint32,ctypes.c_uint32,ctypes.c_uint32);S=ctypes.CFUNCTYPE(None,ctypes.c_uint32,ctypes.c_uint32,ctypes.c_uint32);C=ctypes.CFUNCTYPE(ctypes.c_uint32,ctypes.c_uint32,ctypes.c_uint32,ctypes.c_uint32);lib.draft.argtypes=[ctypes.c_uint32,ctypes.c_uint32,L,S,C];lib.draft.restype=ctypes.c_int
rng=random.Random(0x80076434);n=int(sys.argv[2]);bad=[];counts={'fast':0,'general':0}
for i in range(n):
 rect=0x80140000 if len(sys.argv)>3 and sys.argv[3]=="--separate" else rng.choice([0x80140000,0x800a3300,0x800a3304,0x800a3308,0x800a3310,0x800a3320,0x800a3330]);off=rect&0x1fffff
 ram=bytearray(0x200000);ram[0x10000:0x10000+len(exe)-0x800]=exe[0x800:];ram[0xa3300:0xa3338]=rng.randbytes(56)
 def sw(a,v):struct.pack_into('<I',ram,a&0x1fffff,v&0xffffffff)
 def sh(a,v):struct.pack_into('<H',ram,a&0x1fffff,v&65535)
 x=rng.choice([0,64,128,1,63,-1,rng.randrange(-32768,32768)]);y=rng.randrange(-32768,32768);width=rng.choice([0,64,320,-1,32767,rng.randrange(-32768,32768)]);height=rng.randrange(-32768,32768);lw=rng.choice([0,1,64,1024,32767,-32768,-1]);lh=rng.choice([0,1,64,512,32767,-32768,-1]);color=rng.getrandbits(32);status=rng.getrandbits(32);area=[rng.getrandbits(20),rng.getrandbits(20),rng.getrandbits(22)]
 for j,v in enumerate((x,y,width,height)):sh(off+2*j,v)
 sh(0x95750,lw);sh(0x95752,lh);sw(0x95854,0x80141000);sw(0x141000,status);native=bytearray(ram);calls=[]
 regs=execute(ram,0x80076434,(rect,color),stop_at=(0x80076be0,0x80076b98))
 for _ in range(6):
  if not regs[31]:break
  ret=regs[31];fn=0x80000000|((struct.unpack_from('<I',ram,(ret-8)&0x1fffff)[0]&0x3ffffff)<<2);calls.append((fn,regs[4],regs[5] if fn==0x80076b98 else 0));regs[2]=area[regs[4]-3] if fn==0x80076be0 else 0;regs=execute(ram,ret,initial_regs=dict(enumerate(regs)),stop_at=(0x80076be0,0x80076b98))
 else:raise AssertionError('callback budget')
 nc=[]
 def ld(a,k):return int.from_bytes(native[a&0x1fffff:(a&0x1fffff)+k],'little')
 def st(a,v,k):off=a&0x1fffff;native[off:off+k]=(v&((1<<(k*8))-1)).to_bytes(k,'little')
 def call(fn,a,b):nc.append((fn,a,b));return area[a-3] if fn==0x80076be0 else 0
 result=lib.draft(rect,color,L(ld),S(st),C(call));counts['fast' if len(calls)==1 else 'general']+=1
 if result!=(regs[2]&0xffffffff) or calls!=nc or native[0xa3300:0xa3338]!=ram[0xa3300:0xa3338] or native[off:off+8]!=ram[off:off+8]:
  bad.append(dict(case=i,input=[x,y,width,height,lw,lh,hex(color),hex(status)],original_calls=calls,model_calls=nc,original_packet=ram[0xa3300:0xa3338].hex(),model_packet=native[0xa3300:0xa3338].hex(),original_rect=ram[off:off+8].hex(),model_rect=native[off:off+8].hex(),rect_address=hex(rect)))
  if len(bad)>=3:break
print(json.dumps(dict(requested=n,compared=i+1,mismatches=len(bad),coverage=counts,examples=bad),indent=2));sys.exit(bool(bad))
