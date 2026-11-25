#!/usr/bin/env python3
"""Compare native readback events to bounded local executable instructions."""
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
# The readback digest is observed from the historically verified local file;
# no independent prior pin was found. It guards reproducibility, not identity.
spans=((0x800768a0,0x670a0,0x280,'3b36dbbc6d8c79880186b5b2a7e7864ec0c06056320177b9239ea2e658052f8c'),
       (0x800773d0,0x67bd0,0x34,'3b0637e8b6472855bf28115f4712ea1f43c3d3ad130c388ab71c8101d3d00cb5'))
subjects={}
with open(a.exe,'rb') as f:
    for address,offset,size,digest in spans:
        f.seek(offset);data=f.read(size)
        if len(data)!=size or hashlib.sha256(data).hexdigest()!=digest:
            raise SystemExit('documented span missing or digest mismatch')
        subjects[address]=data
# b53e_oracle imports the unused upload word list at import time. Supply only
# invented zero words there and the separately verified deadline span; the
# synthetic upload region is never executed. This avoids full-EXE reads.
provider=types.ModuleType('pe_exe_words')
def bounded_words(address,count):
    if address==0x80076664 and count==143: return [0]*count
    data=subjects.get(address)
    if data is None or len(data)!=count*4: raise AssertionError('out-of-scope instruction request')
    return list(struct.unpack('<'+'I'*count,data))
provider.exe_words=bounded_words
sys.modules['pe_exe_words']=provider
import b53e_oracle as oracle
from pe_store_image_oracle import ReadMachine
class BoundedImage:
    taddr=0x80010000
    tsize=0x1f0000
    image=bytearray(tsize)
exe=BoundedImage()
for address,data in subjects.items():
    start=address-exe.taddr;exe.image[start:start+len(data)]=data
lib=ctypes.CDLL(a.library)
lib.PE_RamInit.argtypes=[];lib.PE_RamReset.argtypes=[];lib.PE_RamDestroy.argtypes=[]
lib.PE_StoreU16.argtypes=[ctypes.c_uint32,ctypes.c_uint16]
lib.PE_StoreU32.argtypes=[ctypes.c_uint32,ctypes.c_uint32]
lib.PE_LoadU32.argtypes=[ctypes.c_uint32];lib.PE_LoadU32.restype=ctypes.c_uint32
lib.ReadEvents_Reset.argtypes=[];lib.ReadEvents_AddValue.argtypes=[ctypes.c_uint32]
lib.ReadEvents_Count.restype=ctypes.c_uint
for fn in (lib.ReadEvents_Kind,lib.ReadEvents_Address,lib.ReadEvents_Value):
    fn.argtypes=[ctypes.c_uint];fn.restype=ctypes.c_uint32
lib.pe_plat_gpu_readback_worker_host.argtypes=[ctypes.c_uint32,ctypes.c_uint32]
lib.pe_plat_gpu_readback_worker_host.restype=ctypes.c_int
lib.pe_plat_gpu_readback_inline_host.argtypes=[ctypes.c_uint32,ctypes.c_uint32,ctypes.c_uint32]
lib.pe_plat_gpu_readback_inline_host.restype=ctypes.c_int
rng=random.Random(0x768a0ee)
shapes=[(1,1),(32,1),(33,1),(1100,3),(4,600)]+[(rng.randrange(1,129),rng.randrange(1,9)) for _ in range(251)]
dst=0x800d0000
coverage={'cpu_only':0,'dma_only':0,'mixed':0}
lib.PE_RamInit()
for index,(w,h) in enumerate(shapes):
    x,y=rng.randrange(800),rng.randrange(400)
    cw,ch=min(w,1024),min(h,512)
    words=(cw*ch+1)//2;blocks,prefix=divmod(words,16)
    values=[rng.getrandbits(32) for _ in range(prefix)]
    xy=(y<<16)|x;wh=(h<<16)|w;clamped=(ch<<16)|cw
    events=[]
    def ev(kind,pc,address,value):events.append(oracle.HardwareEvent(kind,pc,address,value))
    ev('read',0x8007698c,oracle.GPUSTAT_GP1,0x04000000)
    ev('write',0x800769e0,oracle.GPUSTAT_GP1,0x04000000)
    ev('write',0x800769f0,oracle.GP0,0x01000000)
    ev('write',0x80076a00,oracle.GP0,0xc0000000)
    ev('write',0x80076a14,oracle.GP0,xy)
    ev('write',0x80076a28,oracle.GP0,clamped)
    ev('read',0x80076a38,oracle.GPUSTAT_GP1,0x0c000000)
    for v in values: ev('read',0x80076a9c,oracle.GP0,v)
    if blocks:
        ev('write',0x80076ac4,oracle.GPUSTAT_GP1,0x04000003)
        ev('write',0x80076ad4,oracle.DMA2_MADR,dst+4*prefix)
        ev('write',0x80076ae8,oracle.DMA2_BCR,(blocks<<16)|16)
        ev('write',0x80076af8,oracle.DMA2_CHCR,0x01000200)
    m=ReadMachine(exe,events,oracle.callee_script(17))
    oracle.seed_rect(m,x,y,w,h)
    m.seed(oracle.LIMIT_W,2,1024);m.seed(oracle.LIMIT_H,2,512)
    for ptr,value in ((oracle.GP0_POINTER,oracle.GP0),(oracle.GPUSTAT_POINTER,oracle.GPUSTAT_GP1),
                      (oracle.DMA2_MADR_POINTER,oracle.DMA2_MADR),(oracle.DMA2_BCR_POINTER,oracle.DMA2_BCR),
                      (oracle.DMA2_CHCR_POINTER,oracle.DMA2_CHCR)):
        m.seed(ptr,4,value)
    m.seed_words(dst,[0xa5a5a5a5]*16)
    result=m.run(oracle.RECT,dst,entry=0x800768a0);m.assert_scripts_consumed()
    if any(not (0x800768a0<=pc<0x80076b20 or 0x800773d0<=pc<0x80077404) for pc in m.pc_counts):
        raise AssertionError(f'case {index}: execution outside the two bounded spans')
    original=[(int(e.kind=='write'),e.address,e.value) for e in m.hardware_observed]
    for entry in range(2):
        lib.PE_RamReset();lib.ReadEvents_Reset()
        lib.PE_StoreU32(oracle.RECT,xy);lib.PE_StoreU32(oracle.RECT+4,wh if entry==0 else 0xdeadbeef)
        lib.PE_StoreU16(oracle.LIMIT_W,1024);lib.PE_StoreU16(oracle.LIMIT_H,512)
        for i in range(16):lib.PE_StoreU32(dst+4*i,0xa5a5a5a5)
        for value in values:lib.ReadEvents_AddValue(value)
        got=(lib.pe_plat_gpu_readback_worker_host(oracle.RECT,dst) if entry==0 else
             lib.pe_plat_gpu_readback_inline_host(xy,wh,dst))
        actual=[(lib.ReadEvents_Kind(i),lib.ReadEvents_Address(i),lib.ReadEvents_Value(i)) for i in range(lib.ReadEvents_Count())]
        if result!=0 or got!=0 or actual!=original or lib.PE_LoadU32(oracle.RECT+4)!=(clamped if entry==0 else 0xdeadbeef):
            raise AssertionError(f'case {index} entry {entry}: return, events or RECT mismatch')
        if any(lib.PE_LoadU32(dst+4*i)!=m.peek(dst+4*i,4) for i in range(16)):
            raise AssertionError(f'case {index} entry {entry}: destination prefix mismatch')
    coverage['cpu_only' if not blocks else 'dma_only' if not prefix else 'mixed']+=1
lib.PE_RamDestroy()
print(f'PASS {len(shapes)} bounded local-original cases x 2 native readback entries: {coverage}; scripted ready GPU')
