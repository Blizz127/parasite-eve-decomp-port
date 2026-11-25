#!/usr/bin/env python3
"""Compare actual native upload writes to bounded original instructions."""
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
rng=random.Random(0x76664ee)
shapes=[(1,1),(32,1),(33,1),(1100,3),(4,600)] + [(rng.randrange(1,129),rng.randrange(1,9)) for _ in range(251)]
source=0x80110000
coverage={'cpu_only':0,'dma_only':0,'mixed':0}
lib.PE_RamInit()
for index,(w,h) in enumerate(shapes):
    x,y=rng.randrange(800),rng.randrange(400)
    clamped_w,clamped_h=min(w,1024),min(h,512)
    words=(clamped_w*clamped_h+1)//2
    blocks,prefix=divmod(words,16)
    prefix_values=[rng.getrandbits(32) for _ in range(prefix)]
    rect0=(y<<16)|x
    rect1=(h<<16)|w
    clamped_rect1=(clamped_h<<16)|clamped_w
    hardware=oracle.issue_hardware([oracle.GPUSTAT_READY_GP0],rect0,clamped_rect1,prefix_values,blocks,source+prefix*4)
    machine=oracle.make_machine(exe,rect=(x,y,w,h),hardware=hardware,source=source,source_words=prefix_values)
    for pointer,value in ((oracle.GP0_POINTER,oracle.GP0),(oracle.GPUSTAT_POINTER,oracle.GPUSTAT_GP1),
                          (oracle.DMA2_MADR_POINTER,oracle.DMA2_MADR),(oracle.DMA2_BCR_POINTER,oracle.DMA2_BCR),
                          (oracle.DMA2_CHCR_POINTER,oracle.DMA2_CHCR)):
        machine.seed(pointer,4,value)
    result=machine.run(oracle.RECT,source)
    machine.assert_scripts_consumed()
    if any(not (0x80076664<=pc<0x800768a0 or 0x800773d0<=pc<0x80077404) for pc in machine.pc_counts):
        raise AssertionError(f'case {index}: original execution outside the two bounded spans')
    original=[(e.address,e.value) for e in machine.hardware_observed if e.kind=='write']
    lib.PE_RamReset()
    lib.UploadEvents_Reset()
    lib.PE_StoreU32(oracle.RECT,rect0)
    lib.PE_StoreU16(oracle.RECT+4,w)
    lib.PE_StoreU16(oracle.RECT+6,h)
    lib.PE_StoreU16(oracle.LIMIT_W,1024)
    lib.PE_StoreU16(oracle.LIMIT_H,512)
    for i,value in enumerate(prefix_values): lib.PE_StoreU32(source+4*i,value)
    got=lib.pe_plat_gpu_upload_worker_host(oracle.RECT,source)
    native=[(lib.UploadEvents_Address(i),lib.UploadEvents_Value(i)) for i in range(lib.UploadEvents_Count())]
    if result!=0 or got!=0 or original!=native or lib.PE_LoadU32(oracle.RECT+4)!=machine.peek(oracle.RECT+4,4):
        raise AssertionError(f'case {index}: native/original event or RECT mismatch')
    # The inline entry takes RECT words by value; unrelated guest RECT remains unchanged.
    lib.PE_StoreU32(oracle.RECT+4,0xdeadbeef)
    lib.UploadEvents_Reset()
    inline_result=lib.pe_plat_gpu_upload_inline_host(rect0,rect1,source)
    inline=[(lib.UploadEvents_Address(i),lib.UploadEvents_Value(i)) for i in range(lib.UploadEvents_Count())]
    if inline_result!=0 or inline!=original or lib.PE_LoadU32(oracle.RECT+4)!=0xdeadbeef:
        raise AssertionError(f'case {index}: inline/original event or writeback mismatch')
    coverage['cpu_only' if not blocks else 'dma_only' if not prefix else 'mixed']+=1
lib.PE_RamDestroy()
print(f'PASS {len(shapes)} bounded-original cases x 2 native entries: {coverage}; 2 clamped; ready GPU and VSync scripted')
