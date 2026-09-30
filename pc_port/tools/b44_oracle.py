#!/usr/bin/env python3
"""Independent delay-slot-aware MIPS-I oracle for func_8005B91C.

The interpreter executes all 87 literal retail words, models only the already
translated func_8005DB8C leaf, and never loads or calls production C.
"""
import hashlib
import struct
import sys
from pe_exe_words import exe_words

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
BASE = 0x8005B91C
END = 0x8005BA78
RAM_BASE = 0x80000000
RAM_END = 0x80200000
STACK = 0x801FFF00
RETURN = 0xDEADC0DE

W = exe_words(0x8005B91C, 87)

# Three words before each jal, the jal, its delay slot, and three following.
CALLERS = {
    0x80043AB8: tuple(exe_words(0x80043AB8, 8)),
    0x80043CF0: tuple(exe_words(0x80043CF0, 8)),
    0x80048CF4: tuple(exe_words(0x80048CF4, 8)),
    0x8004A93C: tuple(exe_words(0x8004A93C, 8)),
    0x8004B844: tuple(exe_words(0x8004B844, 8)),
    0x8004B89C: tuple(exe_words(0x8004B89C, 8)),
    0x8004BD98: tuple(exe_words(0x8004BD98, 8)),
    0x8004BDC4: tuple(exe_words(0x8004BDC4, 8)),
    0x80050D88: tuple(exe_words(0x80050D88, 8)),
    0x80050EB8: tuple(exe_words(0x80050EB8, 8)),
    0x800521A4: tuple(exe_words(0x800521A4, 8)),
    0x80052274: tuple(exe_words(0x80052274, 8)),
    0x800522A8: tuple(exe_words(0x800522A8, 8)),
    0x800522DC: tuple(exe_words(0x800522DC, 8)),
    0x80052324: tuple(exe_words(0x80052324, 8)),
    0x80052364: tuple(exe_words(0x80052364, 8)),
    0x80052398: tuple(exe_words(0x80052398, 8)),
    0x80052414: tuple(exe_words(0x80052414, 8)),
}


def u32(value):
    return value & 0xFFFFFFFF


def s32(value):
    value = u32(value)
    return value - 0x100000000 if value & 0x80000000 else value


def trunc_div(left, right):
    if right == 0:
        raise ZeroDivisionError
    if left == -0x80000000 and right == -1:
        raise OverflowError
    quotient = abs(left) // abs(right)
    return -quotient if (left < 0) != (right < 0) else quotient


class Oracle:
    def __init__(self, executable):
        self.data = open(executable, "rb").read()
        digest = hashlib.sha1(self.data).hexdigest()
        if digest != SHA1:
            raise SystemExit(f"FATAL: executable SHA-1 {digest} != {SHA1}")
        self.taddr = struct.unpack_from("<I", self.data, 0x18)[0]
        self.verify_words(BASE, W, "func_8005B91C")
        for address, words in CALLERS.items():
            self.verify_words(address, words, f"caller {address:08X}")
        self.verify_census()
        self.ram = bytearray(RAM_END - RAM_BASE)
        self.r = [0] * 32
        self.r[29] = STACK
        self.r[31] = RETURN
        self.hi = self.lo = 0
        self.events = []
        self.context = "func_8005B91C"
        self.control_issues = self.delay_executions = 0
        self.out_addresses = set()

    def verify_words(self, address, words, label):
        for i, want in enumerate(words):
            offset = address + i * 4 - self.taddr + 0x800
            got = struct.unpack_from("<I", self.data, offset)[0]
            if got != want:
                raise SystemExit(
                    f"FATAL: {label} @{address+i*4:08X}: {got:08X}!={want:08X}")

    def verify_census(self):
        hits = []
        for offset in range(0x800, len(self.data) - 3, 4):
            if struct.unpack_from("<I", self.data, offset)[0] == 0x0C016E47:
                hits.append(self.taddr + offset - 0x800)
        want = [address + 12 for address in CALLERS]
        if hits != want:
            raise SystemExit(f"FATAL: func_8005B91C jal census {hits} != {want}")

    def off(self, address, width):
        address = u32(address)
        if address < RAM_BASE or width < 0 or address > RAM_END - width:
            raise SystemExit(f"FATAL: guest access {address:08X}/{width}")
        return address - RAM_BASE

    def poke(self, address, width, value):
        offset = self.off(address, width)
        self.ram[offset:offset + width] = u32(value).to_bytes(4, "little")[:width]

    def peek(self, address, width):
        offset = self.off(address, width)
        return int.from_bytes(self.ram[offset:offset + width], "little")

    def load(self, address, width, signed=False, opcode="load"):
        address = u32(address)
        value = self.peek(address, width)
        if signed and value & (1 << (width * 8 - 1)):
            value -= 1 << width * 8
        value = u32(value)
        self.events.append(("read", self.context, opcode, address, width, value))
        return value

    def store(self, address, width, value, opcode="store"):
        address = u32(address)
        self.poke(address, width, value)
        self.events.append(("write", self.context, opcode, address, width,
                            value & ((1 << width * 8) - 1)))

    def one(self, word, address):
        op = word >> 26
        rs, rt, rd = (word >> 21) & 31, (word >> 16) & 31, (word >> 11) & 31
        shift, fn = (word >> 6) & 31, word & 63
        imm = word & 0xFFFF
        simm = imm if imm < 0x8000 else imm - 0x10000
        r = self.r
        if word == 0:
            return
        if op == 0:
            if fn == 0x00:
                r[rd] = u32(r[rt] << shift)
            elif fn == 0x03:
                r[rd] = u32(s32(r[rt]) >> shift)
            elif fn in (0x08, 0x0D):
                return
            elif fn == 0x12:
                r[rd] = self.lo
            elif fn == 0x1A:
                left, right = s32(r[rs]), s32(r[rt])
                if right != 0 and not (left == -0x80000000 and right == -1):
                    self.lo = u32(trunc_div(left, right))
                    self.hi = u32(left - trunc_div(left, right) * right)
            elif fn == 0x21:
                r[rd] = u32(r[rs] + r[rt])
            elif fn == 0x23:
                r[rd] = u32(r[rs] - r[rt])
            elif fn == 0x2A:
                r[rd] = int(s32(r[rs]) < s32(r[rt]))
            else:
                raise SystemExit(f"FATAL: SPECIAL {fn:02X} @{address:08X}")
        elif op == 0x09:
            r[rt] = u32(r[rs] + simm)
        elif op == 0x0A:
            r[rt] = int(s32(r[rs]) < simm)
        elif op == 0x0F:
            r[rt] = u32(imm << 16)
        elif op == 0x23:
            r[rt] = self.load(u32(r[rs] + simm), 4, False, "lw")
        elif op == 0x2B:
            self.store(u32(r[rs] + simm), 4, r[rt], "sw")
        else:
            raise SystemExit(f"FATAL: opcode {op:02X} @{address:08X}")
        r[0] = 0

    def db8c(self):
        args = (u32(self.r[4]),)
        self.events.append(("call", "func_8005DB8C", args))
        old = self.context
        self.context = "func_8005DB8C translated"
        base_value = self.load(0x800A8038, 4, False, "lw")
        self.context = old
        self.r[2] = u32(base_value + 0x800A8028 + (args[0] << 9))
        self.events.append(("return", "func_8005DB8C", self.r[2]))

    def run(self, table_number, key, index_out, fraction_out):
        self.out_addresses = {address for address in (index_out, fraction_out)
                              if address}
        self.r[4:8] = [u32(table_number), u32(key), u32(index_out),
                       u32(fraction_out)]
        self.events.append(("output_before", index_out,
                            self.peek(index_out, 4) if index_out else None,
                            fraction_out,
                            self.peek(fraction_out, 4) if fraction_out else None))
        pc = BASE
        for _ in range(4096):
            if pc == RETURN:
                self.events.append(("output_after", index_out,
                                    self.peek(index_out, 4) if index_out else None,
                                    fraction_out,
                                    self.peek(fraction_out, 4) if fraction_out else None))
                self.events.append(("return", "func_8005B91C", u32(self.r[2])))
                return
            if pc < BASE or pc >= END or (pc - BASE) & 3:
                raise SystemExit(f"FATAL: bad PC {pc:08X}")
            word = W[(pc - BASE) // 4]
            op, rs, rt = word >> 26, (word >> 21) & 31, (word >> 16) & 31
            imm = word & 0xFFFF
            simm = imm if imm < 0x8000 else imm - 0x10000
            if op in (2, 3):
                target = u32(((pc + 4) & 0xF0000000) |
                             ((word & 0x03FFFFFF) << 2))
                self.control_issues += 1
                if op == 3:
                    self.r[31] = u32(pc + 8)
                self.one(W[(pc + 4 - BASE) // 4], pc + 4)
                self.delay_executions += 1
                if op == 3:
                    if target != 0x8005DB8C:
                        raise SystemExit(f"FATAL: unexpected jal {target:08X}")
                    self.db8c()
                    pc = u32(pc + 8)
                else:
                    pc = target
                continue
            if op in (4, 5):
                equal = u32(self.r[rs]) == u32(self.r[rt])
                taken = equal if op == 4 else not equal
                target = u32(pc + 4 + simm * 4)
                self.control_issues += 1
                self.one(W[(pc + 4 - BASE) // 4], pc + 4)
                self.delay_executions += 1
                pc = target if taken else u32(pc + 8)
                continue
            if op == 0 and (word & 63) == 0x08:
                target = u32(self.r[rs])
                self.control_issues += 1
                self.one(W[(pc + 4 - BASE) // 4], pc + 4)
                self.delay_executions += 1
                pc = target
                continue
            if op == 0 and (word & 63) == 0x0D:
                raise ArithmeticError(f"MIPS break {(word >> 6) & 0xFFFFF}")
            self.one(word, pc)
            pc = u32(pc + 4)
        raise SystemExit("FATAL: step limit")


def seed_linear(oracle, base=0x800B0000):
    oracle.poke(0x800A8038, 4, base - 0x800A8028)
    for table in range(7):
        table_base = base + table * 0x200
        oracle.poke(table_base - 4, 4, -10)
        for index in range(129):
            oracle.poke(table_base + index * 4, 4, index * 10)
    return base


def outputs(oracle):
    return [event for event in oracle.events
            if event[0] == "write" and event[1] == "func_8005B91C"
            and event[3] in oracle.out_addresses]


def test_interpreter_and_paths(executable):
    cases = [
        (-1, 0, -4), (0, 0, 0), (5, 0, 24), (10, 1, 0),
        (15, 1, 24), (2000, 98, 48),
    ]
    for key, want_index, want_fraction in cases:
        oracle = Oracle(executable)
        seed_linear(oracle)
        oracle.run(0, key, 0x80001000, 0x80001004)
        assert s32(oracle.peek(0x80001000, 4)) == want_index
        assert s32(oracle.peek(0x80001004, 4)) == want_fraction
        assert oracle.control_issues == oracle.delay_executions
        assert [event[4] for event in outputs(oracle)] == [4, 4]

    # Null pointers do not suppress the table reads/interpolation arithmetic.
    null = Oracle(executable)
    seed_linear(null)
    null.run(0, 5, 0, 0)
    assert not outputs(null)
    assert any(event[0] == "read" and event[3] == 0x800B0004
               for event in null.events)

    # When outputs alias, retail's fraction store follows and wins.
    alias = Oracle(executable)
    seed_linear(alias)
    alias.run(0, 15, 0x80001000, 0x80001000)
    writes = outputs(alias)
    assert [event[5] for event in writes] == [1, 24]
    assert alias.peek(0x80001000, 4) == 24


def test_b43_seven_contexts(executable):
    offsets = [0, 3, 5, 7, 11, 13, 0]
    dbac_inputs = []
    for table in range(7):
        oracle = Oracle(executable)
        seed_linear(oracle)
        out = 0x80001000
        oracle.run(table, table * 10 + 5, out, 0)
        index = s32(oracle.peek(out, 4))
        assert index == table
        dbac_inputs.append(u32(index + offsets[table]))
    assert dbac_inputs == [0, 4, 7, 10, 15, 18, 6]

    # Exact real Disc 1 first state: table 0 begins 0,10,... and T[0]==0.
    real = Oracle(executable)
    seed_linear(real, 0x800AD5E8)
    real.run(0, 0, 0x80001000, 0)
    assert real.peek(0x80001000, 4) == 0


def main():
    executable = (sys.argv[1] if len(sys.argv) > 1 else
                  "build/extracted/disc1/SLUS_006.62")
    assert len(W) == 87 and END - BASE == len(W) * 4
    test_interpreter_and_paths(executable)
    test_b43_seven_contexts(executable)
    print("B44 func_8005B91C oracle: PASS")
    print("  87/87 body words; 18/18 executable call sites")
    print("  delay slots, signed search/division, exact output write order")
    print("  seven B43 output-to-func_8005DBAC contexts verified")


if __name__ == "__main__":
    main()
