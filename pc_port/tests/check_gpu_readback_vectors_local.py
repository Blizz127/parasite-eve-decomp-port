#!/usr/bin/env python3
"""Compare bounded readback instructions to previously recorded vectors."""
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
import re
line_re=re.compile(r"\s*\{\{([^}]*)\},\{([^}]*)\},(\d+),(\d+),(0x[0-9A-Fa-f]+)u,(\d+),\{([^}]*)\},\{([^}]*)\}\},")
def integers(text):
    return [int(x.strip().removesuffix('u'),0) for x in text.split(',')]
rows=[]
for line in (root/'pc_port/tests/retail_store_image_cases.h').read_text().splitlines():
    match=line_re.fullmatch(line)
    if not match:
        if line.lstrip().startswith('{{'): raise AssertionError('unparsed vector row')
        continue
    r,limits,w,h,result,count,prefix,dma=match.groups()
    rows.append((integers(r),integers(limits),int(w),int(h),int(result,16),int(count),integers(prefix),integers(dma)))
if len(rows)!=47: raise AssertionError('unexpected vector count')
dst=0x800d0000
for index,(rect,limits,w,h,result,count,prefix,dma) in enumerate(rows):
    x,y,_,_=rect;xy=(y&0xffff)<<16 | (x&0xffff)
    pixels=w*h;words=(pixels+1)//2;blocks=words//16
    if count!=words%16: raise AssertionError(f'case {index}: vector prefix count')
    values=prefix[:count]
    events=[]
    def ev(kind,pc,address,value):events.append(oracle.HardwareEvent(kind,pc,address,value))
    if words:
        ev('read',0x8007698c,oracle.GPUSTAT_GP1,0x04000000)
        ev('write',0x800769e0,oracle.GPUSTAT_GP1,0x04000000)
        ev('write',0x800769f0,oracle.GP0,0x01000000)
        ev('write',0x80076a00,oracle.GP0,0xc0000000)
        ev('write',0x80076a14,oracle.GP0,xy)
        ev('write',0x80076a28,oracle.GP0,(h<<16)|w)
        ev('read',0x80076a38,oracle.GPUSTAT_GP1,0x0c000000)
        for value in values: ev('read',0x80076a9c,oracle.GP0,value)
        if blocks:
            ev('write',0x80076ac4,oracle.GPUSTAT_GP1,0x04000003)
            ev('write',0x80076ad4,oracle.DMA2_MADR,dst+4*count)
            ev('write',0x80076ae8,oracle.DMA2_BCR,(blocks<<16)|16)
            ev('write',0x80076af8,oracle.DMA2_CHCR,0x01000200)
    m=ReadMachine(exe,events,oracle.callee_script(17))
    oracle.seed_rect(m,*rect)
    m.seed(oracle.LIMIT_W,2,limits[0]);m.seed(oracle.LIMIT_H,2,limits[1])
    for ptr,value in ((oracle.GP0_POINTER,oracle.GP0),(oracle.GPUSTAT_POINTER,oracle.GPUSTAT_GP1),
                      (oracle.DMA2_MADR_POINTER,oracle.DMA2_MADR),(oracle.DMA2_BCR_POINTER,oracle.DMA2_BCR),
                      (oracle.DMA2_CHCR_POINTER,oracle.DMA2_CHCR)):
        m.seed(ptr,4,value)
    m.seed_words(dst,[0xa5a5a5a5]*16)
    got=m.run(oracle.RECT,dst,entry=0x800768a0);m.assert_scripts_consumed()
    if any(not (0x800768a0<=pc<0x80076b20 or 0x800773d0<=pc<0x80077404) for pc in m.pc_counts):
        raise AssertionError(f'case {index}: execution outside the two bounded spans')
    if got!=result or m.peek(oracle.RECT+4,4)!=(h<<16)|w:
        raise AssertionError(f'case {index}: return or RECT mismatch')
    if any(m.peek(dst+4*i,4)!=(values[i] if i<count else 0xa5a5a5a5) for i in range(16)):
        raise AssertionError(f'case {index}: prefix write mismatch')
    seen=[next((e.value for e in reversed(m.hardware_observed) if e.kind=='write' and e.address==a),0)
          for a in (oracle.DMA2_MADR,oracle.DMA2_BCR,oracle.DMA2_CHCR)]
    if seen!=dma:
        raise AssertionError(f'case {index}: DMA mismatch')
print(f'PASS {len(rows)} recorded readback vectors against bounded local instructions; scripted GPU reads')
