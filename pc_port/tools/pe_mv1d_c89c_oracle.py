#!/usr/bin/env python3
"""Phase 6E-MV1d - func_8010C89C VLC decoder oracle + independent model.

Part 1 (retail-source proof): authenticates the 38-sector movie-module
carve from the Disc 1 raw image (SHA-256), then checks the decoder's
entry/exit/call-site instruction anchors in the module and ov133
carves. Part 2 (behavioral model): an independent transliteration of
the [0x8010C89C,0x8010CBF8) disassembly (215 words) used to generate
and verify the fixed test vectors pinned by the native suite
(MV1D_PAD, MV1D_BOUND, MV1D_RESUME, MV1D_TABLE, MV1D_MARKER).
"""
from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

M = 0xFFFFFFFF
BIN = (pathlib.Path(__file__).resolve().parents[2] / "rom" / "image" /
       "Parasite Eve (USA) (Disc 1)" / "Parasite Eve (USA) (Disc 1).bin")
MOV_SHA256 = ("d0a22a1adccb38ee2e8f8ad1897955dc36f2b2968ed98dc97680eec5"
              "ba8d0b40")
EB8C, EB90, EBB4 = 0x8011EB8C, 0x8011EB90, 0x8011EBB4


def require(cond: bool, msg: str) -> None:
    if not cond:
        raise SystemExit(f"FAIL: {msg}")


class Ram:
    def __init__(self) -> None:
        self.b: dict[int, int] = {}

    def lu8(self, a: int) -> int:
        return self.b.get(a & M, 0)

    def lu16(self, a: int) -> int:
        return self.lu8(a) | (self.lu8(a + 1) << 8)

    def lu32(self, a: int) -> int:
        return self.lu16(a) | (self.lu16(a + 2) << 16)

    def su8(self, a: int, v: int) -> None:
        self.b[a & M] = v & 0xFF

    def su16(self, a: int, v: int) -> None:
        self.su8(a, v)
        self.su8(a + 1, v >> 8)

    def su32(self, a: int, v: int) -> None:
        self.su16(a, v)
        self.su16(a + 2, v >> 16)


def s32(v: int) -> int:
    v &= M
    return v - 0x100000000 if v & 0x80000000 else v


class Model:
    """Independent read of the C89C disassembly, driven by an explicit
    trampoline (no Python recursion: real frames restart often)."""

    def __init__(self, cap: int = 100_000_000) -> None:
        self.cap = cap
        self.steps = 0
        self.cov: set[str] = set()
        self.ram: Ram | None = None
        self.a2 = 0
        self.a3 = 0
        self.t6 = 0

    def tick(self) -> None:
        self.steps += 1
        if self.steps > self.cap:
            raise RuntimeError("model step cap hit (non-termination?)")

    @staticmethod
    def refill(st: dict, ram: Ram, dst: str) -> None:
        v1 = st["v1"]
        low = v1 & 0xF
        if not (v1 & 0x10):
            st["v1"] = low
            return
        st[dst] = ram.lu16(st["a0"])
        st["a0"] = (st["a0"] + 2) & M
        st["v0"] = (st["v0"] | (st[dst] << low)) & M
        st["v1"] = low

    def run(self, ram: Ram, a0: int, a1: int, a2: int) -> int:
        self.ram = ram
        st = {"a0": a0 & M, "a1": a1 & M, "a2": a2 & M,
              "v0": 0, "v1": 0, "t4": 0, "t5": 0,
              "t7": 0, "t8": 0, "t9": 0}
        t1 = ram.lu32(EB8C)
        self.a2 = (a2 + 0x800) & M
        self.a3 = (self.a2 + 0x10000) & M
        if a0 == 0:
            st["a0"] = ram.lu32(EB90)
            st["a1"] = ram.lu32(EB90 + 4)
            st["v0"] = ram.lu32(EB90 + 8)
            st["v1"] = s32(ram.lu32(EB90 + 12))
            st["t4"] = ram.lu32(EB90 + 16)
            st["t5"] = s32(ram.lu32(EB90 + 20))
            st["t7"] = ram.lu32(EB90 + 24)
            st["t8"] = ram.lu32(EB90 + 28)
            st["t9"] = ram.lu32(EB90 + 32)
            t1 = (t1 + t1) & M
            self.t6 = (st["a1"] + t1) & M
            mode: str | int = "loop"
        else:
            st["t5"] = st["t7"] = st["t8"] = st["t9"] = 0
            t1 = (t1 + t1) & M
            self.t6 = (st["a1"] + t1) & M
            # header
            t1 = ram.lu32(a0)
            t4 = ram.lu16(a0 + 4)
            t2 = ram.lu16(a0 + 6)
            v0 = ram.lu16(a0 + 8)
            v1 = ram.lu16(a0 + 10)
            t2 = s32((t2 - 3) & M)
            t4 = (t4 << 10) & M
            if t2 >= 0:
                st["t5"] = 1
            st["a0"] = (a0 + 12) & M
            st["v0"] = ((v0 << 16) | v1) & M
            st["v1"] = 0
            ram.su32(st["a1"], t1)
            t1 = (((t1 & 0xFFFF) << 2) + 4 + st["a1"]) & M
            ram.su32(EBB4, t1)
            st["a1"] = (st["a1"] + 2) & M
            st["t4"] = t4
            mode = "dispatch"
        while True:
            self.tick()
            if mode == "dispatch":
                mode = self.dispatch(st)
            elif mode == "limb0":
                mode = self.limb0(st)
            elif mode == "bound":
                mode = self.bound(st)
            elif mode == "loop":
                mode = self.loop(st)
            else:
                assert isinstance(mode, int)
                return mode

    def dispatch(self, st: dict):
        ram = self.ram
        assert ram is not None
        t0 = st["v0"] >> 22
        if st["t5"] == 0:
            return self.limb0body(st, t0)
        at = s32(t0 ^ 0x3FF)
        st["a1"] = (st["a1"] + 2) & M
        if at == 0:
            self.cov.add("pad_via_3ff")
            return self.pad(st)
        tab = (self.a2 - 0x400) & M
        if st["t5"] >= 3:
            tab = (tab - 0x400) & M
        t0 = ((st["v0"] >> 24) << 2) & M
        t0 = (t0 + tab) & M
        t1 = ram.lu16(t0)
        t2 = ram.lu16(t0 + 2)
        t0 = 0
        st["v0"] = (st["v0"] << (t1 & 31)) & M
        if t2 != 0:
            self.cov.add("table_extract")
            at = 32 - t2
            t0 = st["v0"] >> (at & 31)
            neg = s32(st["v0"]) < 0
            st["v0"] = (st["v0"] << (t2 & 31)) & M
            if not neg:
                t3 = M >> (at & 31)
                t0 = (t0 - t3) & M
            st["v1"] = st["v1"] + t2
        st["v1"] = st["v1"] + t1
        st["t1"] = t1
        self.refill(st, ram, "t1")
        t1 = st["t1"]
        at = st["t5"] - 2
        t1 = (st["t9"] + t0) & M
        if at > 0:
            st["t9"] = (st["t9"] + t0) & M
        else:
            t1 = (st["t8"] + t0) & M
            if at == 0:
                st["t8"] = (st["t8"] + t0) & M
            else:
                t1 = (st["t7"] + t0) & M
                st["t7"] = (st["t7"] + t0) & M
        t1 = (t1 << 2) & M
        t1 &= 0x3FF
        t1 |= st["t4"]
        st["t5"] += 1
        at = st["t5"] - 7
        ram.su16(st["a1"], t1)
        if at == 0:
            st["t5"] -= 6
        return "bound"

    def limb0body(self, st: dict, t0: int):
        ram = self.ram
        assert ram is not None
        at = s32(t0 ^ 0x1FF)
        st["a1"] = (st["a1"] + 2) & M
        if at == 0:
            self.cov.add("pad_via_1ff")
            return self.pad(st)
        st["v0"] = (st["v0"] << 10) & M
        st["v1"] = st["v1"] + 10
        self.refill(st, ram, "t1")
        t0 = (t0 | st["t4"]) & M
        ram.su16(st["a1"], t0)
        return "bound"

    def limb0(self, st: dict):
        return self.limb0body(st, st["v0"] >> 22)

    def bound(self, st: dict):
        ram = self.ram
        assert ram is not None
        at = s32(st["a1"] - self.t6)
        st["a1"] = (st["a1"] + 2) & M
        if at >= 0:
            self.cov.add("bound_exit")
            ram.su32(EB90, st["a0"])
            ram.su32(EB90 + 4, st["a1"])
            ram.su32(EB90 + 8, st["v0"])
            ram.su32(EB90 + 12, st["v1"] & M)
            ram.su32(EB90 + 16, st["t4"])
            ram.su32(EB90 + 20, st["t5"] & M)
            ram.su32(EB90 + 24, st["t7"])
            ram.su32(EB90 + 28, st["t8"])
            ram.su32(EB90 + 32, st["t9"])
            return 1
        return "loop"

    def loop(self, st: dict):
        ram = self.ram
        assert ram is not None
        t0 = ((st["v0"] >> 19) << 3) & M
        t0 = (t0 + self.a2) & M
        t1 = ram.lu32(t0)
        at = s32(t1 & 0xFF)
        if t1 == 0:
            st["v0"] = (st["v0"] << 8) & M
            st["v1"] = st["v1"] + 8
            self.refill(st, ram, "t0")
            t0 = ((st["v0"] >> 23) << 2) & M
            t0 = (t0 + self.a3) & M
            t1 = ram.lu32(t0)
            t3 = 0
        else:
            self.cov.add("main_nonzero")
            t3 = ram.lu32(t0 + 4)
        st["v0"] = (st["v0"] << (at & 31)) & M
        st["v1"] = st["v1"] + at
        # refill3's t0 is dead after (CB00 uses t1; CB60 recomputes).
        self.refill(st, ram, "t0")
        t1 >>= 16
        if t1 == 0x7C1F:
            self.cov.add("marker_7c1f")
            self.cb60(st)
            return "loop"
        at = s32(t1 ^ 0xFE00)
        ram.su16(st["a1"], t1)
        if at == 0:
            self.cov.add("marker_fe00")
            return "dispatch"
        st["a1"] = (st["a1"] + 2) & M
        if t3 == 0:
            return "loop"
        self.cov.add("t3_chain")
        t2 = s32(t3 & 0xFFFF)
        if t2 == 0x7C1F:
            self.cb60(st)
            return "loop"
        at = s32(t2 ^ 0xFE00)
        ram.su16(st["a1"], t2)
        if at == 0:
            return "dispatch"
        t2 = s32((t3 >> 16) & M)
        st["a1"] = (st["a1"] + 2) & M
        if t2 == 0:
            return "loop"
        if t2 == 0x7C1F:
            self.cb60(st)
            return "loop"
        at = s32(t2 ^ 0xFE00)
        ram.su16(st["a1"], t2)
        if at == 0:
            return "dispatch"
        st["a1"] = (st["a1"] + 2) & M
        return "loop"

    def cb60(self, st: dict) -> None:
        ram = self.ram
        assert ram is not None
        self.cov.add("cb60")
        t0 = st["v0"] >> 16
        ram.su16(st["a1"], t0)
        st["a1"] = (st["a1"] + 2) & M
        t0 = ram.lu16(st["a0"])
        st["a0"] = (st["a0"] + 2) & M
        st["v0"] = (st["v0"] << 16) & M
        st["v0"] = (st["v0"] | (t0 << (st["v1"] & 31))) & M

    def pad(self, st: dict) -> int:
        ram = self.ram
        assert ram is not None
        bound = ram.lu32(EBB4)
        while s32(st["a1"] - bound) < 0:
            self.tick()
            ram.su16(st["a1"], 0xFE00)
            st["a1"] = (st["a1"] + 2) & M
        return 0


def carve(raw: bytes, lba: int, n: int) -> bytes:
    out = bytearray()
    for i in range(n):
        b = (lba + i) * 2352 + 24
        out += raw[b:b + 2048]
    return bytes(out)


def u32(blob: bytes, off: int) -> int:
    return struct.unpack_from("<I", blob, off)[0]


def check_anchors() -> tuple[bytes, bytes]:
    require(BIN.is_file(), f"Disc 1 image absent: {BIN}")
    raw = BIN.read_bytes()
    sig = raw.find(b"\x07\x00\x00\x00MDEC")
    require(sig % 2352 == 24, "movie signature misaligned")
    msec0 = (sig - 24) // 2352
    mov = carve(raw, msec0, 38)
    require(hashlib.sha256(mov).hexdigest() == MOV_SHA256,
            "movie module authentication")
    print("OK module: 38 sectors, SHA-256 exact")
    base = 0x8010BCF8
    # entry: lui t0,0x8012; addiu -0x1474; addi a2,+0x800; lui at,1
    require(u32(mov, 0x8010C89C - base) == 0x3C088012, "C89C lui")
    require(u32(mov, 0x8010C8A0 - base) == 0x2508EB8C, "C89C addiu EB8C")
    require(u32(mov, 0x8010C8A4 - base) == 0x20C60800, "C89C a2+=0x800")
    require(u32(mov, 0x8010C8A8 - base) == 0x3C010001, "C89C lui at,1")
    require(u32(mov, 0x8010C8AC - base) == 0x00C13820, "C89C a3=a2+at")
    # exits: pad-return-0 and bound-return-1
    require(u32(mov, 0x8010CBC0 - base) == 0x03E00008, "pad jr ra")
    require(u32(mov, 0x8010CBC4 - base) == 0x00001020, "pad ret 0")
    require(u32(mov, 0x8010CBF4 - base) == 0x03E00008, "bound jr ra")
    require(u32(mov, 0x8010CBF8 - base) == 0x20020001, "bound ret 1")
    # file-seeded state: [EB8C] = 0x00FFFFFF
    require(u32(mov, EB8C - base) == 0x00FFFFFF, "EB8C file seed")
    print("OK decoder: entry/a3-kill/pad-exit/bound-exit/EB8C seed")
    # ov133 tail call site: s3 setup, a2 load, jal C89C, EC stores
    pvd = raw[16 * 2352 + 24: 16 * 2352 + 24 + 2048]
    root = struct.unpack_from("<I", pvd, 158)[0]
    dpos, dend = 0, struct.unpack_from("<I", pvd, 166)[0]
    rdir = b""
    while len(rdir) < dend:
        rdir += raw[(root + len(rdir) // 2048) * 2352 + 24:
                    (root + len(rdir) // 2048) * 2352 + 24 + 2048]
    peimg = None
    while dpos < dend:
        rl = rdir[dpos]
        if rl == 0:
            dpos = ((dpos // 2048) + 1) * 2048
            continue
        if rdir[dpos + 33:dpos + 33 + rdir[dpos + 32]] == b"PE.IMG;1":
            peimg = struct.unpack_from("<I", rdir, dpos + 2)[0]
        dpos += rl
    require(peimg is not None, "PE.IMG not found")
    ov = carve(raw, peimg + 0x3D2, 0x85)
    rel = lambda pc: pc - 0x8018EFF0
    require(u32(ov, rel(0x801927C0)) == 0x3C13801D, "tail lui s3")
    require(u32(ov, rel(0x801927C4)) == 0x26731464, "tail s3=1464")
    require(u32(ov, rel(0x80192814)) == 0x3C06801D, "tail lui a2")
    require(u32(ov, rel(0x80192818)) == 0x8CC60DF8, "tail lw a2,[DF8]")
    require(u32(ov, rel(0x8019284C)) == 0x8C450000, "tail lw a1,(v0)")
    require(u32(ov, rel(0x80192850)) == 0x0C043227, "tail jal C89C")
    require(u32(ov, rel(0x80192854)) == 0x02202021, "tail delay move a0,s1")
    require(u32(ov, rel(0x80192858)) == 0x0C01F0E5, "tail jal 7C394")
    require(u32(ov, rel(0x801928F8)) == 0xA0200DBD, "EC sb zero,[DBD]")
    require(u32(ov, rel(0x80192908)) == 0xA4250DBC, "EC sh 1,[DBC]")
    print("OK callsite: s3/a2/a1 loads, jal C89C, jal 7C394, EC stores")
    return mov, ov


S = 0x1000      # synthetic stream base
A = 0x2000      # synthetic arena base
T = 0x3000      # synthetic table-word base


def plant_pad(ram: Ram) -> None:
    """Fresh frame whose header forces the t5==0 limb with a fast
    0x1FF pad exit: [S+6] = 0, v0>>22 = 0x1FF, W = 1 (two FE00
    fills to the A+8 bound)."""
    ram.su32(EB8C, 0x00FFFFFF)
    ram.su32(S, 0x00000001)
    ram.su16(S + 4, 0)
    ram.su16(S + 6, 0)
    ram.su16(S + 8, 0x7FC0)
    ram.su16(S + 10, 0x0000)


def plant_bound(ram: Ram) -> None:
    """Fresh frame with v0>>22 = 0: t5==0 limb writes once, then the
    [EB8C] = 0 bound fires immediately (return 1)."""
    ram.su32(EB8C, 0x00000000)
    ram.su32(S, 0x00000000)
    ram.su16(S + 4, 0)
    ram.su16(S + 6, 0)
    ram.su16(S + 8, 0x0000)
    ram.su16(S + 10, 0x0000)
    ram.su16(S + 12, 0x1234)   # present but unread here (v1&0x10 == 0)


def plant_pad3ff(ram: Ram) -> None:
    """Fresh frame, t5 = 1, v0>>22 = 0x3FF: the first dispatch takes
    the pad exit directly (return 0, no fills)."""
    ram.su32(EB8C, 0x00FFFFFF)
    ram.su32(S, 0x00000000)
    ram.su16(S + 4, 0)
    ram.su16(S + 6, 3)
    ram.su16(S + 8, 0xFFC0)
    ram.su16(S + 10, 0x0000)


def plant_table(ram: Ram) -> None:
    """Fresh frame driving the C984 table-extract (t2 = 8) with a
    taken C9CC refill (v1 = 8 + 16 hits bit 4), one emit, then the
    [EB8C] = 0 bound (ret 1). Header LO is zeroed by the C938
    move, so v1 = t2 + t1 = 24 at the refill."""
    ram.su32(EB8C, 0x00000000)
    ram.su32(S, 0x00000000)
    ram.su16(S + 4, 0)
    ram.su16(S + 6, 3)
    ram.su16(S + 8, 0x0100)
    ram.su16(S + 10, 0)
    ram.su16(S + 12, 0x00BB)
    ram.su16(T + 0x404, 16)     # tab base (t5 < 3: a2+0x800-0x400)
    ram.su16(T + 0x406, 8)      # tab count


def plant_main(ram: Ram) -> None:
    """Fresh frame ([S+6] = 0, so the t5 == 0 limb) running one
    main-loop symbol (FE00 restart) and a taken CA4C refill before
    the [EB8C] = 3 bound fires (ret 1)."""
    ram.su32(EB8C, 0x00000003)
    for i in range(12):
        ram.su8(S + i, 0)
    ram.su32(T + 0x800, 0xFE000003) # visited main slot: FE00, at = 3
    ram.su32(T + 0x804, 0)          # its t3


def plant_resume_tables(ram: Ram) -> None:
    ram.su32(T + 0x800, 0xFE000000) # visited main slot: FE00, at = 0
    ram.su32(T + 0x804, 0)


def run_vectors() -> None:
    # MV1D_PAD
    ram = Ram()
    plant_pad(ram)
    m = Model()
    ret = m.run(ram, S, A, T)
    require(ret == 0, "PAD ret")
    require(ram.lu32(EBB4) == A + 8, "PAD bound")
    require(ram.lu32(A) == 1, "PAD stream word store")
    require(ram.lu16(A + 2) == 0, "PAD gap untouched")
    require(ram.lu16(A + 4) == 0xFE00, "PAD fill 1")
    require(ram.lu16(A + 6) == 0xFE00, "PAD fill 2")
    print(f"OK MV1D_PAD: ret 0, EBB4={ram.lu32(EBB4):#x}, "
          f"steps={m.steps}, cov={sorted(m.cov)}")
    # MV1D_BOUND
    ram = Ram()
    plant_bound(ram)
    m = Model()
    ret = m.run(ram, S, A, T)
    require(ret == 1, "BOUND ret")
    require(ram.lu32(EB90) == S + 12, "BOUND saved a0")
    require(ram.lu32(EB90 + 4) == A + 6, "BOUND saved a1")
    require(ram.lu32(EB90 + 8) == 0, "BOUND saved v0")
    require(ram.lu32(EB90 + 12) == 10, "BOUND saved v1")
    require(ram.lu32(EBB4) == A + 4, "BOUND EBB4")
    print(f"OK MV1D_BOUND: ret 1, saved a0={ram.lu32(EB90):#x} "
          f"a1={ram.lu32(EB90 + 4):#x}, steps={m.steps}")
    # MV1D_PAD3FF
    ram = Ram()
    plant_pad3ff(ram)
    m = Model()
    ret = m.run(ram, S, A, T)
    require(ret == 0, "PAD3FF ret")
    require(ram.lu32(EBB4) == A + 4, "PAD3FF bound")
    require(ram.lu32(A) == 0, "PAD3FF stream word store")
    require(ram.lu16(A + 2) == 0, "PAD3FF gap untouched")
    print(f"OK MV1D_PAD3FF: ret 0, steps={m.steps}, "
          f"cov={sorted(m.cov)}")
    # MV1D_TABLE
    ram = Ram()
    plant_table(ram)
    m = Model()
    ret = m.run(ram, S, A, T)
    require(ret == 1, "TABLE ret")
    require(ram.lu32(EB90) == S + 14, "TABLE saved a0")
    require(ram.lu32(EB90 + 4) == A + 6, "TABLE saved a1")
    require(ram.lu32(EB90 + 8) == 0xBB00, "TABLE saved v0")
    require(ram.lu32(EB90 + 12) == 8, "TABLE saved v1")
    require(ram.lu32(EB90 + 20) == 2, "TABLE saved t5")
    require(ram.lu32(EB90 + 24) == 0xFFFFFF01, "TABLE saved t7")
    require(ram.lu32(EBB4) == A + 4, "TABLE EBB4")
    require(ram.lu16(A + 4) == 4, "TABLE emit")
    print(f"OK MV1D_TABLE: ret 1, steps={m.steps}, "
          f"cov={sorted(m.cov)}")
    # MV1D_MAIN
    ram = Ram()
    plant_main(ram)
    m = Model()
    ret = m.run(ram, S, A, T)
    require(ret == 1, "MAIN ret")
    require(ram.lu32(EB90) == S + 14, "MAIN saved a0")
    require(ram.lu32(EB90 + 4) == A + 10, "MAIN saved a1")
    require(ram.lu32(EB90 + 8) == 0, "MAIN saved v0")
    require(ram.lu32(EB90 + 12) == 7, "MAIN saved v1")
    require(ram.lu32(EB90 + 20) == 0, "MAIN saved t5")
    require(ram.lu32(EBB4) == A + 4, "MAIN EBB4")
    require(ram.lu32(A) == 0, "MAIN stream word store")
    require(ram.lu16(A + 4) == 0, "MAIN emit 1")
    require(ram.lu16(A + 6) == 0xFE00, "MAIN restart store")
    require(ram.lu16(A + 8) == 0, "MAIN emit 2")
    print(f"OK MV1D_MAIN: ret 1, steps={m.steps}, "
          f"cov={sorted(m.cov)}")
    # MV1D_RESUME: part 1 is the BOUND vector ([EB8C] = 0); part 2
    # resumes (a0 = 0) into steered tables, exercising the 9-word
    # reload, a main-loop symbol, an FE00 restart, a taken CA4C
    # refill, and a second bound exit.
    ram = Ram()
    plant_bound(ram)
    plant_resume_tables(ram)
    m = Model()
    require(m.run(ram, S, A, T) == 1, "RESUME part1 ret")
    m = Model()
    ret = m.run(ram, 0, 0, T)
    require(ret == 1, "RESUME part2 ret")
    require(ram.lu32(EB90) == S + 14, "RESUME saved a0")
    require(ram.lu32(EB90 + 4) == A + 10, "RESUME saved a1")
    require(ram.lu32(EB90 + 8) == 0x12340, "RESUME saved v0")
    require(ram.lu32(EB90 + 12) == 4, "RESUME saved v1")
    require(ram.lu32(EB90 + 20) == 0, "RESUME saved t5")
    print(f"OK MV1D_RESUME: ret 1, steps={m.steps}, "
          f"cov={sorted(m.cov)}")


def main() -> int:
    check_anchors()
    run_vectors()
    print("PASS: mv1d anchors + model vectors")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

