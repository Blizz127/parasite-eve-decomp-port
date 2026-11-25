#!/usr/bin/env python3
"""Compare nonpositive native upload cases to bounded original instructions."""
import argparse
import ctypes
import hashlib
import random
import struct
import sys
import types
from pathlib import Path
root=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(root/'pc_port/tools'))
p=argparse.ArgumentParser(description=__doc__)
p.add_argument('library')
p.add_argument('exe')
a=p.parse_args()
spans=((0x80076664,0x66e64,0x23c,'79dd44e3819f51eb5928c9af3ec0d6906cc3d95765dc2718c3c10e49ef78e0f7'),
       (0x800773d0,0x67bd0,0x34,'3b0637e8b6472855bf28115f4712ea1f43c3d3ad130c388ab71c8101d3d00cb5'))
subjects={}
with open(a.exe,'rb') as f:
    for address,offset,size,digest in spans:
        f.seek(offset)
        data=f.read(size)
        if len(data)!=size or hashlib.sha256(data).hexdigest()!=digest:
            raise SystemExit('documented span missing or digest mismatch')
        subjects[address]=data
# The prior oracle's default provider reads the entire executable. Supply only
# the two validated spans; reject any attempt to request other original bytes.
provider=types.ModuleType('pe_exe_words')
def bounded_words(address,count):
    data=subjects.get(address)
    if data is None or len(data)!=count*4:
        raise AssertionError('original instruction request outside bounded spans')
    return list(struct.unpack('<'+'I'*count,data))
provider.exe_words=bounded_words
sys.modules['pe_exe_words']=provider
import b53e_oracle as oracle
class BoundedImage:
    taddr=0x80010000
    tsize=0x1f0000
    image=bytearray(tsize)
exe=BoundedImage()
for address,data in subjects.items():
    start=address-exe.taddr
    exe.image[start:start+len(data)]=data
lib=ctypes.CDLL(a.library)
lib.PE_RamInit.argtypes=[]
lib.PE_RamReset.argtypes=[]
lib.PE_RamDestroy.argtypes=[]
lib.PE_StoreU16.argtypes=[ctypes.c_uint32,ctypes.c_uint16]
lib.PE_StoreU32.argtypes=[ctypes.c_uint32,ctypes.c_uint32]
lib.PE_LoadU32.argtypes=[ctypes.c_uint32]
lib.PE_LoadU32.restype=ctypes.c_uint32
lib.UploadEvents_Reset.argtypes=[]
lib.UploadEvents_Count.restype=ctypes.c_uint
lib.UploadEvents_Address.argtypes=[ctypes.c_uint]
lib.UploadEvents_Address.restype=ctypes.c_uint32
lib.UploadEvents_Value.argtypes=[ctypes.c_uint]
lib.UploadEvents_Value.restype=ctypes.c_uint32
lib.pe_plat_gpu_upload_worker_host.argtypes=[ctypes.c_uint32,ctypes.c_uint32]
lib.pe_plat_gpu_upload_worker_host.restype=ctypes.c_int
lib.pe_plat_gpu_upload_inline_host.argtypes=[ctypes.c_uint32,ctypes.c_uint32,ctypes.c_uint32]
lib.pe_plat_gpu_upload_inline_host.restype=ctypes.c_int
rng=random.Random(0x7666400)
edge=[-32768,-1025,-1,0,1,2,511,512,1024,32767]
def clamp(v,limit):
    return 0 if v<0 else min(v,limit)
cases=[]
while len(cases)<512:
    w,h,lw,lh=(rng.choice(edge) if rng.randrange(2) else rng.randrange(-32768,32768) for _ in range(4))
    if clamp(w,lw)*clamp(h,lh)<=0:
        cases.append((w,h,lw,lh))
source=0x80110000
negative_limits=0
lib.PE_RamInit()
for index,(w,h,lw,lh) in enumerate(cases):
    x,y=rng.randrange(1024),rng.randrange(512)
    xy=(y<<16)|x
    wh=((h&0xffff)<<16)|(w&0xffff)
    machine=oracle.make_machine(exe,rect=(x,y,w,h),hardware=[],source=source,
                                width_limit=lw&0xffff,height_limit=lh&0xffff)
    result=machine.run(oracle.RECT,source)
    machine.assert_scripts_consumed()
    if any(not (0x80076664<=pc<0x800768a0 or 0x800773d0<=pc<0x80077404) for pc in machine.pc_counts):
        raise AssertionError(f'case {index}: original execution outside the two bounded spans')
    if result!=0xffffffff or machine.hardware_observed:
        raise AssertionError(f'case {index}: original was not no-op')
    lib.PE_RamReset()
    lib.UploadEvents_Reset()
    lib.PE_StoreU32(oracle.RECT,xy)
    lib.PE_StoreU32(oracle.RECT+4,wh)
    lib.PE_StoreU16(oracle.LIMIT_W,lw&0xffff)
    lib.PE_StoreU16(oracle.LIMIT_H,lh&0xffff)
    direct=lib.pe_plat_gpu_upload_worker_host(oracle.RECT,source)
    if direct!=-1 or lib.UploadEvents_Count()!=0 or lib.PE_LoadU32(oracle.RECT+4)!=machine.peek(oracle.RECT+4,4):
        raise AssertionError(f'case {index}: direct return, RECT or event mismatch')
    lib.PE_StoreU32(oracle.RECT+4,0xdeadbeef)
    lib.UploadEvents_Reset()
    inline=lib.pe_plat_gpu_upload_inline_host(xy,wh,source)
    if inline!=-1 or lib.UploadEvents_Count()!=0 or lib.PE_LoadU32(oracle.RECT+4)!=0xdeadbeef:
        raise AssertionError(f'case {index}: inline return, sentinel or event mismatch')
    negative_limits+=int(lw<0 or lh<0)
lib.PE_RamDestroy()
print(f'PASS {len(cases)} bounded-original nonpositive cases x 2 native entries; {negative_limits} with negative limit; no GPU writes')
