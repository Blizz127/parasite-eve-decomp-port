#!/usr/bin/env python3
"""Compare nonpositive original upload paths with a test-only native model."""
from pathlib import Path
import ctypes,json,os,random,sys
root=Path(__file__).resolve().parents[2];os.environ['PE_EXE_PATH']=str(root/'build/disc1.candidate.exe');sys.path.insert(0,str(root/'pc_port/tools'))
from b53e_oracle import Exe,make_machine,RECT,WAIT_DEADLINE,WAIT_COUNTER
exe=Exe(root/'build/disc1.candidate.exe');lib=ctypes.CDLL(sys.argv[1]);L=ctypes.CFUNCTYPE(ctypes.c_uint32,ctypes.c_uint32,ctypes.c_uint32);S=ctypes.CFUNCTYPE(None,ctypes.c_uint32,ctypes.c_uint32,ctypes.c_uint32);H=ctypes.CFUNCTYPE(ctypes.c_uint32,ctypes.c_uint32,ctypes.c_uint32,ctypes.c_uint32);lib.draft.argtypes=[ctypes.c_uint32,ctypes.c_uint32,L,S,H];lib.draft.restype=ctypes.c_int
rng=random.Random(0x76664000);bad=[]
for i in range(500):
 x=rng.randrange(-32768,32768);y=rng.randrange(-32768,32768);kind=i%3
 if kind==0:w=rng.randrange(-32768,0);h=rng.randrange(1,513);lw=rng.randrange(1,1025);lh=512
 elif kind==1:w=rng.randrange(1,1025);h=rng.randrange(-32768,0);lw=1024;lh=rng.randrange(1,513)
 else:w=rng.randrange(1,1025);h=rng.randrange(1,513);lw=rng.randrange(-32768,0);lh=512
 m=make_machine(exe,rect=(x,y,w,h),hardware=[],width_limit=lw&0xffff,height_limit=lh,source=0x80110000);ram=bytearray(m.ram);result=m.run(RECT,0x80110000);m.assert_scripts_consumed();events=[]
 def ld(a,k):return int.from_bytes(ram[a-0x80000000:a-0x80000000+k],'little')
 def st(a,v,k):off=a-0x80000000;ram[off:off+k]=(v&((1<<(8*k))-1)).to_bytes(k,'little')
 def hw(kind,a,v):events.append((kind,a,v));return 0
 got=lib.draft(RECT,0x80110000,L(ld),S(st),H(hw))
 if (got&0xffffffff)!=result or events or ram[RECT-0x80000000:RECT-0x80000000+8]!=m.ram[RECT-0x80000000:RECT-0x80000000+8] or ld(WAIT_DEADLINE,4)!=m.peek(WAIT_DEADLINE,4) or ld(WAIT_COUNTER,4)!=m.peek(WAIT_COUNTER,4):bad.append((i,(x,y,w,h,lw,lh),result,got,events));break
print(json.dumps({'requested':500,'compared':i+1,'mismatches':len(bad),'examples':bad},indent=2));sys.exit(bool(bad))
