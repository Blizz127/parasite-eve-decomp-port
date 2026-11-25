#!/usr/bin/env python3
"""Compare malformed upload span handling to a bounded original prefix."""
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
lib.pe_plat_gpu_upload_inline_host.argtypes=[ctypes.c_uint32,ctypes.c_uint32,ctypes.c_uint32]
lib.pe_plat_gpu_upload_inline_host.restype=ctypes.c_int
rng=random.Random(0x76664bad)
cases=[(-32768,-32768),(-256,-256)]
cases += [(rng.randrange(-32768,-255),rng.randrange(-32768,-255)) for _ in range(254)]
rect=0x801e0000
source=0x801ff000
lib.PE_RamInit()
for case,(limit_w,limit_h) in enumerate(cases):
    width,height=5,5
    ram=bytearray(0x200000)
    ram[0x76664:0x768a0]=subject
    # Deadline JAL is redirected only in this in-memory test image.
    struct.pack_into('<I',ram,0x76688,0x0c000000 | ((0x80143000>>2)&0x03ffffff))
    struct.pack_into('<II',ram,0x143000,0x03e00008,0)
    struct.pack_into('<HHHH',ram,rect&0x1fffff,12,34,width,height)
    struct.pack_into('<HH',ram,0x95750,limit_w&0xffff,limit_h&0xffff)
    visited=set()
    regs=execute(ram,0x80076664,(rect,source),stop_at=(0x80076740,0x8007687c),
                 visited_pcs=visited,instruction_budget=160)
    if not all(0x80076664<=pc<0x80076740 or 0x80143000<=pc<0x80143008 for pc in visited):
        raise AssertionError(f'case {case}: left bounded prefix or deadline stub')
    expected_words=(limit_w*limit_h+1)//2
    if (expected_words<=1024 or 0x80076730 not in visited or
        0x80076738 in visited or regs[16]!=(expected_words>>4)):
        raise AssertionError(f'case {case}: original prefix calculation mismatch')
    original_wh=struct.unpack_from('<I',ram,(rect&0x1fffff)+4)[0]
    lib.PE_RamReset()
    lib.PE_StoreU32(rect,(34<<16)|12)
    lib.PE_StoreU32(rect+4,(height<<16)|width)
    lib.PE_StoreU16(0x80095750,limit_w&0xffff)
    lib.PE_StoreU16(0x80095752,limit_h&0xffff)
    lib.UploadTest_Reset()
    direct=lib.pe_plat_gpu_upload_worker_host(rect,source)
    if direct!=-1 or lib.UploadTest_GP0Calls()!=0 or lib.UploadTest_DMACalls()!=0 or lib.PE_LoadU32(rect+4)!=original_wh:
        raise AssertionError(f'case {case}: direct safety rejection mismatch')
    lib.PE_StoreU32(rect+4,0xdeadbeef)
    lib.UploadTest_Reset()
    inline=lib.pe_plat_gpu_upload_inline_host((34<<16)|12,(height<<16)|width,source)
    if inline!=-1 or lib.UploadTest_GP0Calls()!=0 or lib.UploadTest_DMACalls()!=0 or lib.PE_LoadU32(rect+4)!=0xdeadbeef:
        raise AssertionError(f'case {case}: inline safety rejection mismatch')
lib.PE_RamDestroy()
print(f'PASS {len(cases)} double-negative original-prefix cases; both native entries reject out-of-RAM spans before GPU writes')
