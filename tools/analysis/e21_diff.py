#!/usr/bin/env python3
"""E21 differential: lowered room TUs vs the retail MIPS code they came from.

For every generated E21 room TU (tools/analysis/guest_lower.py) whose only
calls are loud boundaries or retail `jalr`s (both are mocked identically on
the two sides), this

  1. loads the extracted retail room blob at its VMA into a 2 MiB guest RAM
     image pre-filled from a seed (every word a 4-aligned guest address, so
     pointer chains stay inside RAM; integer fields just see big values),
  2. runs the retail leaf in a small MIPS-I interpreter (delay slots, the
     era's lwl/lwr/swl/swr, mult/div), mocking every jal/jalr target with a
     recorded call returning a seed-derived value,
  3. runs the lowered native C on the same image with the same mocks,
  4. compares guest RAM (outside both stacks), the return value and the
     ordered call log.

Inline GTE asm (E21b) is covered too: the interpreter carries an independent
Python COP2 model (psx-spx register semantics, MVMVA mx=RT, NCLIP) and the
native side links the real pc_port/platform/pe_gte.c.  The 1 KiB scratchpad
(0x1F800000) is mapped and compared on both sides.

Cases where the retail code itself faults (address error, `break`, an
unsupported cop2 op) are skipped, not counted.  Retail blobs come from build/extracted (never
committed).  Usage: e21_diff.py [--seeds N] [--limit K] [--jobs J] [--verbose]
"""
from __future__ import annotations

import argparse
import concurrent.futures
import os
import random
import re
import shutil
import struct
import subprocess
import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import gen_overlay_ports as o  # noqa: E402
import guest_lower as gl  # noqa: E402

RAM_BASE, RAM_SIZE = 0x80000000, 0x200000
SPAD = 0x400                             # scratchpad, stored after RAM
SENTINEL = 0x8000DEAC
SP0 = 0x801FFF00
EXCLUDE = (0x801FE000, 0x80200000)      # native frame arena + retail stack
MAX_STEPS = 200000
COVER: set | None = None                # retail pcs executed (--coverage)


class Fault(Exception):
    pass


def s32(v):
    v &= 0xFFFFFFFF
    return v - 0x100000000 if v & 0x80000000 else v


def mock_ret(target: int, seed: int) -> int:
    h = (target * 2654435761 + seed * 40503) & 0xFFFFFFFF
    if ((seed * 2654435761) & 0xFFFFFFFF) >> 16 & 1:   # half the seeds (hashed, so it does not
                                        # couple with the directed combinations): a small signed value (both signs of
        return ((h & 0x1FFF) - 0x1000) & 0xFFFFFFFF   # every division bias get exercised)
    return 0x80100000 + (h & 0xDFFFC)   # stays below both stack windows


def sx16(v):
    v &= 0xFFFF
    return (v - 0x10000 if v & 0x8000 else v) & 0xFFFFFFFF


def s16(v):
    v &= 0xFFFF
    return v - 0x10000 if v & 0x8000 else v


_NATIVE_GTE = None


def native_gte():
    """ctypes handle on the port's own GTE engine (pc_port/platform/pe_gte.c +
    pe_gte_commands.c), built once per process into TMPDIR.  The ctest
    gte-native-hardware checks that engine against the real-console ps1-tests
    log; the oracle runs the commands it does not model itself (RTPS, RTPT,
    NCDS, AVSZ, ...) on it, so e21 exercises a leaf's lowering, not GTE
    arithmetic, for those."""
    global _NATIVE_GTE
    if _NATIVE_GTE is None:
        import ctypes
        import hashlib
        pc = o.REPO_ROOT / "pc_port"
        srcs = [pc / "platform" / "pe_gte.c", pc / "platform" / "pe_gte_commands.c"]
        key = hashlib.sha1(b"".join(s.read_bytes() for s in srcs)).hexdigest()[:16]
        d = Path(tempfile.gettempdir()) / f"e21_native_gte_{key}"
        so = d / "libpegte.so"
        if not so.is_file():
            d.mkdir(parents=True, exist_ok=True)
            stub = d / "stub.c"
            stub.write_text("#include <stdint.h>\n"
                            "uint16_t PE_LoadU16(uint32_t a) { (void)a; return 0; }\n"
                            "uint32_t PE_LoadU32(uint32_t a) { (void)a; return 0; }\n"
                            "void PE_Port_RequestStop(const char *r) { (void)r; }\n")
            tmp = d / f"libpegte.{os.getpid()}.so"
            cc = shutil.which("cc") or shutil.which("gcc")
            r = subprocess.run([cc, "-O1", "-std=gnu11", "-w", "-shared", "-fPIC",
                                "-I", str(pc / "include"), "-I", str(pc / "platform"), "-I", str(pc),
                                "-I", str(pc / "bootstrap"), "-o", str(tmp), *map(str, srcs), str(stub)],
                               capture_output=True, text=True)
            if r.returncode:
                raise SystemExit("e21_diff: native GTE library: " + r.stderr[:300])
            os.replace(tmp, so)
        lib = ctypes.CDLL(str(so))
        for fn in ("PE_GTE_ReadData", "PE_GTE_ReadCtrl"):
            getattr(lib, fn).restype = ctypes.c_uint32
            getattr(lib, fn).argtypes = [ctypes.c_uint32]
        for fn in ("PE_GTE_WriteData", "PE_GTE_WriteCtrl"):
            getattr(lib, fn).restype = None
            getattr(lib, fn).argtypes = [ctypes.c_uint32, ctypes.c_uint32]
        lib.PE_GTE_Command.restype = None
        lib.PE_GTE_Command.argtypes = [ctypes.c_uint32]
        _NATIVE_GTE = lib
    return _NATIVE_GTE


class Gte:
    """Independent COP2 model (psx-spx 'GTE Registers' / MVMVA / NCLIP); every
    other command runs on the port's hardware-checked engine (native_gte)."""

    def __init__(self):
        self.d = [0] * 32
        self.c = [0] * 32

    def wd(self, r, v):
        v &= 0xFFFFFFFF
        if r in (1, 3, 5, 8, 9, 10, 11):
            self.d[r] = sx16(v)
        elif r in (7, 16, 17, 18, 19):
            self.d[r] = v & 0xFFFF
        elif r == 15:
            self.d[12], self.d[13], self.d[14] = self.d[13], self.d[14], v
        elif r == 28:
            for i in range(3):
                self.d[9 + i] = ((v >> (5 * i)) & 0x1F) * 0x80
        elif r == 30:
            self.d[30] = v
            sign = v >> 31
            n = 0
            while n < 32 and ((v >> (31 - n)) & 1) == sign:
                n += 1
            self.d[31] = n
        elif r in (29, 31):
            pass
        else:
            self.d[r] = v

    def rd(self, r):
        if r == 15:
            return self.d[14]
        if r in (28, 29):
            out = 0
            for i in range(3):
                c = s32(self.d[9 + i]) >> 7
                out |= min(max(c, 0), 0x1F) << (5 * i)
            return out
        return self.d[r]

    def wc(self, r, v):
        v &= 0xFFFFFFFF
        if r in (4, 12, 20, 26, 27, 29, 30):
            self.c[r] = sx16(v)
        elif r == 31:
            f = v & 0x7FFFF000
            self.c[31] = f | (0x80000000 if f & 0x7F87E000 else 0)
        else:
            self.c[r] = v

    def rc(self, r):
        return self.c[r]

    def command(self, cmd):
        """NCLIP and MVMVA (psx-spx): FLAG restarts per command; every MAC
        addition is checked against 44 bits (FLAG 30-i / 27-i) and wraps;
        IR saturation sets FLAG 24-i; NCLIP's MAC0 sets FLAG 16 / 15."""
        op = cmd & 0x3F
        flag = 0
        if op == 0x06:
            sx = [s16(self.d[12 + i]) for i in range(3)]
            sy = [s16(self.d[12 + i] >> 16) for i in range(3)]
            m = sx[0] * sy[1] + sx[1] * sy[2] + sx[2] * sy[0] - sx[0] * sy[2] - sx[1] * sy[0] - sx[2] * sy[1]
            if m > 0x7FFFFFFF:
                flag |= 1 << 16
            elif m < -0x80000000:
                flag |= 1 << 15
            self.d[24] = m & 0xFFFFFFFF
            self.set_flag(flag)
            return
        sf, mx, vs, cv, lm = (cmd >> 19) & 1, (cmd >> 17) & 3, (cmd >> 15) & 3, (cmd >> 13) & 3, (cmd >> 10) & 1
        if op != 0x12 or mx == 3 or cv == 2:
            self.native_command(cmd)
            return
        base = (0, 8, 16)[mx]
        m = []
        for i in range(4):
            m += [s16(self.c[base + i]), s16(self.c[base + i] >> 16)]
        m.append(s16(self.c[base + 4]))
        if vs == 3:
            v = [s16(self.d[9]), s16(self.d[10]), s16(self.d[11])]
        else:
            v = [s16(self.d[2 * vs]), s16(self.d[2 * vs] >> 16), s16(self.d[2 * vs + 1])]
        t = ([s32(self.c[5 + i]) for i in range(3)] if cv == 0 else
             [s32(self.c[13 + i]) for i in range(3)] if cv == 1 else [0, 0, 0])
        for i in range(3):
            mac = t[i] << 12
            for k in range(3):
                mac += m[3 * i + k] * v[k]
                if mac > 0x7FFFFFFFFFF:
                    flag |= 1 << (30 - i)
                elif mac < -0x80000000000:
                    flag |= 1 << (27 - i)
                mac &= 0xFFFFFFFFFFF
                if mac & 0x80000000000:
                    mac -= 0x100000000000
            mac32 = s32(mac >> (12 * sf))
            self.d[25 + i] = mac32 & 0xFFFFFFFF
            lo = 0 if lm else -0x8000
            ir = min(max(mac32, lo), 0x7FFF)
            if ir != mac32:
                flag |= 1 << (24 - i)
            self.d[9 + i] = ir & 0xFFFFFFFF
        self.set_flag(flag)

    def set_flag(self, flag):
        self.c[31] = flag | (0x80000000 if flag & 0x7F87E000 else 0)

    def native_command(self, cmd):
        """Copy the architectural state into the native engine, run the
        command there, copy back what a command can change: data 0-27 (but
        15, SXYP, a view of SXY2) and FLAG.  Data 28/29 (IRGB/ORGB) and 31
        (LZCR) are views of other registers and are not written; LZCS/LZCR
        and the other control registers are never command outputs, so the
        oracle keeps its own (writing LZCS here recomputes LZCR)."""
        g = native_gte()
        for r in range(32):
            g.PE_GTE_WriteCtrl(r, self.c[r] & 0xFFFFFFFF)
        for r in range(32):
            if r not in (15, 28, 29, 31):
                g.PE_GTE_WriteData(r, self.d[r] & 0xFFFFFFFF)
        g.PE_GTE_Command(cmd & 0x1FFFFFF)
        for r in range(28):
            if r != 15:
                self.d[r] = g.PE_GTE_ReadData(r)
        self.c[31] = g.PE_GTE_ReadCtrl(31)


OUT_LEN_SHIFT = 40                      # out=FN:K:LEN keeps LEN above the FN<<4|K key
# Output extents of known callees' pointer arguments: the bytes a mocked
# callee writes through an out= stack argument when the spec gives no LEN.
OUT_EXTENTS = {
    (0x80079754, 1): 18, (0x80078CC4, 0): 20,
    (0x800CE8F0, 3): 6, (0x800CE9D4, 2): 6, (0x800CEAE8, 2): 6,
    (0x800CFB7C, 2): 6, (0x800CF3AC, 1): 3,
    # ApplyMatrixSV writes a 3-short vector; the RotMatrix variant 800794C4
    # writes rotation + translation (32 bytes, as the m0075i directed specs);
    # 80071A44 is memset(dst, byte, len) with the 16 bytes room callers pass.
    (0x80078C34, 2): 6, (0x800794C4, 1): 32, (0x80071A44, 0): 16,
    # colour lookup (src/func_800C3134.c: a2[0..2] = lerped r,g,b) and the
    # range test's result word (src/func_800C5EB0.c: *a2 = 0 / hit)
    (0x800C3134, 2): 3, (0x800C5EB0, 2): 4,
    # libgte VectorNormal(v0, v1) writes v1's three words; RotTrans(v0, v1,
    # flag) writes v1 and *flag; Akao_Calc3DPan (src/func_8006DFA8.c) writes
    # *pan and *vol.  Callers read all of them back from their own frames.
    (0x80078134, 1): 12, (0x800792D4, 1): 12, (0x800792D4, 2): 4,
    (0x8006DFA8, 1): 4, (0x8006DFA8, 2): 4,
}
# Per overlay, for callees inside an arena other overlays reuse: ovl_0700's
# spline sampler (src/overlays/ovl_0700/func_8018F55C.c) writes out->vx/vy/vz
# and rot->vx/vy/vz, which its callers read back from their own frames.
OUT_EXTENTS_OVL = {
    "ovl_0700": {(0x8018F55C, 3): 12, (0x8018F55C, 4): 6},
}
# Read extents of the same callees' pointer arguments, per overlay (both are
# pure outputs of func_8018F55C).
STACK_EXTENTS_OVL = {
    "ovl_0700": {(0x8018F55C, 3): 0, (0x8018F55C, 4): 0},
}


def out_key(v: int) -> int:
    return v & ((1 << OUT_LEN_SHIFT) - 1)


def out_len(v: int) -> int:
    k = out_key(v)
    return (v >> OUT_LEN_SHIFT) or OUT_EXTENTS.get((k >> 4, k & 15), 8)


def out_bytes(fn: int, k: int, seed: int, n: int | None = None) -> bytes:
    """The bytes a mocked callee writes through an `out=` stack argument (both
    sides): LEN from the spec, else the callee's known extent, else 8."""
    x, b = (fn * 2654435761 + seed * 40503 + k * 0x9E37) & 0xFFFFFFFF, bytearray()
    for _ in range(OUT_EXTENTS.get((fn, k), 8) if n is None else n):
        x = (x * 1103515245 + 12345) & 0xFFFFFFFF
        b.append((x >> 16) & 0xFF)
    return bytes(b)


def prologue_save_instructions(ram: bytearray, entry: int) -> frozenset[int]:
    """Recognize saves of incoming registers, not every store using an s-register.

    Era prologues interleave saves with assignments to other saved registers.
    Follow the entry basic block, retaining only registers not yet assigned,
    and require a first save inside the allocated stack frame.  Any control
    transfer ends this conservative recognition; later stores (including call
    delay slots) are caller data even when their source is s0-s7/fp/ra.  The
    one exception is the delay slot of the transfer that ends the entry block:
    it executes before the transfer takes effect, and era prologues often put
    the last register save there (`beq ...; sw $s0,0x18($sp)`).  It counts as
    a save under the same rule (an incoming register, not yet saved or
    assigned, stored inside the frame).
    """
    incoming = set(range(16, 24)) | {30, 31}
    saved, pcs = set(), set()
    stack_delta = 0

    def is_save(w: int) -> bool:
        op, rs, rt = w >> 26, (w >> 21) & 31, (w >> 16) & 31
        simm = (w & 0xFFFF) - 0x10000 if w & 0x8000 else w & 0xFFFF
        return (op == 0x2B and rs == 29 and rt in incoming and rt not in saved
                and stack_delta < 0 and simm >= 0 and stack_delta + simm + 4 <= 0)

    for offset in range(0, 64 * 4, 4):
        pc = entry + offset
        at = pc & 0x1FFFFFFF
        if at + 4 > RAM_SIZE:
            break
        w = struct.unpack_from("<I", ram, at)[0]
        op, rs, rt, rd = w >> 26, (w >> 21) & 31, (w >> 16) & 31, (w >> 11) & 31
        fn, imm = w & 63, w & 0xFFFF
        simm = imm - 0x10000 if imm & 0x8000 else imm
        if 1 <= op <= 7 or (op == 0 and fn in (8, 9, 12, 13)):
            if at + 8 <= RAM_SIZE:
                slot = struct.unpack_from("<I", ram, at + 4)[0]
                if is_save(slot):
                    pcs.add(pc + 4)
            break
        if op in (0x28, 0x29, 0x2A, 0x2B, 0x2E):
            if is_save(w):
                pcs.add(pc)
                saved.add(rt)
            continue
        written_reg = None
        if op == 0:
            if fn in (0, 2, 3, 4, 6, 7, 16, 18, 32, 33, 34, 35, 36, 37, 38, 39, 42, 43):
                written_reg = rd
            elif fn not in (17, 19, 24, 25, 26, 27):
                break
        elif 8 <= op <= 15 or 0x20 <= op <= 0x26:
            written_reg = rt
        elif op == 0x12:
            if rs in (0, 2):
                written_reg = rt
            elif rs not in (4, 6) and not (rs & 16):
                break
        else:
            break
        if written_reg is not None:
            incoming.discard(written_reg)
            if written_reg == 29:
                if op not in (8, 9) or rs != 29:
                    break
                stack_delta += simm
    return frozenset(pcs)


def narrow_ret(value: int, ty: str | None) -> int:
    """A mocked callee's v0 as its declared return type leaves it: retail code
    returning a 16/8-bit value zero- or sign-extends it, and the caller relies on
    that (no re-masking), so a full 32-bit mock would be a value no real callee
    returns.  The native stub casts to the same type."""
    if not ty:
        return value & 0xFFFFFFFF
    t = re.sub(r"\b(?:extern|static|const|volatile)\b", "", ty).strip()
    signed = not re.search(r"\bunsigned\b|\bu(?:int)?(?:8|16)(?:_t)?\b", t)
    if re.search(r"\bshort\b|\bu?int16_t\b|\b[us]16\b", t):
        return (sx16(value) if signed else value & 0xFFFF) & 0xFFFFFFFF
    if re.search(r"\bchar\b|\bu?int8_t\b|\b[us]8\b", t):
        v = value & 0xFF
        return ((v - 0x100) if signed and v & 0x80 else v) & 0xFFFFFFFF
    return value & 0xFFFFFFFF


def run_mips(ram: bytearray, entry: int, args: list[int], seed: int, outs: frozenset = frozenset(),
             gte_state: list | None = None, ret_types: dict | None = None,
             ret_override: dict | None = None):
    ret_types = ret_types or {}
    ret_override = ret_override or {}
    r = [0] * 32
    r[4:8] = args
    r[29] = SP0
    r[31] = SENTINEL
    r[28] = 0x8009CD70
    hi = lo = 0
    pc, npc = entry, entry + 4
    calls = []
    gte = Gte()

    cur_pc = [entry]

    def addr(a, n):
        if a & (n - 1):
            raise Fault(f"unaligned {n}-byte access {a:08X} at pc {cur_pc[0]:08X}")
        if (a & 0x1FFFFC00) == 0x1F800000:
            return RAM_SIZE + (a & 0x3FF)
        off = (a & 0x1FFFFFFF)
        if off + n > RAM_SIZE:
            raise Fault(f"bad address {a:08X}")
        return off

    def ld(a, n, signed):
        off = addr(a, n)
        v = int.from_bytes(ram[off:off + n], "little")
        if signed and v & (1 << (8 * n - 1)):
            v -= 1 << (8 * n)
        return v & 0xFFFFFFFF

    written: set[int] = set()
    register_saves = prologue_save_instructions(ram, entry)

    def st(a, n, v, mark=True):
        off = addr(a, n)
        ram[off:off + n] = (v & ((1 << (8 * n)) - 1)).to_bytes(n, "little")
        if mark and 0x1FF000 <= off < RAM_SIZE:   # only identified prologue saves are not caller data
            written.update(range(off, off + n))

    steps = 0
    while True:
        steps += 1
        if steps > MAX_STEPS:
            raise Fault("step limit")
        if pc == SENTINEL:
            if gte_state is not None:
                gte_state[:] = [gte.rd(i) for i in range(32)] + [gte.rc(i) for i in range(32)]
            return r[2], calls
        if COVER is not None:
            COVER.add(pc)
        w = ld(pc, 4, False)
        op, rs, rt, rd = w >> 26, (w >> 21) & 31, (w >> 16) & 31, (w >> 11) & 31
        sh, fn, imm = (w >> 6) & 31, w & 63, w & 0xFFFF
        simm = imm - 0x10000 if imm & 0x8000 else imm
        cur, pc, npc = pc, npc, npc + 4
        cur_pc[0] = cur
        A, B = r[rs], r[rt]
        wr = None
        if op == 0:
            if fn == 0x00: wr = (rd, B << sh)
            elif fn == 0x02: wr = (rd, B >> sh)
            elif fn == 0x03: wr = (rd, s32(B) >> sh)
            elif fn == 0x04: wr = (rd, B << (A & 31))
            elif fn == 0x06: wr = (rd, B >> (A & 31))
            elif fn == 0x07: wr = (rd, s32(B) >> (A & 31))
            elif fn == 0x08:                                   # jr
                npc = A
            elif fn == 0x09:                                   # jalr
                tgt = A
                # mocked callee: the delay slot runs first (it may set an
                # argument register), then the call is recorded and "returns"
                pend_ret = narrow_ret(ret_override.get(tgt, mock_ret(tgt, seed)), ret_types.get(tgt))
                run_slot(ram, r, pc, ld, st)
                calls.append((tgt, tuple(r[4:8])))
                r[2] = pend_ret
                pc, npc = cur + 8, cur + 12
                r[rd] = cur + 8 if rd else 0
                r[0] = 0
                continue
            elif fn == 0x0C: raise Fault("syscall")
            elif fn == 0x0D: raise Fault("break")
            elif fn == 0x10: wr = (rd, hi)
            elif fn == 0x11: hi = A
            elif fn == 0x12: wr = (rd, lo)
            elif fn == 0x13: lo = A
            elif fn == 0x18:
                p = s32(A) * s32(B); lo, hi = p & 0xFFFFFFFF, (p >> 32) & 0xFFFFFFFF
            elif fn == 0x19:
                p = A * B; lo, hi = p & 0xFFFFFFFF, (p >> 32) & 0xFFFFFFFF
            elif fn == 0x1A:
                if B == 0: raise Fault("div0")
                a, b = s32(A), s32(B)
                q = abs(a) // abs(b) * (1 if (a >= 0) == (b >= 0) else -1)
                lo, hi = q & 0xFFFFFFFF, (a - q * b) & 0xFFFFFFFF
            elif fn == 0x1B:
                if B == 0: raise Fault("div0")
                lo, hi = A // B, A % B
            elif fn in (0x20, 0x21): wr = (rd, A + B)
            elif fn in (0x22, 0x23): wr = (rd, A - B)
            elif fn == 0x24: wr = (rd, A & B)
            elif fn == 0x25: wr = (rd, A | B)
            elif fn == 0x26: wr = (rd, A ^ B)
            elif fn == 0x27: wr = (rd, ~(A | B))
            elif fn == 0x2A: wr = (rd, int(s32(A) < s32(B)))
            elif fn == 0x2B: wr = (rd, int(A < B))
            else: raise Fault(f"special {fn:#x}")
        elif op == 1:
            if rt in (0x10, 0x11):
                raise Fault("bltzal/bgezal")
            take = (s32(A) < 0) if rt == 0 else (s32(A) >= 0) if rt == 1 else None
            if take is None: raise Fault("regimm")
            if take: npc = cur + 4 + (simm << 2)
        elif op == 2:
            npc = (cur & 0xF0000000) | ((w & 0x3FFFFFF) << 2)
        elif op == 3:
            tgt = (cur & 0xF0000000) | ((w & 0x3FFFFFF) << 2)
            run_slot(ram, r, pc, ld, st)
            sp = (r[29] + 16) & 0x1FFFFFFF
            av = tuple(r[4:8]) + struct.unpack_from("<12I", ram, sp)
            # stack-local arguments: frames differ between the sides, so the
            # pointed-to bytes (at call time) are compared instead of the address
            calls.append((tgt, tuple(("S", bytes(ram[(v & 0x1FFFFFFF):(v & 0x1FFFFFFF) + 32]),
                                      [((v & 0x1FFFFFFF) + j) in written for j in range(32)], v)
                                     if 0x801FF000 <= v < SP0 + 0x100 else v for v in av)))
            for k in range(len(av)):
                n_ = next((out_len(v) for v in outs if out_key(v) == (tgt << 4 | k)), 0)
                if n_ and 0x801FF000 <= av[k] < SP0 + 0x100:
                    o_ = av[k] & 0x1FFFFFFF
                    ram[o_:o_ + n_] = out_bytes(tgt, k, seed, n_)
                    written.update(range(o_, o_ + n_))
            r[2] = narrow_ret(ret_override.get(tgt, mock_ret(tgt, seed)), ret_types.get(tgt))
            r[31] = cur + 8
            pc, npc = cur + 8, cur + 12
            continue
        elif op == 4:
            if A == B: npc = cur + 4 + (simm << 2)
        elif op == 5:
            if A != B: npc = cur + 4 + (simm << 2)
        elif op == 6:
            if s32(A) <= 0: npc = cur + 4 + (simm << 2)
        elif op == 7:
            if s32(A) > 0: npc = cur + 4 + (simm << 2)
        elif op in (8, 9): wr = (rt, A + simm)
        elif op == 10: wr = (rt, int(s32(A) < simm))
        elif op == 11: wr = (rt, int(A < (simm & 0xFFFFFFFF)))
        elif op == 12: wr = (rt, A & imm)
        elif op == 13: wr = (rt, A | imm)
        elif op == 14: wr = (rt, A ^ imm)
        elif op == 15: wr = (rt, imm << 16)
        elif op == 0x12:                                        # COP2
            if w & (1 << 25):
                gte.command(w & 0x1FFFFFF)
            elif rs == 0: wr = (rt, gte.rd(rd))                 # mfc2
            elif rs == 2: wr = (rt, gte.rc(rd))                 # cfc2
            elif rs == 4: gte.wd(rd, B)                         # mtc2
            elif rs == 6: gte.wc(rd, B)                         # ctc2
            else: raise Fault("cop2 form")
        elif op == 0x32: gte.wd(rt, ld(A + simm, 4, False))     # lwc2
        elif op == 0x3A: st(A + simm, 4, gte.rd(rt))            # swc2
        elif op == 0x20: wr = (rt, ld(A + simm, 1, True))
        elif op == 0x21: wr = (rt, ld(A + simm, 2, True))
        elif op == 0x23: wr = (rt, ld(A + simm, 4, False))
        elif op == 0x24: wr = (rt, ld(A + simm, 1, False))
        elif op == 0x25: wr = (rt, ld(A + simm, 2, False))
        elif op in (0x22, 0x26):                                # lwl / lwr
            a = (A + simm) & 0xFFFFFFFF
            al = ld(a & ~3, 4, False)
            k = a & 3
            if op == 0x22:
                sh_ = (3 - k) * 8
                v = (B & (0xFFFFFFFF >> (32 - sh_) if sh_ else 0)) | ((al << sh_) & 0xFFFFFFFF)
            else:
                sh_ = k * 8
                v = (B & ~(0xFFFFFFFF >> sh_) & 0xFFFFFFFF) | (al >> sh_)
            wr = (rt, v)
        elif op == 0x28: st(A + simm, 1, B)
        elif op == 0x29: st(A + simm, 2, B)
        elif op == 0x2B: st(A + simm, 4, B, cur not in register_saves)
        elif op in (0x2A, 0x2E):                                # swl / swr
            a = (A + simm) & 0xFFFFFFFF
            al = ld(a & ~3, 4, False)
            k = a & 3
            if op == 0x2A:
                sh_ = (3 - k) * 8
                mask = 0xFFFFFFFF >> sh_
                v = (al & ~mask & 0xFFFFFFFF) | (B >> sh_)
            else:
                sh_ = k * 8
                mask = (0xFFFFFFFF << sh_) & 0xFFFFFFFF
                v = (al & ~mask & 0xFFFFFFFF) | ((B << sh_) & 0xFFFFFFFF)
            st(a & ~3, 4, v)
        else:
            raise Fault(f"opcode {op:#x}")
        if wr is not None and wr[0]:
            r[wr[0]] = wr[1] & 0xFFFFFFFF
        r[0] = 0
        if npc != pc + 4 and cur + 4 == pc:
            pass


def run_slot(ram, r, pc, ld, st):
    """Execute the single delay-slot instruction of a jal/jalr (ALU/load/store)."""
    if COVER is not None:
        COVER.add(pc)
    w = ld(pc, 4, False)
    op, rs, rt, rd = w >> 26, (w >> 21) & 31, (w >> 16) & 31, (w >> 11) & 31
    sh, fn, imm = (w >> 6) & 31, w & 63, w & 0xFFFF
    simm = imm - 0x10000 if imm & 0x8000 else imm
    A, B = r[rs], r[rt]
    v = None
    if w == 0:
        return
    if op == 0 and fn == 0x21: v = (rd, A + B)
    elif op == 0 and fn == 0x25: v = (rd, A | B)
    elif op == 0 and fn == 0x23: v = (rd, A - B)
    elif op == 0 and fn == 0x00: v = (rd, B << sh)
    elif op == 0 and fn == 0x02: v = (rd, B >> sh)
    elif op == 0 and fn == 0x03: v = (rd, s32(B) >> sh)
    elif op == 0 and fn == 0x24: v = (rd, A & B)
    elif op == 0 and fn == 0x2A: v = (rd, int(s32(A) < s32(B)))
    elif op == 0 and fn == 0x2B: v = (rd, int(A < B))
    elif op in (8, 9): v = (rt, A + simm)
    elif op == 10: v = (rt, int(s32(A) < simm))
    elif op == 12: v = (rt, A & imm)
    elif op == 13: v = (rt, A | imm)
    elif op == 15: v = (rt, imm << 16)
    elif op == 0x20: v = (rt, ld(A + simm, 1, True))
    elif op == 0x21: v = (rt, ld(A + simm, 2, True))
    elif op == 0x23: v = (rt, ld(A + simm, 4, False))
    elif op == 0x24: v = (rt, ld(A + simm, 1, False))
    elif op == 0x25: v = (rt, ld(A + simm, 2, False))
    elif op == 0x28: st(A + simm, 1, B)
    elif op == 0x29: st(A + simm, 2, B)
    elif op == 0x2B: st(A + simm, 4, B)
    else:
        raise Fault(f"delay-slot op {w:08X}")
    if v is not None and v[0]:
        r[v[0]] = v[1] & 0xFFFFFFFF
    r[0] = 0


MOCK_H = r"""
#pragma once
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
typedef uint32_t pe_addr_t;
typedef struct { int16_t x, y, w, h; } RECT;   /* pc_port/include/psx_compat.h */
extern uint8_t ram[0x200400];
static inline uint8_t *at(pe_addr_t a) { if ((a & 0x1FFFFC00u) == 0x1F800000u) return ram + 0x200000u + (a & 0x3FFu); if ((a & 0x1FFFFFFFu) + 4u > 0x200000u) { printf("FAULT %08X\n", a); exit(3); } return ram + (a & 0x1FFFFFFFu); }
uint32_t PE_GTE_ReadData(uint32_t reg);
void PE_GTE_WriteData(uint32_t reg, uint32_t value);
uint32_t PE_GTE_ReadCtrl(uint32_t reg);
void PE_GTE_WriteCtrl(uint32_t reg, uint32_t value);
void PE_GTE_Command(uint32_t cmd);
static inline uint8_t  PE_LoadU8(pe_addr_t a)  { return *at(a); }
static inline uint16_t PE_LoadU16(pe_addr_t a) { uint16_t v; memcpy(&v, at(a), 2); return v; }
static inline uint32_t PE_LoadU32(pe_addr_t a) { uint32_t v; memcpy(&v, at(a), 4); return v; }
static inline void PE_StoreU8(pe_addr_t a, uint8_t v)   { *at(a) = v; }
static inline void PE_StoreU16(pe_addr_t a, uint16_t v) { memcpy(at(a), &v, 2); }
static inline void PE_StoreU32(pe_addr_t a, uint32_t v) { memcpy(at(a), &v, 4); }
static inline void PE_Fill(pe_addr_t a, uint32_t n, uint8_t v) { memset(at(a), v, n); }
static inline void *PE_Translate(pe_addr_t a, size_t n) { (void)n; return at(a); }
/* a host pointer into the mock RAM, back as its guest address (host stubs log it) */
static inline pe_addr_t mock_guest(const void *p) { size_t o = (size_t)((const uint8_t *)p - ram); return p ? (o >= 0x200000u ? 0x1F800000u + (pe_addr_t)(o - 0x200000u) : 0x80000000u + (pe_addr_t)o) : 0u; }
pe_addr_t PE_GuestFrame_Enter(uint32_t n);
void PE_GuestFrame_Leave(pe_addr_t b, uint32_t n);
uint32_t mock_call(uint32_t fn, unsigned n, uint32_t a0, uint32_t a1, uint32_t a2, uint32_t a3);
static inline int PE_GuestCall(const char *s, pe_addr_t fn, unsigned n, uintptr_t a0, uintptr_t a1, uintptr_t a2, uintptr_t a3)
{ (void)s; return (int)mock_call(fn, n, (uint32_t)a0, (uint32_t)a1, (uint32_t)a2, (uint32_t)a3); }
#define PE_D_COMP_A(_1, _2, _3, _4, ...) (uint32_t)(uintptr_t)(_1), (uint32_t)(uintptr_t)(_2), (uint32_t)(uintptr_t)(_3), (uint32_t)(uintptr_t)(_4)
static inline void PE_Decomp_GuestTrapDivZero(const char *fn) { (void)fn; printf("GUEST_TRAP\n"); exit(4); }
#define PE_D_COMP_BOUNDARY0(s, v) (int)mock_call(v, 0u, 0, 0, 0, 0)
#define PE_D_COMP_BOUNDARY1(s, v, ...) (int)mock_call(v, 1u, PE_D_COMP_A(__VA_ARGS__, 0, 0, 0, 0))
#define PE_D_COMP_BOUNDARY2(s, v, ...) (int)mock_call(v, 2u, PE_D_COMP_A(__VA_ARGS__, 0, 0, 0, 0))
#define PE_D_COMP_BOUNDARY3(s, v, ...) (int)mock_call(v, 3u, PE_D_COMP_A(__VA_ARGS__, 0, 0, 0, 0))
#define PE_D_COMP_BOUNDARY4(s, v, ...) (int)mock_call(v, 4u, PE_D_COMP_A(__VA_ARGS__, 0, 0, 0, 0))
"""

GTE_EXT = r"""
#include <stdint.h>
extern uint8_t ram[0x200400];
static uint8_t *ext_at(uint32_t a) { return (a & 0x1FFFFC00u) == 0x1F800000u ? ram + 0x200000u + (a & 0x3FFu) : ram + (a & 0x1FFFFFu); }
uint16_t PE_LoadU16(uint32_t a) { uint8_t *p = ext_at(a); return (uint16_t)(p[0] | p[1] << 8); }
uint32_t PE_LoadU32(uint32_t a) { uint8_t *p = ext_at(a); return p[0] | p[1] << 8 | p[2] << 16 | (uint32_t)p[3] << 24; }
void PE_Port_RequestStop(const char *r) { (void)r; }
"""

HARNESS = r"""
uint8_t ram[0x200400];
static pe_addr_t fsp = 0x801FEC00u;
pe_addr_t PE_GuestFrame_Enter(uint32_t n) { fsp -= (n + 7u) & ~7u; return fsp; }
void PE_GuestFrame_Leave(pe_addr_t b, uint32_t n) { (void)b; fsp += (n + 7u) & ~7u; }
static unsigned seed_;
static FILE *logf_;
static uint32_t rov_fn_[32], rov_v_[32];   /* directed r(FN)=V returns */
static unsigned rov_n_;
static int rov_get(uint32_t fn, uint32_t *v)
{
    unsigned k;
    for (k = 0; k < rov_n_; k++) if (rov_fn_[k] == fn) { *v = rov_v_[k]; return 1; }
    return 0;
}
uint32_t mock_call(uint32_t fn, unsigned n, uint32_t a0, uint32_t a1, uint32_t a2, uint32_t a3)
{
    uint32_t a[4] = {a0, a1, a2, a3}, h = fn * 2654435761u + seed_ * 40503u;
    unsigned i;
    fprintf(logf_, "C %08X", fn);
    for (i = 0; i < n && i < 4; i++) {
        if (a[i] >= 0x801FE000u && a[i] < 0x801FF000u) {   /* a guest-frame pointer, as mock_args */
            unsigned j;
            fprintf(logf_, " S:%08X:", a[i]);
            for (j = 0; j < 32; j++) fprintf(logf_, "%02X", ram[(a[i] & 0x1FFFFFu) + j]);
        } else
            fprintf(logf_, " %08X", a[i]);
    }
    fprintf(logf_, "\n");
    { uint32_t v; if (rov_get(fn, &v)) return v; }
    if (((seed_ * 2654435761u) >> 16) & 1u) return (uint32_t)((int32_t)(h & 0x1FFFu) - 0x1000);
    return 0x80100000u + (h & 0xDFFFCu);
}
uint32_t mock_args(uint32_t fn, unsigned n, const uint32_t *a)
{
    uint32_t h = fn * 2654435761u + seed_ * 40503u;
    unsigned i;
    fprintf(logf_, "C %08X", fn);
    for (i = 0; i < n && i < 16; i++) {
        if (a[i] >= 0x801FE000u && a[i] < 0x801FF000u) {
            unsigned j;
            fprintf(logf_, " S:%08X:", a[i]);
            for (j = 0; j < 32; j++) fprintf(logf_, "%02X", ram[(a[i] & 0x1FFFFFu) + j]);
        } else
            fprintf(logf_, " %08X", a[i]);
    }
    fprintf(logf_, "\n");
    for (i = 0; i < n && i < 16; i++) {
        unsigned j, q;
        uint32_t x;
        for (q = 0; q < sizeof outs_ / sizeof outs_[0]; q++)
            if (outs_[q] == (fn << 4 | i) && a[i] >= 0x801FE000u && a[i] < 0x801FF000u) {
                x = fn * 2654435761u + seed_ * 40503u + i * 0x9E37u;
                for (j = 0; j < outl_[q]; j++) {
                    x = x * 1103515245u + 12345u;
                    ram[(a[i] & 0x1FFFFFu) + j] = (uint8_t)(x >> 16);
                }
            }
    }
    { uint32_t v; if (rov_get(fn, &v)) return v; }
    if (((seed_ * 2654435761u) >> 16) & 1u) return (uint32_t)((int32_t)(h & 0x1FFFu) - 0x1000);
    return 0x80100000u + (h & 0xDFFFCu);
}
int main(int argc, char **argv)
{
    FILE *f = fopen(argv[1], "rb");
    uint32_t a0 = strtoul(argv[3], 0, 16), a1 = strtoul(argv[4], 0, 16),
             a2 = strtoul(argv[5], 0, 16), a3 = strtoul(argv[6], 0, 16);
    seed_ = (unsigned)strtoul(argv[7], 0, 10);
    for (int k = 9; k < argc && rov_n_ < 32; k++) {   /* FN=V pairs */
        char *eq = strchr(argv[k], '=');
        if (!eq) continue;
        rov_fn_[rov_n_] = (uint32_t)strtoul(argv[k], 0, 16);
        rov_v_[rov_n_++] = (uint32_t)strtoul(eq + 1, 0, 16);
    }
    if (!f || fread(ram, 1, sizeof ram, f) != sizeof ram) return 2;
    fclose(f);
    logf_ = fopen(argv[8], "w");
    (void)a0; (void)a1; (void)a2; (void)a3;
    CALL_LINE
    fprintf(logf_, "G");
    for (unsigned i = 0; i < 32; i++) fprintf(logf_, " %08X", PE_GTE_ReadData(i));
    for (unsigned i = 0; i < 32; i++) fprintf(logf_, " %08X", PE_GTE_ReadCtrl(i));
    fprintf(logf_, "\n");
    fclose(logf_);
    f = fopen(argv[2], "wb"); fwrite(ram, 1, sizeof ram, f); fclose(f);
    return 0;
}
"""


EXE_IMAGE = Path("build/extracted/disc1/SLUS_006.62")


def exe_overlay() -> dict:
    """The EXE as a pseudo-overlay: the SLUS file mapped so that its offset 0x800
    lands at 0x80010000, every matched and NON_MATCHING EXE leaf by name."""
    import gen_decomp_ports as g
    leaves = {n: lf for n, lf in g.parse_matched().items()}
    leaves.update(g.nonmatching_exe_leaves())
    return {"id": "exe", "vram": g.EXE_FILE_VRAM, "end": 0x800E4000, "leaves": leaves}


def candidates(with_calls: bool = False) -> list[tuple[Path, str, str]]:
    """Generated E21 room TUs to differential-test.  By default a matched TU
    that makes calls is left out (NON_MATCHING TUs with calls are always in);
    with_calls takes those too: their host-prototyped callees are mocked
    exactly like NON_MATCHING ones (host_stubs)."""
    out = []
    tus = sorted((o.REPO_ROOT / o.ROOMS_DIR).glob("*_port.c"))
    # subsystem overlays (ovl_0457 credits, ovl_03D2 title, ...): one TU per leaf
    tus += sorted((o.REPO_ROOT / o.OUT_ROOT).glob("ovl_*/*_port.c"))
    # E21-lowered EXE leaves (matched and NON_MATCHING): pc_port/game/decomp
    for p in sorted((o.REPO_ROOT / "pc_port/game/decomp").glob("func_*_port.c")):
        t = p.read_text()
        if "E21 guest-memory" not in t:
            continue
        name = p.name[: -len("_port.c")]
        body = re.sub(r'"[^"]*"', '""', t.split("*/", 1)[1])
        calls = set(re.findall(r"\b(func_[0-9A-F]{8})\s*\(", body)) - {name}
        if "PE_HostToGuest" in t or ((calls or "host prototypes" in t) and not with_calls
                                     and "(NON_MATCHING)" not in t):
            continue
        out.append((p, name, "exe"))
    for p in tus:
        t = p.read_text()
        if "E21 guest-memory" not in t:
            continue
        name = p.name.split("__")[0][: -len("_port.c")] if p.name.endswith("_port.c") and "__" not in p.name \
            else p.name.split("__")[0]
        body = re.sub(r'"[^"]*"', '""', t.split("*/", 1)[1])
        calls = set(re.findall(r"\b(func_[0-9A-F]{8})\s*\(", body)) - {name}
        m0 = re.search(r"decomp-source-overlay: ((?:room|ovl)_\w+)/", t)
        if not m0:
            continue
        nonmatching = name in o.nonmatching_leaves().get(m0.group(1), {})
        if "PE_HostToGuest" in t or ((calls or "host prototypes" in t) and not nonmatching
                                     and not with_calls):
            continue
        out.append((p, name, m0.group(1)))
    return out


PROTO_RE = re.compile(r"^(\w[\w \*]*?)\s*\b(func_[0-9A-F]{8}|PE_\w+)\((.*?)\);", re.M)


def host_prototypes(t: str):
    """Native prototypes paired with the retail address being replaced.

    Renamed SDK replacements (for example PE_RotMatrixYXZ_Guest) must be
    called under their host name while the differential logs the retail jal
    target.  Unknown host names are refused, never silently left untested.
    """
    if "host prototypes" not in t:
        return
    replacements = {v["host"]: int(k[5:], 16)
                    for k, v in o.host_replacements().items()}
    block = t.split("host prototypes", 1)[1].split("\n\n", 1)[0]
    for ret, fn, prm in PROTO_RE.findall(block):
        if fn.startswith("func_"):
            address = int(fn[5:], 16)
        else:
            if fn not in replacements:
                raise ValueError(f"e21_diff: no retail identity for host replacement {fn}")
            address = replacements[fn]
        yield ret, fn, prm, address


def narrow_mask(ty: str) -> int:
    """Compare mask for a host parameter type: a 16/8-bit parameter carries only
    its low bits, whatever the caller's register held above them (the callee's
    own e21 test feeds it full registers)."""
    if re.search(r"\bshort\b|\bu?int16_t\b|\b[us]16\b", ty):
        return 0xFFFF
    if re.search(r"\bchar\b|\bu?int8_t\b|\b[us]8\b", ty):
        return 0xFF
    return 0xFFFFFFFF


# printf (func_80071A74, the BIOS A(3Fh) stub): its arguments after the format
# only shape debug TTY text, never guest state, and the host replacement
# (PE_Printf_Guest) receives 0 where a call site passes fewer than seven.
# Compare the format pointer only.
TTY_ONLY_ARGS = {0x80071A74: [0xFFFFFFFF, 0, 0, 0, 0, 0, 0, 0]}


def dead_param_masks(t: str) -> dict[int, list[int]]:
    """Arguments guest_lower.DEAD_TRAILING_PARAMS documents as never read by the
    callee (func_8010C89C's a3 is recomputed from a2 before its first use): the
    lowering passes 0 there, retail leaves whatever the register held."""
    out = {}
    for name, live in gl.DEAD_TRAILING_PARAMS.items():
        if name == "func_80071A74":
            continue
        out[int(name[5:], 16)] = [0xFFFFFFFF] * live + [0] * (8 - live)
    return out


def host_masks(t: str) -> dict[int, list[int]]:
    """{callee vma: per-argument compare mask} for host-prototyped callees."""
    out = {}
    for _ret, fn, prm, address in host_prototypes(t):
        ps = [] if prm.strip() == "void" else [x.strip() for x in prm.split(",")]
        out[address] = [narrow_mask(ty) for ty in ps]
    return out


_C_TYPE_WORDS = {"int", "char", "short", "long", "unsigned", "signed", "float", "double",
                 "void", "const", "volatile"}


def param_is_named(ty: str) -> bool:
    """True when a prototype parameter carries a name.  Qualifiers aside, one
    token is a bare type (`uint32_t`, `RECT`), all-keyword tokens are a
    multi-word C type (`unsigned int`), `struct X` is a type; otherwise the
    last token is the name (`unsigned code`, `uint32_t a_t`, `RECT *r`)."""
    toks = [w for w in re.findall(r"\w+", ty) if w not in ("const", "volatile")]
    if len(toks) <= 1:
        return False
    if all(w in _C_TYPE_WORDS for w in toks):
        return False
    if len(toks) == 2 and toks[0] in ("struct", "union", "enum"):
        return False
    return True


def host_stubs(t: str) -> str:
    """Mock definitions for a TU's host-prototyped callees (NON_MATCHING leaves):
    every argument is logged (stack args too, as retail passes them at sp+16+4k),
    narrow parameters masked to their width so they compare with the retail
    register/stack word; the return value is the same seed-derived mock as the
    retail interpreter's jal."""
    out = []
    for ret, fn, prm, address in host_prototypes(t):
        ps = [] if prm.strip() == "void" else [x.strip() for x in prm.split(",")]
        names, logs = [], []
        for k, ty in enumerate(ps):
            nm = f"p{k}"
            named = param_is_named(ty)
            names.append(re.sub(r"\w+$", nm, ty) if named else f"{ty} {nm}")
            w = f"0x{narrow_mask(ty):X}u"
            if "*" in ty:            # host pointer parameter: log the guest address it maps
                logs.append(f"(uint32_t)mock_guest({nm})")
            else:
                logs.append(f"(uint32_t)(uintptr_t){nm} & {w}")
        if len(ps) > 16:
            raise ValueError(f"e21_diff: too many arguments for {fn}: {len(ps)}")
        body = (f"uint32_t a_[16] = {{{', '.join(logs) or '0'}}}; "
                f"uint32_t v_ = mock_args(0x{address:08X}u, {len(ps)}u, a_);")
        retst = " (void)v_;" if ret.strip() == "void" else f" return ({ret.strip()})v_;"
        out.append(f"{ret} {fn}({', '.join(names) or 'void'}) {{ {body}{retst} }}")
    # Host replacements (pe_port_compat.h `host-replacement: func_X`): the lowered
    # TU calls PE_* directly; mock it as the retail jal to func_X.
    for fx, r in o.host_replacements().items():
        if not re.search(r"\b" + r["host"] + r"\s*\(", t):
            continue
        if any(re.search(r"\b" + r["host"] + r"\(", x) for x in out):
            continue
        ps = [x.strip() for x in r["params"] if x.strip() not in ("", "void")]
        nm = [f"p{k}" for k in range(len(ps))]
        decl = ", ".join(f"pe_addr_t {n}" if "pe_addr_t" in ty else f"{re.sub(r'\s*\w+$', '', ty) or ty} {n}"
                         for ty, n in zip(ps, nm))
        logs = ", ".join(f"(uint32_t)(uintptr_t){n}" for n in nm) or "0"
        retst = "" if r["ret"] == "void" else f" return ({r['ret']})v_;"
        out.append(f"{r['ret']} {r['host']}({decl or 'void'}) {{ uint32_t a_[16] = {{{logs}}}; "
                   f"uint32_t v_ = mock_args(0x{fx[5:]}u, {len(ps)}u, a_);{retst} }}")
    return "\n" + "\n".join(out) + "\n"


# Read extents of pointer arguments used by park effects.  Comparing adjacent
# caller locals would require identical stack layout, which native C does not
# promise.  Matrices include their three translations, not just rotation.
STACK_EXTENTS = {
    (0x80078C34, 0): 18, (0x80078C34, 1): 6, (0x80078C34, 2): 0,   # ApplyMatrixSV
    (0x800C3134, 2): 0,                     # pure output (3 colour bytes)
    (0x800C5EB0, 1): 0, (0x800C5EB0, 2): 0,  # a1 unread; a2 pure output
    (0x800D3114, 10): 4, (0x800D3114, 11): 4,  # col0/col1: one CV each (r,g,b read)
    (0x8007506C, 0): 8, (0x800750CC, 0): 8, (0x8007512C, 0): 8,   # one RECT
    (0x80074F44, 0): 8,                     # ClearImage: one RECT
    (0x800794C4, 0): 6, (0x800794C4, 1): 0,
    (0x800C4FC4, 1): 32,                                            # ring renderer matrix
    (0x80079754, 0): 6, (0x80079754, 1): 0,
    (0x80078CC4, 0): 20, (0x80078CC4, 1): 12,
    # pure outputs (the callee writes them before any read): nothing to compare
    (0x80078134, 1): 0, (0x800792D4, 1): 0, (0x800792D4, 2): 0,
    (0x8006DFA8, 1): 0, (0x8006DFA8, 2): 0,
    (0x800C71E4, 1): 32,
    (0x800C6B90, 0): 6,
    (0x800CE8F0, 2): 6, (0x800CE8F0, 3): 0,
    (0x800CE9D4, 2): 0,
    (0x800CEAE8, 0): 18, (0x800CEAE8, 1): 6, (0x800CEAE8, 2): 0,
    (0x800CFB7C, 0): 6, (0x800CFB7C, 2): 0,
    (0x800CF3AC, 1): 0,
    (0x800CEE20, 0): 6, (0x800CEE20, 1): 8, (0x800CEE20, 8): 3,
    (0x800D2B58, 0): 6, (0x800D2B58, 1): 6,
    (0x800D2B58, 2): 3, (0x800D2B58, 3): 3,
    (0x800D2370, 0): 6, (0x800D2370, 1): 6,
    (0x800D2370, 9): 3, (0x800D2370, 10): 3,
    (0x800D004C, 0): 6, (0x800D004C, 4): 8,
    (0x800D004C, 7): 3, (0x800D004C, 8): 3,
    # func_800D1384 (trail-ribbon GTE wrapper): col0 = a3 -> t2, col1 = sp+0x90 -> t4,
    # each read only as lbu 0/1/2 (retail 800D1538..800D1604).
    (0x800D1384, 3): 3, (0x800D1384, 4): 3,
    # func_800D1DEC (glow point): pos = a0 -> lwc2 $0/$1 at 0/4 (VZ0 keeps the
    # low halfword: 6 bytes); col = a1 -> s2, read only as lbu 0/1/2
    # (retail 800D1E60/64 and 800D1E90..800D1EE0).
    (0x800D1DEC, 0): 6, (0x800D1DEC, 1): 3,
}


def directed_specs() -> dict[tuple[str, str], list[tuple[str, int, list[int]]]]:
    """Directed inputs for NON_MATCHING leaves, from configs/USA/overlays/nonmatching.yaml:
    `inputs: [ovl, name, "a0=1,2; h(a1+0x10)=0..5"]`.  Each seed takes the
    (seed mod product)-th combination: `aN=` sets argument N, `h(aN+OFF)=` /
    `w(aN+OFF)=` store a halfword / word there (value lists or LO..HI ranges);
    `r(0xADDR)=` makes the mocked callee at ADDR return that value (on the
    retail oracle and in the native stub alike) instead of the seeded mock,
    which a busy-wait on a callee's status needs.
    A leaf may have several lines (one per mode, say): each is a spec of its
    own and runs the full seed count, so the value is a list of specs."""
    out: dict = {}
    if not o.NONMATCH_MANIFEST.exists():
        return out
    for m in re.finditer(r'^\s*-\s*\[\s*(\w+)\s*,\s*(\w+)\s*,\s*"([^"]*)"\s*\]',
                         o.NONMATCH_MANIFEST.read_text().split("inputs:", 1)[-1], re.M):
        out.setdefault((m.group(1), m.group(2)), []).append(parse_directed(m.group(3)))
    return out


def parse_directed(spec: str) -> list[tuple[str, list[int]]]:
    """One directed-input spec string (see directed_specs) as its term list."""
    terms = []
    if True:
        for t in spec.split(";"):
            if not t.strip():
                continue
            lhs, rhs = (x.strip() for x in t.split("="))
            vals: list[int] = []
            if lhs == "out":                   # out=FN:K[:LEN],...: callee FN writes LEN (8) bytes through stack arg K
                terms.append((lhs, [int(x.split(":")[0], 16) << 4 | int(x.split(":")[1])
                                    | (int(x.split(":")[2], 0) << OUT_LEN_SHIFT if x.count(":") == 2 else 0)
                                    for x in rhs.split(",")]))
                continue
            for v in rhs.split(","):
                if ".." in v:
                    lo_, hi_ = (int(x, 0) for x in v.split(".."))
                    vals += list(range(lo_, hi_ + 1))
                else:
                    vals.append(int(v, 0))
            terms.append((lhs, vals))
    return terms


# Explicit seeds instead of range(seeds) (e21_leaf.py --seed-list): a seed is
# the RNG key of one random RAM image and of every mocked callee's return
# value, so a listed seed reproduces one exact input, for example the first
# seeds whose mock makes a callee return 0.
SEED_LIST: list[int] | None = None


def inputs(name, ov, data, seeds, recorded, spec, seed_range=None):
    """(seed, ram, args): recorded game snapshots if given, else seeded random
    RAM (pointer-heavy) with the directed terms applied on top.  seed_range,
    when given, replaces SEED_LIST / range(seeds) (the E21_MIN_OK extension)."""
    if recorded:
        for k, f in enumerate(sorted(Path(recorded).glob("rec_*.bin"))):
            b = f.read_bytes()
            if len(b) != RAM_SIZE + SPAD + 16:
                continue
            lf = ov["leaves"][name]
            at_ = lf["vram"] & 0x1FFFFFFF
            if b[at_:at_ + lf["file_size"]] != data[lf["file_offset"]:lf["file_offset"] + lf["file_size"]]:
                continue                       # another overlay's code was resident: not this leaf
            yield k, bytearray(b[:RAM_SIZE + SPAD]), list(struct.unpack_from("<4I", b, RAM_SIZE + SPAD)), {}
        return
    specs = spec if spec and isinstance(spec[0], list) else [spec]
    for spec, seed in ((sp, sd) for sp in specs
                       for sd in (seed_range if seed_range is not None else
                                  SEED_LIST if SEED_LIST is not None else range(seeds))):
        rng = random.Random(f"{name}:{seed}")
        ram = bytearray(RAM_SIZE + SPAD)
        # Seeded pointers stay below both stack windows (as mock returns do):
        # an argument or RAM pointer into the retail frame aliases the leaf's
        # own locals there but not in the native guest frame.
        words = [0x80100000 + ((rng.getrandbits(18) << 2) & 0xDFFFC) if rng.random() < 0.7
                 else rng.choice((0, 1, 2, 3, 0xFF, 0xFFFF, rng.getrandbits(32)))
                 for _ in range((RAM_SIZE + SPAD) // 4)]
        ram[:] = struct.pack(f"<{len(words)}I", *words)
        base = ov["vram"] & 0x1FFFFFFF
        ram[base:base + len(data)] = data
        args = [0x80100000 + ((rng.getrandbits(18) << 2) & 0xDFFFC) for _ in range(4)]
        rets: dict[int, int] = {}
        if spec:
            k = seed
            for lhs, vals in spec:
                if lhs == "out":
                    continue
                v = vals[k % len(vals)]
                k //= len(vals)
                mm = re.fullmatch(r"r\((0x[0-9A-Fa-f]+)\)", lhs)
                if mm:
                    rets[int(mm.group(1), 16)] = v & 0xFFFFFFFF
                    continue
                mm = re.fullmatch(r"a(\d)", lhs)
                if mm:
                    args[int(mm.group(1))] = v & 0xFFFFFFFF
                    continue
                mm = re.fullmatch(r"([hw])\((?:a(\d)\+)?(0x[0-9A-Fa-f]+|\d+)\)", lhs)
                if not mm:
                    raise SystemExit(f"e21_diff: bad directed term {lhs!r}")
                base_ = args[int(mm.group(2))] if mm.group(2) else 0
                at = (base_ + int(mm.group(3), 0)) & 0x1FFFFFFF
                struct.pack_into("<H" if mm.group(1) == "h" else "<I", ram, at,
                                 v & (0xFFFF if mm.group(1) == "h" else 0xFFFFFFFF))
        yield seed, ram, args, rets


def e21_cflags(gte: bool = False) -> list[str]:
    """Native-side optimisation flags, -O1 by default.  E21_CFLAGS sets them
    for the TU and E21_GTE_CFLAGS for pe_gte.c / pe_gte_commands.c.  The
    production build compiles decomp TUs with `-O3 -fno-strict-aliasing
    -fwrapv` (pc_port/CMakeLists.txt) and the GTE units with plain -O3."""
    return (os.environ.get("E21_GTE_CFLAGS" if gte else "E21_CFLAGS") or "-O1").split()


def retail_frame_size(data: bytes, leaf: dict) -> int:
    """The retail prologue's `addiu $sp,$sp,-N` (first 16 words), or 0."""
    off = leaf["file_offset"]
    for i in range(16):
        if off + 4 * i + 4 > len(data):
            break
        w = struct.unpack_from("<I", data, off + 4 * i)[0]
        if w >> 16 == 0x27BD and w & 0x8000:
            return 0x10000 - (w & 0xFFFF)
    return 0


def native_frame_size(text: str, name: str) -> int:
    """The guest frame the lowered public wrapper of `name` enters, or 0."""
    m = re.search(r"^\w[\w \*]*\b" + name + r"\([^;{]*\)\s*\{(.*?)^\}", text, re.M | re.S)
    g = re.search(r"PE_GuestFrame_Enter\((\d+)u?\)", m.group(1)) if m else None
    return int(g.group(1)) if g else 0


def one(p: Path, name: str, ovl: str, seeds: int, cc: str, cover: bool = False,
        recorded: str = "", directed: dict | None = None,
        check_gte: bool = False) -> tuple[str, str]:
    directed = directed or {}
    global COVER
    COVER = set() if cover else None
    if ovl == "exe":
        ov, blob = exe_overlay(), o.REPO_ROOT / EXE_IMAGE
        if not blob.is_file():
            return "noblob", ""
    else:
        ov = o.parse_overlay(ovl)
        blob = o.blob_path(ovl)
    if blob is None:
        return "noblob", ""
    leaf = ov["leaves"][name]
    data = blob.read_bytes()
    t = p.read_text()
    sig = re.search(r"^(\w+) " + name + r"\((.*?)\)\s*\{", t, re.M)
    nparams = 0 if sig.group(2).strip() == "void" else len(sig.group(2).split(","))
    regs = ["a0", "a1", "a2", "a3"] + [f"PE_LoadU32(0x{SP0 + 16 + 4 * k:08X}u)" for k in range(8)]
    argl = ", ".join(regs[:nparams])
    if sig.group(1) == "void":
        call = f"{name}({argl}); fprintf(logf_, \"R -\\n\");"
    else:
        call = (f"uint32_t rv = (uint32_t)(int32_t){name}({argl}); "
                f"fprintf(logf_, \"R %08X\\n\", rv);")
    stubs = host_stubs(t)
    masks = {**host_masks(t), **TTY_ONLY_ARGS, **dead_param_masks(t)}
    ret_types = {address: ret for ret, _fn, _prm, address in host_prototypes(t)}
    with tempfile.TemporaryDirectory() as d:
        d = Path(d)
        (d / "pe_guest_decomp.h").write_text(MOCK_H)
        dspec = directed.get((ovl, name)) or []
        if dspec and not isinstance(dspec[0], list):
            dspec = [dspec]                    # one spec (e21_leaf --inputs)
        outs = frozenset(v for sp in dspec for lhs, vs in sp if lhs == "out" for v in vs)
        # A callee whose output extent is known writes it on both sides even
        # without a directed out= term, so a caller that reads its own stack
        # buffer back after the call (a RotMatrix into a local matrix) compares
        # the same bytes instead of two different stack fills.
        keyed = {out_key(v) for v in outs}
        outs = outs | frozenset(fn << 4 | k for (fn, k) in OUT_EXTENTS if (fn << 4 | k) not in keyed)
        outs = outs | frozenset((fn << 4 | k) | (n << OUT_LEN_SHIFT)
                                for (fn, k), n in OUT_EXTENTS_OVL.get(ovl, {}).items()
                                if (fn << 4 | k) not in keyed)
        harness = HARNESS.replace("static unsigned seed_;", "static unsigned seed_;\nstatic const uint32_t outs_[] = {"
                                  + ", ".join(f"0x{out_key(v) & 0xFFFFFFFF:X}u" for v in sorted(outs) or [0xFFFFFFFF]) + "};\n"
                                  + "static const unsigned outl_[] = {"
                                  + ", ".join(f"{out_len(v)}u" for v in sorted(outs) or [0]) + "};")
        # E21_STACK_COINCIDE=1: put the native frame where retail's locals
        # are, so a body that reads its own uninitialized stack (the RotMatrix
        # TR residue class) reads the same seeded bytes on both sides.  The
        # lowered frame drops retail's 16-byte argument home area, so its
        # base is retail sp+16 = SP0 - N + 16 (N from the prologue's addiu).
        # Off by default.
        if os.environ.get("E21_STACK_COINCIDE") == "1":
            n_ret, n_nat = retail_frame_size(data, leaf), native_frame_size(t, name)
            if n_ret and n_nat:
                harness = (harness.replace("static pe_addr_t fsp = 0x801FEC00u;",
                                           f"static pe_addr_t fsp = 0x{SP0 - n_ret + 16 + ((n_nat + 7) & ~7):08X}u;")
                           .replace("a[i] < 0x801FF000u", "a[i] < 0x80200000u"))
        src = t + harness.replace("CALL_LINE", call) + stubs
        (d / "t.c").write_text(src)
        (d / "gte_ext.c").write_text(GTE_EXT)
        pc = o.REPO_ROOT / "pc_port"
        # The native GTE: pe_gte.c plus, when present, the command engine it
        # dispatches to (pe_gte_commands.c).
        gte_objs = []
        stubbed = set(re.findall(r"^\S[^\n]*?\b(func_[0-9A-F]{8})\(", stubs, re.M))
        for unit in ("pe_gte.c", "pe_gte_commands.c"):
            src_ = pc / "platform" / unit
            if not src_.exists():
                continue
            obj = d / (unit[:-2] + ".o")
            # a callee the TU mocks may also be defined by the native GTE unit
            # (func_80079004 in pe_gte.c): rename the unit's copy so the mock,
            # which logs the call as retail's jal does, is the one linked
            ren = [f"-D{n}=pe_harness_{n}" for n in sorted(stubbed)
                   if re.search(r"\b" + n + r"\s*\(", src_.read_text(errors="replace"))]
            r = subprocess.run([cc, *e21_cflags(gte=True), "-std=gnu11", "-funsigned-char", "-w", "-c", *ren,
                                "-I", str(pc / "include"), "-I", str(pc / "platform"), "-I", str(pc),
                                "-I", str(pc / "bootstrap"), "-o", str(obj), str(src_)],
                               capture_output=True, text=True)
            if r.returncode:
                return "nocompile", f"{unit}: " + r.stderr[:300]
            gte_objs.append(str(obj))
        r = subprocess.run([cc, *e21_cflags(), "-std=gnu11", "-funsigned-char", "-w", "-I", str(d),
                            "-o", str(d / "t"), str(d / "t.c"), str(d / "gte_ext.c"),
                            *gte_objs], capture_output=True, text=True)
        if r.returncode:
            return "nocompile", r.stderr[:300]
        ran = 0
        # E21_MIN_OK=K (e21_diff --min-ok): when fewer than K of the first
        # `seeds` seeds survive the oracle (a body that chases pointers
        # through its arguments and mock returns faults on most random RAM
        # images), go on with seeds `seeds`, `seeds`+1, ... up to
        # E21_MAX_SEEDS until K have run.  Off by default, so a run without
        # it is the same as before; the first `seeds` seeds are identical
        # either way.
        min_ok = int(os.environ.get("E21_MIN_OK", "0") or 0)
        max_seeds = int(os.environ.get("E21_MAX_SEEDS", "256") or 256)
        seed_ext = {"to": 0}

        def runs():
            yield from inputs(name, ov, data, seeds, recorded, dspec or None)
            if ran >= min_ok or recorded or SEED_LIST is not None or max_seeds <= seeds:
                return
            for item in inputs(name, ov, data, seeds, recorded, dspec or None,
                               seed_range=range(seeds, max_seeds)):
                if ran >= min_ok:
                    return
                seed_ext["to"] = item[0] + 1
                yield item

        for seed, ram, args, rets in runs():
            r_ram = bytearray(ram)
            try:
                gte_state = []
                v0, calls = run_mips(r_ram, leaf["vram"], list(args), seed, outs,
                                     gte_state if check_gte else None, ret_types=ret_types,
                                     ret_override=rets)
            except Fault as exc:
                if os.environ.get("E21_FAULTS"):   # why a seed was dropped
                    print(f"e21_diff: {name} seed {seed}: oracle fault: {exc}", file=sys.stderr)
                continue
            (d / "in.bin").write_bytes(ram)
            r = subprocess.run([str(d / "t"), str(d / "in.bin"), str(d / "out.bin"),
                                *[f"{a:08X}" for a in args], str(seed), str(d / "log"),
                                *[f"{a:08X}={narrow_ret(v, ret_types.get(a)):08X}"
                                  for a, v in rets.items()]],
                               capture_output=True, text=True, timeout=30)
            if r.returncode:
                return "MISMATCH", f"seed {seed}: native exit {r.returncode} {r.stdout[:80]}"
            n_ram = (d / "out.bin").read_bytes()
            lo, hi = EXCLUDE[0] & 0x1FFFFFFF, EXCLUDE[1] & 0x1FFFFFFF
            if r_ram[:lo] != n_ram[:lo]:
                i = next(i for i in range(lo) if r_ram[i] != n_ram[i])
                return "MISMATCH", (f"seed {seed}: RAM 0x{0x80000000 + i:08X} retail "
                                    f"{r_ram[i]:02X} native {n_ram[i]:02X}")
            if r_ram[RAM_SIZE:] != n_ram[RAM_SIZE:]:
                i = next(i for i in range(SPAD) if r_ram[RAM_SIZE + i] != n_ram[RAM_SIZE + i])
                return "MISMATCH", (f"seed {seed}: scratchpad 0x{0x1F800000 + i:08X} retail "
                                    f"{r_ram[RAM_SIZE + i]:02X} native {n_ram[RAM_SIZE + i]:02X}")
            log = (d / "log").read_text().split("\n")
            if check_gte:
                native_gte = [int(v, 16) for v in next(l for l in log if l.startswith("G ")).split()[1:]]
                if native_gte != gte_state:
                    i = next(i for i in range(64) if native_gte[i] != gte_state[i])
                    return "MISMATCH", (f"seed {seed}: GTE {'data' if i < 32 else 'ctrl'} {i % 32} "
                                        f"retail {gte_state[i]:08X} native {native_gte[i]:08X}")
            ncalls = [l for l in log if l.startswith("C ")]
            if len(ncalls) != len(calls):
                rt_ = [f"{c[0]:08X}" for c in calls]
                nt_ = [l.split()[1] for l in ncalls]
                k_ = next((i for i in range(min(len(rt_), len(nt_))) if rt_[i] != nt_[i].upper()), min(len(rt_), len(nt_)))
                return "MISMATCH", (f"seed {seed}: {len(calls)} retail calls vs {len(ncalls)} native "
                                    f"(first difference at call {k_}: retail {' '.join(rt_[k_:k_ + 4])} | "
                                    f"native {' '.join(nt_[k_:k_ + 4])})")
            for (tgt, regs), nl in zip(calls, ncalls):
                parts = nl.split()[1:]
                if int(parts[0], 16) != tgt:
                    return "MISMATCH", f"seed {seed}: call {tgt:08X} vs {parts[0]}"
                mk = masks.get(tgt, [])
                for k, pv in enumerate(parts[1:]):
                    m = mk[k] if k < len(mk) else 0xFFFFFFFF
                    rv_ = regs[k]
                    if isinstance(rv_, tuple) and not pv.startswith("S:"):
                        rv_ = rv_[3]           # a guest pointer that merely lies in the stack window
                    if pv.startswith("S:") and not isinstance(rv_, tuple):
                        pv = pv.split(":")[1]  # likewise on the native side
                    if isinstance(rv_, tuple) or pv.startswith("S:"):
                        nb = bytes.fromhex(pv.split(":")[2]) if pv.startswith("S:") else b""
                        # Matrix consumers read translations at +20/+24/+28;
                        # the old 16-byte probe could miss misplaced meshes.
                        extent = STACK_EXTENTS_OVL.get(ovl, {}).get((tgt, k), STACK_EXTENTS.get((tgt, k), 16))
                        if not (isinstance(rv_, tuple) and len(nb) == 32 and
                                all(nb[j] == rv_[1][j] for j in range(extent) if rv_[2][j])):
                            return "MISMATCH", (f"seed {seed}: call {tgt:08X} a{k} stack arg "
                                                f"{('@%08X:' % rv_[3]) + rv_[1].hex() if isinstance(rv_, tuple) else hex(rv_)} vs {pv} "
                                                f"(retail-written {''.join('w' if x else '.' for x in rv_[2]) if isinstance(rv_, tuple) else '-'})")
                        continue
                    if int(pv, 16) & m != rv_ & m:
                        return "MISMATCH", f"seed {seed}: call {tgt:08X} a{k} {rv_:08X} vs {pv}"
            rl = [l for l in log if l.startswith("R ")][0].split()[1]
            if rl != "-":
                nv = int(rl, 16)
                rty = sig.group(1)
                if rty.startswith(("uint8", "int8", "uint16", "int16")):
                    # A narrow return type is checked, not assumed: GCC returns
                    # the value zero-/sign-extended (andi, sll+sra, lhu, lb), so
                    # retail's v0 must already be its own extension; anything
                    # else means the declared type is too narrow.  The value is
                    # then compared at that width (the native log is extended).
                    ext = narrow_ret(v0, rty.replace("_t", "").replace("uint", "unsigned ")
                                     .replace("int8", "signed char").replace("int16", "short")
                                     .replace("unsigned 8", "unsigned char").replace("unsigned 16", "unsigned short"))
                    if ext != v0:
                        return "MISMATCH", (f"seed {seed}: retail v0 {v0:08X} is not a {rty} "
                                            f"(the declared return type is too narrow)")
                    if nv != ext:
                        return "MISMATCH", f"seed {seed}: v0 {v0:08X} vs {rl}"
                elif nv != v0:
                    return "MISMATCH", f"seed {seed}: v0 {v0:08X} vs {rl}"
            ran += 1
        cov = ""
        if COVER is not None:
            lo_, hi_ = leaf["vram"], leaf["vram"] + leaf["file_size"]
            hit = sum(1 for a in COVER if lo_ <= a < hi_)
            cov = f", retail coverage {hit}/{leaf['file_size'] // 4} words"
        more = f" (seed range extended to {seed_ext['to']})" if seed_ext["to"] else ""
        return ("ok" if ran else "skipped"), f"{ran} seeds{more}{cov}"


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--seeds", type=int, default=6)
    ap.add_argument("--limit", type=int, default=0)
    ap.add_argument("--verbose", action="store_true")
    ap.add_argument("--jobs", type=int, default=4)
    ap.add_argument("--recorded", default="", help="replay PE_RECORD_GUESTCALL snapshots from DIR")
    ap.add_argument("--coverage", action="store_true", help="report retail instruction coverage")
    ap.add_argument("--check-gte", action="store_true", help="also compare final GTE data/control registers")
    ap.add_argument("--with-calls", action="store_true",
                    help="also test matched TUs that make calls (callees mocked as for NON_MATCHING)")
    ap.add_argument("--only", default="",
                    help="substring of the TU file name or source overlay (comma-separated: any of them)")
    ap.add_argument("--min-ok", type=int, default=0,
                    help="extend the seed range of a TU until this many seeds survive the oracle (E21_MIN_OK)")
    ap.add_argument("--max-seeds", type=int, default=256,
                    help="last seed + 1 the --min-ok extension may use (E21_MAX_SEEDS)")
    args = ap.parse_args()
    if args.min_ok:
        os.environ["E21_MIN_OK"] = str(args.min_ok)
        os.environ["E21_MAX_SEEDS"] = str(args.max_seeds)
    cc = shutil.which("cc") or shutil.which("gcc")
    cands = candidates(args.with_calls)
    if args.only:
        only = [s for s in args.only.split(",") if s]
        cands = [c for c in cands if any(s in c[0].name or s == c[2] for s in only)]
    if args.limit:
        cands = cands[: args.limit]
    res: dict[str, list] = {}
    with concurrent.futures.ProcessPoolExecutor(max_workers=max(1, args.jobs)) as ex:
        futs = {ex.submit(one, p, n, ov, args.seeds, cc, args.coverage, args.recorded, directed_specs(), args.check_gte): (p.name, ov) for p, n, ov in cands}
        for f in concurrent.futures.as_completed(futs):
            try:
                st, why = f.result()
            except Exception as exc:          # one TU's harness failure must not end the sweep
                st, why = "error", repr(exc)[:300]
            res.setdefault(st, []).append((futs[f][0], why))
    for st in sorted(res):
        print(f"e21_diff: {st}: {len(res[st])}")
        if st in ("MISMATCH", "nocompile", "error") or args.verbose:
            for nm, why in sorted(res[st]):
                print(f"  {st} {nm}: {why}")
    return 1 if res.get("MISMATCH") or res.get("nocompile") or res.get("error") else 0


if __name__ == "__main__":
    raise SystemExit(main())
