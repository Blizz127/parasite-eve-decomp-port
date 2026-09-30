#!/usr/bin/env python3
"""Independent static B54K-AN CdlReadS delivery audit."""
from pathlib import Path
import hashlib, struct, sys

def need(ok, msg):
    if not ok: raise SystemExit("FAIL: " + msg)

def u32(b,o): return struct.unpack_from("<I",b,o)[0]

def main():
    root=Path(__file__).resolve().parents[2]
    exe_path=Path(sys.argv[1]) if len(sys.argv)>1 else root/"build/extracted/disc1/SLUS_006.62"
    b=exe_path.read_bytes()
    need(hashlib.sha1(b).hexdigest()=="452fb033f2eaa4b18aa20a5bca60b8125af3a37b","EXE identity")
    spans=((0x71BE8,0x71C08,"0a54e2df91311e97e5ae88a9dc0b0ca485386c20ae1bb2a109b83ae193f7edd0"),
           (0x6CD64,0x6D680,"7c4ca8096a8921313e5b9a5f036f2b9341d656633afed2132b62af8d40aa8c74"),
           (0x6CA14,0x6CAA0,"bfe0812b6e5d671c9b90c55f63306291ed8a733953e57a1378930757146bc775"))
    for s,e,h in spans: need(hashlib.sha256(b[s:e]).hexdigest()==h,f"span {s:x}")
    cd=b[0x6CD64:0x6D680]
    # Calls to the generic copier, DMA issuer, record clear, and DMA callback.
    for insn in (0x0C01F3A0,0x0C01F3AB,0x0C01F111,0x0C01F085):
        need(insn.to_bytes(4,"little") in cd,f"callback call {insn:08x}")
    need((0x24020003).to_bytes(4,"little") in cd and
         (0xA4620000).to_bytes(4,"little") in cd,
         "record status 3 publication")
    dma=b[0x6CA14:0x6CAA0]
    need((0x24020002).to_bytes(4,"little") in dma and
         (0xA4620000).to_bytes(4,"little") in dma,
         "DMA callback status 2 publication")
    src=(root/"pc_port/game/boot/func_80081314_port.c").read_text()
    libcd=(root/"pc_port/platform/pe_libcd.c").read_text()
    need("func_80081314_func_8007F0C8_cut" not in src and
         "func_8007F0C8(mode & 0xFFu, location, 0x1Bu, 0u" in src and
         "func_8007F0C8_completion_selector" in libcd and
         "func_8007C214(" not in src and "func_800813E8(" not in src,
         "native boundary or fabricated callback")
    print("  OK retail: 583-word CD parser and distinct 35-word DMA callback")
    print("  OK ordering: status 3 precedes conditional status-2 callback")
    print("  OK native: no sector delivery or callback invocation shortcut")
    print("\nB54K-AN CdlReadS delivery audit: PASS.")

if __name__=="__main__": main()
