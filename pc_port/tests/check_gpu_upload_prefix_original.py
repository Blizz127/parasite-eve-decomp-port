#!/usr/bin/env python3
"""Compare native worker prefix to a bounded original worker span."""
import argparse
import ctypes
import hashlib
import random
import struct
import sys
from pathlib import Path

root=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(root/'pc_port/tools'))
from pe_battle_hud_oracle import execute
p=argparse.ArgumentParser(description=__doc__)
p.add_argument('library')
p.add_argument('exe')
a=p.parse_args()
with open(a.exe,'rb') as f:
    f.seek(0x66e64)
    subject=f.read(0x23c)
digest='79dd44e3819f51eb5928c9af3ec0d6906cc3d95765dc2718c3c10e49ef78e0f7'
if len(subject)!=0x23c or hashlib.sha256(subject).hexdigest()!=digest:
    raise SystemExit('worker span missing or digest mismatch')
lib=ctypes.CDLL(a.library)
lib.PE_RamInit.argtypes=[]
lib.PE_RamReset.argtypes=[]
lib.PE_RamDestroy.argtypes=[]
lib.PE_StoreU16.argtypes=[ctypes.c_uint32,ctypes.c_uint16]
lib.PE_StoreU32.argtypes=[ctypes.c_uint32,ctypes.c_uint32]
lib.PE_LoadU32.argtypes=[ctypes.c_uint32]
lib.PE_LoadU32.restype=ctypes.c_uint32
lib.UploadTest_Reset.argtypes=[]
lib.UploadTest_GP0Calls.restype=ctypes.c_uint
lib.UploadTest_DMACalls.restype=ctypes.c_uint
lib.UploadTest_DMABCR.restype=ctypes.c_uint32
lib.pe_plat_gpu_upload_worker_host.argtypes=[ctypes.c_uint32,ctypes.c_uint32]
lib.pe_plat_gpu_upload_worker_host.restype=ctypes.c_int
rng=random.Random(0x76664)
cases=[(0,0),(1,1),(31,1),(32,1),(33,1),(1024,512),(-1,4),(4,-1),(1100,600)]
cases += [(rng.randrange(-30,1201),rng.randrange(-30,601)) for _ in range(247)]
rect=0x801e0000
source=0x80040000
lib.PE_RamInit()
positive=negative=0
for case,(width,height) in enumerate(cases):
    ram=bytearray(0x200000)
    ram[0x76664:0x768a0]=subject
    # Replace the deadline JAL in the test copy with a synthetic immediate return.
    struct.pack_into('<I',ram,0x76688,0x0c000000 | ((0x80143000>>2)&0x03ffffff))
    struct.pack_into('<II',ram,0x143000,0x03e00008,0)
    struct.pack_into('<HHHH',ram,rect&0x1fffff,12,34,width&0xffff,height&0xffff)
    struct.pack_into('<HH',ram,0x95750,1024,512)
    lib.PE_RamReset()
    lib.PE_StoreU32(rect,(34<<16)|12)
    lib.PE_StoreU16(rect+4,width&0xffff)
    lib.PE_StoreU16(rect+6,height&0xffff)
    lib.PE_StoreU16(0x80095750,1024)
    lib.PE_StoreU16(0x80095752,512)
    lib.UploadTest_Reset()
    visited=set()
    regs=execute(ram,0x80076664,(rect,source),stop_at=(0x80076740,0x8007687c),
                 visited_pcs=visited,instruction_budget=160)
    if not all(0x80076664<=pc<0x80076740 or 0x80143000<=pc<0x80143008 for pc in visited):
        raise AssertionError(f'case {case}: left documented prefix or deadline stub')
    native=lib.pe_plat_gpu_upload_worker_host(rect,source)
    original_wh=struct.unpack_from('<I',ram,(rect&0x1fffff)+4)[0]
    if lib.PE_LoadU32(rect+4)!=original_wh:
        raise AssertionError(f'case {case}: clamped RECT differs')
    w=original_wh&0xffff;h=original_wh>>16
    words=(w*h+1)//2
    if words:
        positive+=1
        if native!=0 or regs[16]!=(words>>4):
            raise AssertionError(f'case {case}: block count or return mismatch')
        if lib.UploadTest_GP0Calls()!=4+(words&15):
            raise AssertionError(f'case {case}: scripted CPU-prefix count')
        if lib.UploadTest_DMACalls()!=int(words>=16):
            raise AssertionError(f'case {case}: scripted DMA count')
        if words>=16 and lib.UploadTest_DMABCR()!=((words>>4)<<16)|16:
            raise AssertionError(f'case {case}: scripted DMA BCR')
    else:
        negative+=1
        if native!=-1 or regs[2]!=0xffffffff or lib.UploadTest_GP0Calls()!=0 or lib.UploadTest_DMACalls()!=0:
            raise AssertionError(f'case {case}: no-op return mismatch')
lib.PE_RamDestroy()
print(f'PASS {len(cases)} bounded-original/native clamp and branch cases: {positive} positive, {negative} no-op; scripted GPU')
