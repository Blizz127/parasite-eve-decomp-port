#!/usr/bin/env python3
"""Compare original readback events with a finite test-only native model."""
from pathlib import Path
import ctypes,json,os,random,sys
root=Path(__file__).resolve().parents[2];os.environ['PE_EXE_PATH']=str(root/'build/disc1.candidate.exe');sys.path.insert(0,str(root/'pc_port/tools'))
from pe_store_image_oracle import ReadMachine,pixel
from b53e_oracle import Exe,HardwareEvent,callee_script,seed_rect,RECT,LIMIT_W,LIMIT_H,WAIT_DEADLINE,WAIT_COUNTER,GP0,GPUSTAT_GP1,DMA2_MADR,DMA2_BCR,DMA2_CHCR
exe=Exe(root/'build/disc1.candidate.exe');lib=ctypes.CDLL(sys.argv[1]);L=ctypes.CFUNCTYPE(ctypes.c_uint32,ctypes.c_uint32,ctypes.c_uint32);S=ctypes.CFUNCTYPE(None,ctypes.c_uint32,ctypes.c_uint32,ctypes.c_uint32);H=ctypes.CFUNCTYPE(ctypes.c_uint32,ctypes.c_uint32,ctypes.c_uint32,ctypes.c_uint32);lib.draft.argtypes=[ctypes.c_uint32,ctypes.c_uint32,L,S,H];lib.draft.restype=ctypes.c_int
rng=random.Random(0x768a0d34);n=int(sys.argv[2]);start=int(sys.argv[3]) if len(sys.argv)>3 else 0;bad=[];coverage={'cpu_only':0,'dma_only':0,'mixed':0}
for i in range(start+n):
 w=rng.randrange(1,129);h=rng.randrange(1,9);x=rng.randrange(1024);y=rng.randrange(512);pixels=w*h;words=(pixels+1)//2;blocks,prefix=divmod(words,16);dst=0x80140000
 if i<start:continue
 values=[pixel(j*2)|((pixel(j*2+1) if j*2+1<pixels else 0)<<16) for j in range(prefix)];events=[]
 def e(kind,pc,address,value):events.append(HardwareEvent(kind,pc,address,value))
 e('read',0x8007698c,GPUSTAT_GP1,0x04000000);e('write',0x800769e0,GPUSTAT_GP1,0x04000000);e('write',0x800769f0,GP0,0x01000000);e('write',0x80076a00,GP0,0xc0000000);e('write',0x80076a14,GP0,(y<<16)|x);e('write',0x80076a28,GP0,(h<<16)|w);e('read',0x80076a38,GPUSTAT_GP1,0x0c000000)
 for v in values:e('read',0x80076a9c,GP0,v)
 if blocks:e('write',0x80076ac4,GPUSTAT_GP1,0x04000003);e('write',0x80076ad4,DMA2_MADR,dst+prefix*4);e('write',0x80076ae8,DMA2_BCR,(blocks<<16)|16);e('write',0x80076af8,DMA2_CHCR,0x01000200)
 m=ReadMachine(exe,events,callee_script(17));m.seed(LIMIT_W,2,1024);m.seed(LIMIT_H,2,512);m.seed(WAIT_DEADLINE,4,0xdddddddd);m.seed(WAIT_COUNTER,4,0xeeeeeeee);seed_rect(m,x,y,w,h);m.seed_words(dst,[0xa5a5a5a5]*16);ram=bytearray(m.ram);result=m.run(RECT,dst,entry=0x800768a0);m.assert_scripts_consumed();ev=[];status=[0];cursor=[0]
 def ld(a,k):return int.from_bytes(ram[a-0x80000000:a-0x80000000+k],'little')
 def st(a,v,k):off=a-0x80000000;ram[off:off+k]=(v&((1<<(8*k))-1)).to_bytes(k,'little')
 def hw(kind,a,v):
  if kind:r=v
  elif a==GPUSTAT_GP1:r=0x04000000 if status[0]==0 else 0x0c000000;status[0]+=1
  else:r=values[cursor[0]];cursor[0]+=1
  ev.append(('write' if kind else 'read',a,r));return r
 got=lib.draft(RECT,dst,L(ld),S(st),H(hw));expect=[(e.kind,e.address,e.value) for e in m.hardware_observed];coverage['cpu_only' if not blocks else 'dma_only' if not prefix else 'mixed']+=1
 if got!=result or ev!=expect or ram[RECT-0x80000000:RECT-0x80000000+8]!=m.ram[RECT-0x80000000:RECT-0x80000000+8] or ram[dst-0x80000000:dst-0x80000000+64]!=m.ram[dst-0x80000000:dst-0x80000000+64] or ld(WAIT_DEADLINE,4)!=m.peek(WAIT_DEADLINE,4) or ld(WAIT_COUNTER,4)!=m.peek(WAIT_COUNTER,4):bad.append(dict(case=i,input=[x,y,w,h],original=expect,model=ev,result=[result,got]));break
print(json.dumps(dict(requested=n,compared=i-start+1,mismatches=len(bad),coverage=coverage,examples=bad),indent=2));sys.exit(bool(bad))
