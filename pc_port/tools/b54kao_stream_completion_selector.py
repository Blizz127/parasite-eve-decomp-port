#!/usr/bin/env python3
"""Independent B54K-AO stream-completion selector census."""
from pathlib import Path
import hashlib, struct, sys

def need(x,m):
    if not x: raise SystemExit("FAIL: "+m)
def words(b,s,e): return [struct.unpack_from("<I",b,o)[0] for o in range(s,e,4)]

def direct_stores(b, hi, lo):
    out=[]
    for o in range(0,len(b)-4,4):
        w=struct.unpack_from("<I",b,o)[0]
        if w>>26 != 0x2B or (w&0xffff)!=lo: continue
        base=(w>>21)&31
        for d in range(1,9):
            p=o-4*d
            if p<0: break
            q=struct.unpack_from("<I",b,p)[0]
            if q>>26==0x0F and ((q>>16)&31)==base and (q&0xffff)==hi:
                out.append((o,w)); break
    return out

def main():
    root=Path(__file__).resolve().parents[2]
    exe=Path(sys.argv[1]).read_bytes(); peimg=Path(sys.argv[2]).read_bytes()
    need(hashlib.sha1(exe).hexdigest()=="452fb033f2eaa4b18aa20a5bca60b8125af3a37b","EXE")
    need(hashlib.sha1(peimg).hexdigest()=="146c0ce7308bf9fdc2ba5a84230e198db0663f3b","PE.IMG")
    need(direct_stores(exe,0x800C,0x0DB8)==[(0x6CB34,0xAC200DB8)],"executable selector writers")
    need(direct_stores(peimg,0x800C,0x0DB8)==[],"PE.IMG selector writers")
    cd=words(exe,0x6CD64,0x6D680)
    for w in (0x8C4289F4,0x1044023E,0xAC2389F4,0x8C630DB8,
              0x10600008,0x0C01F085):
        need(w in cd,f"selector/lifecycle word {w:08x}")
    dma=words(exe,0x6CA14,0x6CAA0)
    need(0xAC2089F4 in dma,"DMA callback clear")
    print("  OK census: one executable zero writer; zero PE.IMG writers")
    print("  OK production: CD tail skips direct callback; DMA3 clears busy")
    print("\nB54K-AO stream-completion selector: PASS.")
if __name__=="__main__": main()
