#!/usr/bin/env python3
"""Independent Phase 6E-B45 oracle for func_80087090.

The 20 words below are an independent transcription of the SPU upload
retry wrapper at 0x80087090.  They are checked against the SHA-verified
executable before this delay-slot-aware interpreter executes the
transcription.  Production C is never called.

func_80087090(a0=buffer, a1=transfer_count):
  Retries func_800851A8(a0, a1) while return == 1.
  Returns last v0 from func_800851A8.
"""
import hashlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
BASE = 0x80087090
RAM_BASE = 0x80000000
RAM_END = 0x80200000

# func_80087090: SPU upload retry wrapper.
# 20 words / 0x50 bytes.  Two call sites:
#   func_8006A9E4 @0x8006AC28 (a0=stream_buf, a1=1, return ignored)
#   func_8006CC68 @0x8006CF98 (a0=$s5, a1=0, return consumed)
#
# Words transcribed from asm/disc1/75F44.s:2103-2123 (hex in comments
# is little-endian file order; converted to native u32 below).
W = [
    0x27BDFFE0,  # addiu $sp, $sp, -0x20
    0xAFB00010,  # sw    $s0, 0x10($sp)
    0x00808021,  # addu  $s0, $a0, $zero
    0xAFB10014,  # sw    $s1, 0x14($sp)
    0x00A08821,  # addu  $s1, $a1, $zero
    0xAFB20018,  # sw    $s2, 0x18($sp)
    0x24120001,  # addiu $s2, $zero, 0x1
    0xAFBF001C,  # sw    $ra, 0x1C($sp)
    0x02002021,  # addu  $a0, $s0, $zero
    # .L800870B4:
    0x0C02146A,  # jal   func_800851A8
    0x02202821,  # addu  $a1, $s1, $zero  (delay slot)
    0x1052FFFD,  # beq   $v0, $s2, .L800870B4
    0x02002021,  # addu  $a0, $s0, $zero  (delay slot)
    0x8FBF001C,  # lw    $ra, 0x1C($sp)
    0x8FB20018,  # lw    $s2, 0x18($sp)
    0x8FB10014,  # lw    $s1, 0x14($sp)
    0x8FB00010,  # lw    $s0, 0x10($sp)
    0x27BD0020,  # addiu $sp, $sp, 0x20
    0x03E00008,  # jr    $ra
    0x00000000,  # nop   (delay slot)
]


def u32(x): return x & 0xFFFFFFFF
def s32(x):
    x = u32(x)
    return x - 0x100000000 if x & 0x80000000 else x


class Oracle:
    def __init__(self, exe):
        data = open(exe, "rb").read()
        got = hashlib.sha1(data).hexdigest()
        if got != SHA1:
            raise SystemExit(f"FATAL: executable SHA-1 {got} != {SHA1}")
        taddr = struct.unpack_from("<I", data, 0x18)[0]
        for i, want in enumerate(W):
            got_w = struct.unpack_from("<I", data, BASE + i * 4 - taddr + 0x800)[0]
            if got_w != want:
                raise SystemExit(f"FATAL: word {i} @ {BASE+i*4:08X}: "
                                 f"{got_w:08X} != {want:08X}")
        print(f"exe SHA-1 {SHA1} OK; func_80087090: {len(W)} words verified")
        self.ram = bytearray(0x200000)
        self.r = [0] * 32
        self.pc = 0
        self.steps = 0
        self.reads = []
        self.writes = []
        self.calls = []
        self.returns = []

    def off(self, a, n):
        if not (RAM_BASE <= a and a + n <= RAM_END):
            raise SystemExit(f"FATAL: guest access {a:08X}/{n}")
        return a - RAM_BASE

    def store(self, a, n, v):
        o = self.off(a, n)
        v = u32(v) & ((1 << (8 * n)) - 1)
        self.ram[o:o+n] = v.to_bytes(n, "little")
        self.writes.append((a, n, v))

    def load(self, a, n):
        o = self.off(a, n)
        return int.from_bytes(self.ram[o:o+n], "little")

    def one(self, w, addr):
        """Execute a single non-branch/jump instruction."""
        op = (w >> 26) & 63
        rs, rt, rd = (w >> 21) & 31, (w >> 16) & 31, (w >> 11) & 31
        sa, fn = (w >> 6) & 31, w & 63
        imm = w & 0xFFFF
        simm = imm if imm < 0x8000 else imm - 0x10000
        r = self.r
        if w == 0:
            return
        if op == 0:
            if fn == 0: r[rd] = u32(r[rt] << sa)
            elif fn == 3: r[rd] = u32(s32(r[rt]) >> sa)
            elif fn == 8: raise SystemExit("FATAL: unexpected jr in one()")
            elif fn == 0x21: r[rd] = u32(r[rs] + r[rt])
            elif fn == 0x25: r[rd] = u32(r[rs] | r[rt])
            elif fn == 0x24: r[rd] = u32(r[rs] & r[rt])
            else: raise SystemExit(f"FATAL: SPECIAL {fn:02X} @{addr:08X}")
        elif op == 9: r[rt] = u32(r[rs] + simm)
        elif op == 0x0F: r[rt] = u32(imm << 16)
        elif op == 0x23:  # lw
            a = u32(r[rs] + simm)
            r[rt] = self.load(a, 4)
            self.reads.append((a, 4, r[rt]))
        elif op == 0x2B:  # sw
            a = u32(r[rs] + simm)
            self.store(a, 4, r[rt])
        else: raise SystemExit(f"FATAL: opcode {op:02X} @{addr:08X}")
        r[0] = 0

    def run(self, a0, a1, mock_returns=None):
        """Execute func_80087090(a0, a1).

        mock_returns: list of v0 values for successive func_800851A8 calls.
                      If None, defaults to [0] (single success).
        """
        if mock_returns is None:
            mock_returns = [0]
        self.r[4] = u32(a0)
        self.r[5] = u32(a1)
        self.r[29] = 0x801FFF00  # $sp
        call_idx = 0

        while True:
            if self.steps > 200:
                raise SystemExit("FATAL: oracle did not terminate")
            self.steps += 1
            addr = BASE + self.pc * 4
            w = W[self.pc]
            op = (w >> 26) & 63
            rs = (w >> 21) & 31
            rt = (w >> 16) & 31
            imm = w & 0xFFFF
            simm = imm if imm < 0x8000 else imm - 0x10000
            fn = w & 63

            if w == 0:
                self.pc += 1
                continue

            # beq: evaluate condition, execute delay slot, then branch or advance
            if op == 4:
                delay = W[self.pc + 1]
                take = (self.r[rs] == self.r[rt])
                self.one(delay, addr + 4)
                if take:
                    self.pc = (addr + 4 + (simm << 2) - BASE) // 4
                else:
                    self.pc += 2
                continue

            # jal: call func_800851A8
            if op == 3:
                target = (addr & 0xF0000000) | ((w & 0x03FFFFFF) << 2)
                if target != 0x800851A8:
                    raise SystemExit(f"FATAL: unexpected call target {target:08X}")
                delay = W[self.pc + 1]
                self.one(delay, addr + 4)
                # Record the call with exact arguments
                buf_arg = self.r[4]
                cnt_arg = self.r[5]
                self.calls.append(("func_800851A8", buf_arg, cnt_arg))
                # Return mock value
                if call_idx < len(mock_returns):
                    ret = mock_returns[call_idx]
                else:
                    ret = mock_returns[-1]  # repeat last value
                call_idx += 1
                self.r[2] = u32(ret)
                self.returns.append(("func_800851A8", u32(ret)))
                # Skip past delay slot (already executed)
                self.pc = ((addr + 8) - BASE) // 4
                continue

            # jr $ra: return
            if op == 0 and fn == 8:
                delay = W[self.pc + 1]
                self.one(delay, addr + 4)
                return

            self.one(w, addr)
            self.pc += 1


def main():
    if len(sys.argv) != 2:
        raise SystemExit("usage: b45_oracle.py executable")

    # Test 1: single success (func_800851A8 returns 0 immediately)
    o = Oracle(sys.argv[1])
    o.run(0x80100000, 1)
    assert len(o.calls) == 1, f"expected 1 call, got {len(o.calls)}"
    assert o.calls[0] == ("func_800851A8", 0x80100000, 1), f"call: {o.calls[0]}"
    assert o.r[2] == 0, f"return: {o.r[2]}"
    print("  S1: single success (ret=0) OK")

    # Test 2: one retry then success (returns 1, then 0)
    o = Oracle(sys.argv[1])
    o.run(0x80100000, 1, [1, 0])
    assert len(o.calls) == 2, f"expected 2 calls, got {len(o.calls)}"
    assert o.calls[0] == ("func_800851A8", 0x80100000, 1)
    assert o.calls[1] == ("func_800851A8", 0x80100000, 1)
    assert o.r[2] == 0
    print("  S2: one retry then success OK")

    # Test 3: three retries then success
    o = Oracle(sys.argv[1])
    o.run(0x80100000, 1, [1, 1, 1, 0])
    assert len(o.calls) == 4
    for i in range(4):
        assert o.calls[i] == ("func_800851A8", 0x80100000, 1)
    assert o.r[2] == 0
    print("  S3: three retries then success OK")

    # Test 4: negative return (error, no retry)
    o = Oracle(sys.argv[1])
    o.run(0x80100000, 1, [-1])
    assert len(o.calls) == 1
    assert o.r[2] == 0xFFFFFFFF, f"return: {o.r[2]:08X}"
    print("  S4: negative return (error) OK")

    # Test 5: a1=0 (second caller pattern from func_8006CC68)
    o = Oracle(sys.argv[1])
    o.run(0x80200000, 0)
    assert len(o.calls) == 1
    assert o.calls[0] == ("func_800851A8", 0x80200000, 0)
    print("  S5: a1=0 (6CC68 caller pattern) OK")

    # Test 6: retry preserves arguments exactly
    o = Oracle(sys.argv[1])
    o.run(0x801ED800, 1, [1, 1, 0])
    for i in range(3):
        assert o.calls[i][1] == 0x801ED800, f"call {i}: buffer corrupted"
        assert o.calls[i][2] == 1, f"call {i}: count corrupted"
    print("  S6: arguments preserved across retries OK")

    # Test 7: return value 2 does NOT trigger retry
    o = Oracle(sys.argv[1])
    o.run(0x80100000, 1, [2])
    assert len(o.calls) == 1
    assert o.r[2] == 2
    print("  S7: return=2 no retry OK")

    # Test 8: verify exact write footprint (only stack)
    o = Oracle(sys.argv[1])
    o.run(0x80100000, 1, [0])
    guest_writes = [w for w in o.writes if w[0] >= RAM_BASE and w[0] < RAM_END
                    and not (0x801FFF00 - 0x20 <= w[0] < 0x801FFF00)]
    assert len(guest_writes) == 0, f"unexpected guest writes: {guest_writes}"
    print("  S8: no guest-memory writes (stack only) OK")

    # Test 9: verify exact stack writes (prologue stores only)
    stack_writes = [w for w in o.writes
                    if 0x801FFF00 - 0x20 <= w[0] < 0x801FFF00]
    # Prologue: sw s0/s1/s2/ra at sp+0x10..0x1C → 4 stores
    # Epilogue: lw ra/s2/s1/s0 → reads, not writes
    assert len(stack_writes) == 4, f"expected 4 stack writes, got {len(stack_writes)}"
    print("  S9: exact stack save/restore footprint OK")

    # Test 10: verify delay slot semantics
    # In the retry loop, the delay slot of jal sets a1=count,
    # and the delay slot of beq sets a0=buffer for next iteration.
    o = Oracle(sys.argv[1])
    o.run(0xABCD0000, 42, [1, 0])
    assert o.calls[0] == ("func_800851A8", 0xABCD0000, 42)
    assert o.calls[1] == ("func_800851A8", 0xABCD0000, 42)
    print("  S10: delay-slot argument passing OK")

    print("PASS: B45 independent transcription, 20 words, delay slots, "
          "retry loop, argument preservation, footprint")


if __name__ == "__main__": main()
