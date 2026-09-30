#!/usr/bin/env python3
"""Independent retail-memory oracle for func_80070D10 / func_80070D6C / func_80070DD0.

Phase 6E-B1/B2.  Models the exact MIPS semantics of the game's lagged-
Fibonacci RNG against the *real retail executable bytes* (PS-X EXE), which
must be supplied at runtime — never embedded or committed.

Usage:
  rng_oracle.py /path/to/disc1.candidate.exe      # PS-X EXE (SHA-1 checked)
  rng_oracle.py --dump-calls N                    # per-call table for N calls

The script is the reference; the C port (func_80070D6C_port.c) must match it
exactly when guest RAM holds the same exe bytes.
"""

import hashlib
import struct
import sys

RETAIL_SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"

GA_INDEX1 = 0x80070E04
GA_INDEX2 = 0x80070E08
GA_TABLE = 0x80070E0C

MASK = 0xFFFFFFFF


def u32(x):
    return x & MASK


def s32(x):
    x &= MASK
    return x - 0x100000000 if x >= 0x80000000 else x


class ExeImage:
    """Retail main executable mapped at its PS-X EXE taddr."""

    def __init__(self, path):
        with open(path, "rb") as f:
            data = f.read()
        sha1 = hashlib.sha1(data).hexdigest()
        if sha1 != RETAIL_SHA1:
            raise SystemExit(
                f"FATAL: {path} SHA-1 {sha1} != retail {RETAIL_SHA1}")
        if data[:8] != b"PS-X EXE":
            raise SystemExit("FATAL: not a PS-X EXE")
        self.taddr = struct.unpack_from("<I", data, 0x18)[0]
        self.tsize = struct.unpack_from("<I", data, 0x1C)[0]
        self.mem = bytearray(self.tsize)
        self.mem[:] = data[0x800:0x800 + self.tsize]
        if len(data) != 0x800 + self.tsize:
            raise SystemExit(
                f"FATAL: exe size {len(data)} != 0x800 + tsize {self.tsize}")

    def load_u32(self, addr):
        off = addr - self.taddr
        if off < 0 or off + 4 > self.tsize:
            raise SystemExit(f"FATAL: read outside exe image: 0x{addr:08X}")
        return struct.unpack_from("<I", self.mem, off)[0]

    def store_u32(self, addr, val):
        off = addr - self.taddr
        if off < 0 or off + 4 > self.tsize:
            raise SystemExit(f"FATAL: write outside exe image: 0x{addr:08X}")
        struct.pack_into("<I", self.mem, off, val & MASK)


class RngOracle:
    """Exact MIPS-semantics model of func_80070D10 / 70D6C / 70DD0."""

    def __init__(self, image):
        self.img = image
        self.read_log = []   # (call, addr) for every lw
        self.write_log = []  # (call, addr) for every sw
        self.call_no = 0

    def func_80070D10(self):
        """Verbatim: 17-word descending-Fibonacci table + two index words."""
        t0 = GA_TABLE
        self.img.store_u32(t0 + 0x40, 1)
        self.img.store_u32(t0 + 0x3C, 2)
        t5 = 0xE
        while True:
            t3 = self.img.load_u32(t0 + 0x40)
            t4 = self.img.load_u32(t0 + 0x3C)
            t0 = u32(t0 - 4)
            self.img.store_u32(t0 + 0x3C, u32(t3 + t4))
            if t5 == 0:
                break            # bnez decides on pre-decrement value
            t5 -= 1
        self.img.store_u32(GA_INDEX1, 0x40)
        self.img.store_u32(GA_INDEX2, 0x10)

    def func_80070D6C(self):
        """Verbatim: lagged-Fibonacci advance.  Returns (v0, i1, i2, a1, a2)."""
        self.call_no += 1
        t1 = s32(self.img.load_u32(GA_INDEX1))
        t2 = s32(self.img.load_u32(GA_INDEX2))
        t3 = u32(GA_TABLE + t1)          # addu $t3, $t0, $t1
        t4 = u32(GA_TABLE + t2)          # addu $t4, $t0, $t2
        t5 = self.img.load_u32(t3)
        t6 = self.img.load_u32(t4)
        self.read_log.append((self.call_no, t3))
        self.read_log.append((self.call_no, t4))
        t5 = u32(t5 + t6)                # addu
        self.img.store_u32(t3, t5)
        self.write_log.append((self.call_no, t3))
        v0 = t5
        t1 = s32(t1 - 4)                 # addiu -4
        t2 = s32(t2 - 4)                 # delay slot of first bgez
        if t1 < 0:
            t1 = 0x40                    # ori $t1, $zero, 0x40
        if t2 < 0:
            t2 = s32(u32(t2) | 0x40)     # ori $t2, $t2, 0x40  (NOT +0x40)
        self.img.store_u32(GA_INDEX1, u32(t1))
        self.img.store_u32(GA_INDEX2, u32(t2))
        self.write_log.append((self.call_no, GA_INDEX1))
        self.write_log.append((self.call_no, GA_INDEX2))
        return v0, t1, t2, t3, t4

    def func_80070DD0(self, a0, a1):
        """Handwritten ranged random: a0 + ((rand16 * (a1-a0)) >> 16)."""
        v0, *_ = self.func_80070D6C()
        diff = s32(u32(a1) - u32(a0))    # sub in delay slot
        r = v0 & 0xFFFF                  # andi
        product = r * diff               # mult: signed 32x32 -> 64
        product &= 0xFFFFFFFFFFFFFFFF
        if product >= 0x8000000000000000:
            product -= 0x10000000000000000
        scaled = product >> 16           # (lo>>16)|(hi<<16), arithmetic
        return u32(u32(a0) + u32(scaled))


def main():
    dump_calls = 0
    args = sys.argv[1:]
    if args and args[0] == "--dump-calls":
        dump_calls = int(args[1])
        args = args[2:]
    if len(args) != 1:
        raise SystemExit(__doc__)
    img = ExeImage(args[0])
    print(f"exe: taddr=0x{img.taddr:08X} tsize=0x{img.tsize:X} "
          f"sha1={RETAIL_SHA1} OK")

    # Cross-check the bytes the RNG reads below its table against the known
    # encodings from asm/disc1/5F3E4.s (func_80070D6C tail + func_80070DD0).
    known = {
        0x80070DC8: 0x03E00008,  # jr $ra
        0x80070DD0: 0x001F1825,  # or $v1, $zero, $ra
        0x80070E00: 0x00441020,  # add $v0, $v0, $a0 (handwritten)
    }
    for addr, want in known.items():
        got = img.load_u32(addr)
        status = "OK" if got == want else "MISMATCH"
        print(f"exe word 0x{addr:08X} = 0x{got:08X} (expect 0x{want:08X}) {status}")
        if got != want:
            raise SystemExit("FATAL: exe bytes do not match disassembly")

    rng = RngOracle(img)
    rng.func_80070D10()

    # Sanity: the seeded table must be the descending Fibonacci sequence.
    expect = [2584, 1597, 987, 610, 377, 233, 144, 89, 55,
              34, 21, 13, 8, 5, 3, 2, 1]
    got = [img.load_u32(GA_TABLE + 4 * i) for i in range(17)]
    if got != expect or img.load_u32(GA_INDEX1) != 0x40 \
            or img.load_u32(GA_INDEX2) != 0x10:
        raise SystemExit("FATAL: func_80070D10 model produced wrong seed")
    print("func_80070D10 seed: OK (17-word descending Fibonacci, idx 0x40/0x10)")

    checkpoints = {1, 2, 16, 17, 64, 256, 2000}
    i2_min = i2_max = None
    addr2_min, addr2_max = MASK, 0
    v0 = i1 = i2 = a1 = a2 = 0
    rows = []
    for call in range(1, 2001):
        v0, i1, i2, a1, a2 = rng.func_80070D6C()
        i2_min = i2 if i2_min is None else min(i2_min, i2)
        i2_max = i2 if i2_max is None else max(i2_max, i2)
        addr2_min, addr2_max = min(addr2_min, a2), max(addr2_max, a2)
        if call <= dump_calls:
            rows.append((call, i1, i2, a1, a2,
                         img.load_u32(a1), v0))
        if call in checkpoints:
            print(f"checkpoint call={call:5d} v0=0x{v0:08X} i1={i1:4d} "
                  f"i2={i2:4d} addr1=0x{a1:08X} addr2=0x{a2:08X}")

    print(f"i2 range over 2000 calls: [{i2_min}, {i2_max}]")
    print(f"addr2 range over 2000 calls: "
          f"[0x{addr2_min:08X}, 0x{addr2_max:08X}]")
    r_min = min(a for _, a in rng.read_log)
    r_max = max(a for _, a in rng.read_log)
    w_min = min(a for _, a in rng.write_log)
    w_max = max(a for _, a in rng.write_log)
    print(f"read addresses:  0x{r_min:08X}..0x{r_max:08X}")
    print(f"write addresses: 0x{w_min:08X}..0x{w_max:08X}")

    for call, ci1, ci2, ca1, ca2, word1, cv0 in rows:
        print(f"  call={call:3d} i1={ci1:4d} i2={ci2:4d} "
              f"a1=0x{ca1:08X} a2=0x{ca2:08X} "
              f"retail[a1]=0x{word1:08X} v0=0x{cv0:08X}")

    # func_80070DD0 samples (continues the RNG stream after the warm-up).
    for a0, a1_ in ((0, 100), (1, 4), (0, 0x10000), (5, 5), (10, 0), (-3, 3)):
        out = rng.func_80070DD0(a0, a1_)
        print(f"70DD0({a0},{a1_}) = {out} (0x{out:08X})")


if __name__ == "__main__":
    main()
