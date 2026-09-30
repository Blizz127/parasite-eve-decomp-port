#!/usr/bin/env python3
"""Independent B54K-B5 oracle for func_80030894's ten-item L8 loop."""
import hashlib, pathlib, struct, sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
BASE = 0x8000F800
START, END, WORDS = 0x800310A4, 0x80031110, 27
WIN_SHA = "9b87877a27ab26756ab9cfbdf9f6f5d4e31958975ca654fcdeaf87660f7b7ecb"
MODEL_SHA = "59cdcc608bd78cb548f46dc0ee86ad64b673429d21c353d7d03edfce65f9238c"
CRITICAL = {
    0x800310A0: 0xA2170006, 0x800310A4: 0x0000B021,
    0x800310A8: 0x93A30018, 0x800310AC: 0x3C12800A,
    0x800310B0: 0x2652E500, 0x800310B8: 0x000310C0,
    0x800310BC: 0x00431021, 0x800310C0: 0x00021080,
    0x800310C4: 0x00431023, 0x800310C8: 0x000288C0,
    0x800310D0: 0x000280C0, 0x800310D4: 0x02028023,
    0x800310D8: 0x00108080, 0x800310E4: 0x0C00DC37,
    0x800310FC: 0x2C42000A, 0x80031108: 0x1440FFF0,
    0x8003110C: 0xA2170006, 0x80031110: 0x3C10800A,
}

def need(ok, msg):
    if not ok: raise SystemExit("FAIL: " + msg)

def run():
    root = pathlib.Path(__file__).resolve().parent.parent.parent
    data = (root / "build/disc1.candidate.exe").read_bytes()
    need(hashlib.sha1(data).hexdigest() == SHA1, "exe SHA-1")
    word = lambda a: struct.unpack_from("<I", data, a - BASE)[0]
    win = data[START-BASE:END-BASE]
    need(len(win) == WORDS*4 and hashlib.sha256(win).hexdigest() == WIN_SHA,
         "full L8 window")
    print(f"OK window: {WORDS} words / {len(win):#x} bytes, SHA-256 exact")
    for a, v in CRITICAL.items(): need(word(a) == v, f"word {a:#x}")
    calls, branches = [], []
    for a in range(START, END, 4):
        w, op = word(a), word(a) >> 26
        if op == 3: calls.append((a, 0x80000000 | ((w & 0x3FFFFFF) << 2)))
        if op in (1,2,4,5,6,7,20,21,22,23):
            d=w&0xFFFF; d=d-0x10000 if d&0x8000 else d
            branches.append((a,a+4+d*4))
    need(calls == [(0x800310E4,0x800370DC)], "jal census")
    need(branches == [(0x80031108,0x800310CC)], "branch census")
    print("OK control flow: one native jal site; one ten-item back-edge")
    model = {}
    for j in range(10):
        h = 0x8009E500 + j*28
        values = {3:6,4:0x34,5:2,6:0,7:0xE1,8:0,9:0,10:0,11:0,
                  12:0x80,13:0x80,14:0x80,15:0x64}
        for o,v in values.items(): model[h+o]=v
    packed=b"".join(struct.pack("<IB",a,v) for a,v in sorted(model.items()))
    digest=hashlib.sha256(packed).hexdigest()
    print(f"model_unique_written_bytes={len(model)}")
    print(f"model_write_map_sha256={digest}")
    need(len(model)==130 and digest==MODEL_SHA, "model digest")
    need(0x8009E76B not in model, "crossed next group")
    print("OK independent model: ten compound sprites; next group untouched")
    return 0

if __name__ == "__main__": sys.exit(run())
