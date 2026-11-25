#!/usr/bin/env python3
"""Compare original upload events with a finite test-only native model."""
from pathlib import Path
import ctypes,json,os,random,sys
root=Path(__file__).resolve().parents[2];os.environ['PE_EXE_PATH']=str(root/'build/disc1.candidate.exe');sys.path.insert(0,str(root/'pc_port/tools'))
from b53e_oracle import Exe,make_machine,issue_hardware,RECT,GPUSTAT_READY_GP0,WAIT_DEADLINE,WAIT_COUNTER
exe=Exe(root/'build/disc1.candidate.exe');lib=ctypes.CDLL(sys.argv[1]);L=ctypes.CFUNCTYPE(ctypes.c_uint32,ctypes.c_uint32,ctypes.c_uint32);S=ctypes.CFUNCTYPE(None,ctypes.c_uint32,ctypes.c_uint32,ctypes.c_uint32);H=ctypes.CFUNCTYPE(ctypes.c_uint32,ctypes.c_uint32,ctypes.c_uint32,ctypes.c_uint32);lib.draft.argtypes=[ctypes.c_uint32,ctypes.c_uint32,L,S,H];lib.draft.restype=ctypes.c_int
rng=random.Random(0x76664d34);n=int(sys.argv[2]);start=int(sys.argv[3]) if len(sys.argv)>3 else 0;bad=[];coverage={'cpu_only':0,'dma_only':0,'mixed':0}
for i in range(start+n):
 w=rng.randrange(1,129);h=rng.randrange(1,9);x=rng.randrange(1024);y=rng.randrange(512);pixels=w*h;words=(pixels+1)//2;blocks,prefix=divmod(words,16);source=0x80110000;values=[rng.getrandbits(32) for _ in range(prefix)];
 if i<start:continue
 xy=(y<<16)|x;wh=(h<<16)|w;events=issue_hardware([GPUSTAT_READY_GP0],xy,wh,values,blocks,source+4*prefix)
 m=make_machine(exe,rect=(x,y,w,h),hardware=events,source=source,source_words=values);ram=bytearray(m.ram);result=m.run(RECT,source);m.assert_scripts_consumed();ev=[];reads=[]
 def ld(a,k):
  v=int.from_bytes(ram[a-0x80000000:a-0x80000000+k],'little')
  if source<=a<source+4*prefix:reads.append((a,v))
  return v
 def st(a,v,k):off=a-0x80000000;ram[off:off+k]=(v&((1<<(8*k))-1)).to_bytes(k,'little')
 def hw(kind,a,v):ev.append(('write' if kind else 'read',a,v if kind else GPUSTAT_READY_GP0));return GPUSTAT_READY_GP0 if not kind else 0
 got=lib.draft(RECT,source,L(ld),S(st),H(hw));expect=[(e.kind,e.address,e.value) for e in m.hardware_observed];coverage['cpu_only' if not blocks else 'dma_only' if not prefix else 'mixed']+=1
 if got!=result or ev!=expect or reads!=[(source+4*j,v) for j,v in enumerate(values)] or ram[RECT-0x80000000:RECT-0x80000000+8]!=m.ram[RECT-0x80000000:RECT-0x80000000+8] or int.from_bytes(ram[WAIT_DEADLINE-0x80000000:WAIT_DEADLINE-0x80000000+4],'little')!=m.peek(WAIT_DEADLINE,4) or int.from_bytes(ram[WAIT_COUNTER-0x80000000:WAIT_COUNTER-0x80000000+4],'little')!=m.peek(WAIT_COUNTER,4):
  bad.append(dict(case=i,input=[x,y,w,h],expected=expect,model=ev,result=[result,got],reads=reads));break
print(json.dumps(dict(requested=n,compared=i-start+1,mismatches=len(bad),coverage=coverage,examples=bad),indent=2));sys.exit(bool(bad))
