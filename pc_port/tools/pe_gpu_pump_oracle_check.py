#!/usr/bin/env python3
"""Compare the compiled pump to the original executor with scripted devices."""
import argparse
import ctypes
import os
import pathlib
import random
parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--exe',required=True,type=pathlib.Path)
parser.add_argument('--library',required=True,type=pathlib.Path)
args=parser.parse_args()
os.environ['PE_EXE_PATH']=str(args.exe.resolve())
import b53h_oracle as r
rng=random.Random(0x76ee4002)
exe=r.Exe(pathlib.Path(os.environ['PE_EXE_PATH']))
r.verify_static(exe)
lib=ctypes.CDLL(str(args.library.resolve()))
lib.run_case.argtypes=[ctypes.c_uint32]*8+[ctypes.POINTER(ctypes.c_uint32)]
for n in range(600):
 consumer=rng.randrange(64);count=rng.randrange(6);busy=int(rng.randrange(4)==0);issue=rng.randrange(2);marker=rng.randrange(2);callback=0x80012345 if rng.randrange(2) else 0;mask=rng.randrange(65536);seed=rng.randrange(65536)
 entries={}
 for j in range(count):
  slot=(consumer+j)&63;base=r.RING+slot*r.RING_STRIDE
  entries[slot]=(r.CANONICAL_WORKER,base+12,0x80010000+((seed+j*0x100)&0xfffc))
 m=r.Machine(exe,chcr=0x201|(busy<<24),gpustat=r.GPUSTAT_READY,producer=(consumer+count)&63,consumer=consumer,entries=entries,work_marker=marker,drawsync_callback=callback,imask=mask,worker_starts_dma=bool(issue))
 result=m.run();out=(ctypes.c_uint32*202)();lib.run_case(consumer,count,busy,issue,marker,callback,mask,seed,out)
 expected=[result,1,m.get32(r.CONSUMER),m.imask,m.get32(r.WORK_MARKER),len(m.worker_calls),len(m.deregister_calls),len(m.callback_calls),int(bool(m.chcr&r.CHCR_BUSY))]
 for i,(a,b) in enumerate(zip(expected,out)):
  if a!=b:raise AssertionError(f'case {n} field {i} oracle={a} draft={b}')
 if not busy and out[9]!=m.get32(r.SAVED_IMASK_PUMP):raise AssertionError(f'case {n} saved mask')
 actual=[tuple(out[10+i*3:13+i*3]) for i in range(len(m.worker_calls))]
 if actual!=m.worker_calls:raise AssertionError(f'case {n} worker calls {actual} != {m.worker_calls}')
print('PASS 600 original-executor vs compiled-pump cases; ready GPU and scripted worker/callbacks')
