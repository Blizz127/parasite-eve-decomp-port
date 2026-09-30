#!/usr/bin/env python3
"""Independent Phase 6E-B43 MIPS-I oracle for func_8005218C.

All 155 literal retail words and the five executable caller contexts are
checked against the SHA-1-exact executable before this delay-slot-aware
interpreter executes the body.  func_8005DBAC is modeled exactly;
func_8005B91C and func_80052F24 use explicit controlled boundary contracts.
Production C is never loaded or called.
"""
import hashlib
import struct
import sys
from pe_exe_words import exe_words

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
BASE = 0x8005218C
END = 0x800523F8
GP = 0x8009CD70
RAM_BASE = 0x80000000
RAM_END = 0x80200000
STACK = 0x801FFF00
RETURN = 0xDEADC0DE
MAX_STEPS = 4096

# func_8005218C, executable 0x8005218C..0x800523F7, file 0x4298C.
W = exe_words(0x8005218C, 155)

# Three words before each jal, its delay slot, and four following words.
CALLERS = {
    0x8002F794: exe_words(0x8002F794, 9),
    0x800490E8: exe_words(0x800490E8, 9),
    0x8004C2C4: exe_words(0x8004C2C4, 9),
    0x80051DC0: exe_words(0x80051DC0, 9),
    0x80052478: exe_words(0x80052478, 9),
}


def u32(value):
    return value & 0xFFFFFFFF


def s32(value):
    value = u32(value)
    return value - 0x100000000 if value & 0x80000000 else value


class Oracle:
    def __init__(self, executable):
        data = open(executable, "rb").read()
        digest = hashlib.sha1(data).hexdigest()
        if digest != SHA1:
            raise SystemExit(f"FATAL: executable SHA-1 {digest} != {SHA1}")
        self.data = data
        self.taddr = struct.unpack_from("<I", data, 0x18)[0]
        self.verify_words(BASE, W, "func_8005218C")
        for base, words in CALLERS.items():
            self.verify_words(base, words, f"caller context {base:08X}")
        self.verify_census()

        self.ram = bytearray(RAM_END - RAM_BASE)
        self.r = [0] * 32
        self.r[28] = GP
        self.r[29] = STACK
        self.r[31] = RETURN
        self.hi = 0
        self.lo = 0
        self.events = []
        self.context = "func_8005218C"
        self.control_issues = 0
        self.delay_executions = 0
        self.outputs_5b91c = [0] * 7
        self.returns_5b91c = [0] * 7
        self.return_52f24 = 0
        self.effects_52f24 = []
        self.call_index_5b91c = 0

    def verify_words(self, base, words, label):
        for index, want in enumerate(words):
            address = base + index * 4
            offset = address - self.taddr + 0x800
            got = struct.unpack_from("<I", self.data, offset)[0]
            if got != want:
                raise SystemExit(
                    f"FATAL: {label} word {index} @ {address:08X}: "
                    f"{got:08X} != {want:08X}")

    def verify_census(self):
        word = 0x0C014863
        hits = []
        for offset in range(0x800, len(self.data) - 3, 4):
            if struct.unpack_from("<I", self.data, offset)[0] == word:
                hits.append(self.taddr + offset - 0x800)
        want = [base + 12 for base in CALLERS]
        if hits != want:
            raise SystemExit(f"FATAL: func_8005218C jal census {hits}")

    def off(self, address, width):
        address = u32(address)
        if width < 0 or address < RAM_BASE or address > RAM_END - width:
            raise SystemExit(f"FATAL: guest access {address:08X}/{width}")
        return address - RAM_BASE

    def poke(self, address, width, value):
        offset = self.off(address, width)
        mask = (1 << (width * 8)) - 1
        self.ram[offset:offset + width] = (value & mask).to_bytes(
            width, "little")

    def peek(self, address, width):
        offset = self.off(address, width)
        return int.from_bytes(self.ram[offset:offset + width], "little")

    def load(self, address, width, signed=False, opcode="load"):
        address = u32(address)
        value = self.peek(address, width)
        if signed and value & (1 << (width * 8 - 1)):
            value -= 1 << (width * 8)
        value = u32(value)
        self.events.append(("read", self.context, opcode,
                            address, width, value))
        return value

    def store(self, address, width, value, opcode="store"):
        address = u32(address)
        self.poke(address, width, value)
        mask = (1 << (width * 8)) - 1
        self.events.append(("write", self.context, opcode,
                            address, width, value & mask))

    def snapshot(self):
        return (
            self.peek(STACK - 0x20, 4),
            self.peek(0x800C0E06, 2),
            self.peek(0x800C0E08, 2),
            self.peek(0x800C0E0A, 1),
            self.peek(0x8009D254, 4),
        )

    def boundary(self, when, symbol, args):
        self.events.append((f"boundary_{when}", symbol, tuple(args),
                            self.snapshot()))

    def dep_5b91c(self):
        i = self.call_index_5b91c
        if i >= len(self.outputs_5b91c):
            raise SystemExit("FATAL: too many func_8005B91C calls")
        args = tuple(u32(self.r[n]) for n in range(4, 8))
        self.boundary("before", "func_8005B91C", args)
        self.events.append(("call", "func_8005B91C", args))
        old = self.context
        self.context = "func_8005B91C controlled"
        self.store(args[2], 4, self.outputs_5b91c[i], "sw-a2-output")
        self.context = old
        self.r[2] = u32(self.returns_5b91c[i])
        self.events.append(("return", "func_8005B91C", self.r[2]))
        self.boundary("after", "func_8005B91C", args)
        self.call_index_5b91c += 1

    def dep_5dbac(self):
        arg = u32(self.r[4])
        args = (arg,)
        self.boundary("before", "func_8005DBAC", args)
        self.events.append(("call", "func_8005DBAC", args))
        clamped = s32(arg)
        if clamped < 0:
            clamped = 0
        elif clamped >= 99:
            clamped = 98
        old = self.context
        self.context = "func_8005DBAC translated"
        base = self.load(0x800A803C, 4, False, "lw")
        self.context = old
        self.r[2] = u32(base + 0x800A8028 + clamped * 24)
        self.events.append(("return", "func_8005DBAC", self.r[2]))
        self.boundary("after", "func_8005DBAC", args)

    def dep_52f24(self):
        args = (u32(self.r[4]),)
        self.boundary("before", "func_80052F24", args)
        self.events.append(("call", "func_80052F24", args))
        old = self.context
        self.context = "func_80052F24 controlled"
        for address, width, value in self.effects_52f24:
            self.store(address, width, value, "controlled-effect")
        self.context = old
        self.r[2] = u32(self.return_52f24)
        self.events.append(("return", "func_80052F24", self.r[2]))
        self.boundary("after", "func_80052F24", args)

    def one(self, word, address):
        op = (word >> 26) & 0x3F
        rs = (word >> 21) & 0x1F
        rt = (word >> 16) & 0x1F
        rd = (word >> 11) & 0x1F
        shift = (word >> 6) & 0x1F
        function = word & 0x3F
        immediate = word & 0xFFFF
        simm = immediate if immediate < 0x8000 else immediate - 0x10000
        r = self.r
        if word == 0:
            return
        if op == 0:
            if function == 0x00:       # sll
                r[rd] = u32(r[rt] << shift)
            elif function == 0x03:     # sra
                r[rd] = u32(s32(r[rt]) >> shift)
            elif function == 0x08:     # jr, handled by run
                return
            elif function == 0x10:     # mfhi
                r[rd] = self.hi
            elif function == 0x12:     # mflo
                r[rd] = self.lo
            elif function == 0x18:     # mult (signed)
                product = (s32(r[rs]) * s32(r[rt])) & 0xFFFFFFFFFFFFFFFF
                self.lo = product & 0xFFFFFFFF
                self.hi = (product >> 32) & 0xFFFFFFFF
            elif function == 0x21:     # addu
                r[rd] = u32(r[rs] + r[rt])
            elif function == 0x23:     # subu
                r[rd] = u32(r[rs] - r[rt])
            elif function == 0x2A:     # slt
                r[rd] = int(s32(r[rs]) < s32(r[rt]))
            elif function == 0x2B:     # sltu
                r[rd] = int(u32(r[rs]) < u32(r[rt]))
            else:
                raise SystemExit(f"FATAL: SPECIAL {function:02X} @{address:08X}")
        elif op == 0x09:               # addiu
            r[rt] = u32(r[rs] + simm)
        elif op == 0x0D:               # ori
            r[rt] = u32(r[rs] | immediate)
        elif op == 0x0F:               # lui
            r[rt] = u32(immediate << 16)
        elif op == 0x21:               # lh
            r[rt] = self.load(u32(r[rs] + simm), 2, True, "lh")
        elif op == 0x23:               # lw
            r[rt] = self.load(u32(r[rs] + simm), 4, False, "lw")
        elif op == 0x24:               # lbu
            r[rt] = self.load(u32(r[rs] + simm), 1, False, "lbu")
        elif op == 0x25:               # lhu
            r[rt] = self.load(u32(r[rs] + simm), 2, False, "lhu")
        elif op == 0x29:               # sh
            self.store(u32(r[rs] + simm), 2, r[rt], "sh")
        elif op == 0x2B:               # sw
            self.store(u32(r[rs] + simm), 4, r[rt], "sw")
        else:
            raise SystemExit(f"FATAL: opcode {op:02X} @{address:08X}")
        r[0] = 0

    def run(self):
        pc = BASE
        steps = 0
        while pc != RETURN:
            steps += 1
            if steps > MAX_STEPS:
                raise SystemExit("FATAL: step limit")
            if pc < BASE or pc >= END or (pc - BASE) & 3:
                raise SystemExit(f"FATAL: bad PC {pc:08X}")
            word = W[(pc - BASE) // 4]
            op = word >> 26
            rs = (word >> 21) & 31
            rt = (word >> 16) & 31
            simm = (word & 0xFFFF)
            if simm & 0x8000:
                simm -= 0x10000
            if op in (2, 3):           # j / jal
                target = u32(((pc + 4) & 0xF0000000)
                             | ((word & 0x03FFFFFF) << 2))
                self.control_issues += 1
                if op == 3:
                    self.r[31] = u32(pc + 8)
                self.one(W[(pc + 4 - BASE) // 4], pc + 4)
                self.delay_executions += 1
                if op == 2:
                    pc = target
                    continue
                if target == 0x8005B91C:
                    self.dep_5b91c()
                elif target == 0x8005DBAC:
                    self.dep_5dbac()
                elif target == 0x80052F24:
                    self.dep_52f24()
                else:
                    raise SystemExit(f"FATAL: unexpected jal {target:08X}")
                pc = u32(pc + 8)
                continue
            if op in (4, 5):           # beq / bne
                equal = u32(self.r[rs]) == u32(self.r[rt])
                taken = equal if op == 4 else not equal
                target = u32(pc + 4 + simm * 4)
                self.control_issues += 1
                self.one(W[(pc + 4 - BASE) // 4], pc + 4)
                self.delay_executions += 1
                pc = target if taken else u32(pc + 8)
                continue
            if op == 0 and (word & 0x3F) == 8:  # jr
                target = u32(self.r[rs])
                self.control_issues += 1
                self.one(W[(pc + 4 - BASE) // 4], pc + 4)
                self.delay_executions += 1
                pc = target
                continue
            self.one(word, pc)
            pc = u32(pc + 4)
        self.events.append(("top_return", u32(self.r[2])))


def seed_full(oracle):
    targets = [-32768, -1, 0, 1, 0x1234, 0x7FFF, -2]
    for i, value in enumerate(targets):
        oracle.poke(0x800C0E28 + i * 2, 2, value)
    oracle.poke(0x800A803C, 4, 0x00007FD8)  # DBAC index 0 -> 0x800B0000
    oracle.poke(0x800A1B30, 4, 0)
    for i, value in enumerate([1, 2, 3, 4, 5]):
        oracle.poke(0x800A1B34 + i * 4, 4, value)
    oracle.outputs_5b91c = [0, 1, 2, 3, 4, 5, 6]
    oracle.returns_5b91c = [0, 1, 0xFFFFFFFF, 7,
                             0x80000000, 0x7FFFFFFF, 0xA5A5A5A5]

    # DBAC indices after the first call: 2,4,6,8,10,6.
    base = 0x800B0000
    oracle.poke(base + 0 * 24 + 0, 2, 5)
    oracle.poke(base + 2 * 24 + 2, 2, 0x1111)
    oracle.poke(base + 4 * 24 + 4, 2, 0x2222)
    oracle.poke(base + 6 * 24 + 8, 4, 0x33333333)
    oracle.poke(base + 6 * 24 + 12, 4, 0x44444444)
    oracle.poke(base + 6 * 24 + 16, 4, 0x55555555)
    oracle.poke(base + 8 * 24 + 0x14, 2, 0x6666)
    oracle.poke(base + 8 * 24 + 0x16, 2, 0x7777)
    oracle.poke(base + 10 * 24 + 6, 1, 0x88)
    oracle.poke(base + 6 * 24 + 7, 1, 0x99)

    holder = 0x800C1000
    record = 0x800C1100
    oracle.poke(0x8009D254, 4, holder)
    oracle.poke(holder, 4, record)
    oracle.poke(record + 0x0C, 2, 100)
    oracle.poke(record + 0x0E, 2, 200)
    oracle.poke(0x800C0E08, 2, 1000)
    oracle.poke(0x800C0E0A, 1, 9)
    oracle.return_52f24 = 0xDEADBEEF
    oracle.effects_52f24 = [(0x800C0E0C, 1, 0x32),
                             (GP + 0x2E0, 4, 0x12345678)]
    return record


def calls(oracle, symbol):
    return [event for event in oracle.events
            if event[0] == "call" and event[1] == symbol]


def writes(oracle, context=None):
    result = [event for event in oracle.events if event[0] == "write"]
    if context is not None:
        result = [event for event in result if event[1] == context]
    return result


def test_unaligned_and_arithmetic(executable):
    oracle = Oracle(executable)
    oracle.poke(0x80001001, 4, 0xA1B2C3D4)
    assert oracle.peek(0x80001001, 4) == 0xA1B2C3D4
    assert oracle.load(0x80001002, 2, False, "self-lhu") == 0xB2C3
    oracle.r[1] = 0xFFFFFFFD
    oracle.r[2] = 7
    oracle.one(0x00220018, BASE)       # mult r1,r2
    assert oracle.hi == 0xFFFFFFFF and oracle.lo == 0xFFFFFFEB
    oracle.one(0x00001810, BASE)       # mfhi r3
    oracle.one(0x00002012, BASE)       # mflo r4
    assert oracle.r[3] == 0xFFFFFFFF and oracle.r[4] == 0xFFFFFFEB


def test_first_boundary(executable):
    oracle = Oracle(executable)
    oracle.poke(0x800C0E28, 2, 0x8001)
    oracle.outputs_5b91c[0] = 0
    oracle.poke(0x800A803C, 4, 0)
    oracle.poke(0x8009D254, 4, 0)
    oracle.run()
    first = calls(oracle, "func_8005B91C")[0]
    assert first[2] == (0, 0xFFFF8001, STACK - 0x20, 0)
    before = next(e for e in oracle.events if e[0] == "boundary_before")
    assert before[1] == "func_8005B91C"
    assert before[3][1:] == (0, 0, 0, 0)
    # Before the dependency: only transient saves, no persistent write.
    before_index = oracle.events.index(before)
    prior_writes = [e for e in oracle.events[:before_index]
                    if e[0] == "write"]
    assert [(e[3], e[4]) for e in prior_writes] == [
        (STACK - 0x10, 4), (STACK - 0x08, 4), (STACK - 0x0C, 4)]
    assert oracle.peek(0x800C0E06, 2) == 0


def test_full_and_controlled_returns(executable):
    first = Oracle(executable)
    record = seed_full(first)
    first.run()
    assert [e[2] for e in calls(first, "func_8005B91C")] == [
        (0, 0xFFFF8000, STACK - 0x20, 0),
        (1, 0xFFFFFFFF, STACK - 0x20, 0),
        (2, 0, STACK - 0x20, 0),
        (3, 1, STACK - 0x20, 0),
        (4, 0x1234, STACK - 0x20, 0),
        (5, 0x7FFF, STACK - 0x20, 0),
        (6, 0xFFFFFFFE, STACK - 0x20, 0),
    ]
    assert [e[2][0] for e in calls(first, "func_8005DBAC")] == [
        0, 2, 4, 6, 8, 10, 6,
    ]
    assert calls(first, "func_80052F24")[0][2] == (0x99,)
    assert first.peek(0x800C0E06, 2) == 5
    assert first.peek(record + 0x1E, 2) == 0x1111
    assert first.peek(record + 0x20, 2) == 0x2222
    assert first.peek(record + 0x28, 4) == 0x33333333
    assert first.peek(record + 0x30, 4) == 0x44444444
    assert first.peek(record + 0x2C, 4) == 0x55555555
    assert first.peek(record + 0x3C, 2) == 0x6666
    assert first.peek(record + 0x3E, 2) == 0x7777
    assert first.peek(record + 0x22, 2) == 0x88
    assert first.peek(record + 0x26, 2) == 0x99
    assert first.peek(record + 4, 2) == 10
    assert first.control_issues == first.delay_executions

    # Change every dependency return while preserving its controlled state
    # effects.  B43's persistent result must remain byte-identical.
    second = Oracle(executable)
    seed_full(second)
    second.returns_5b91c = [0xDEADBEEF] * 7
    second.return_52f24 = 0
    second.run()
    assert first.ram == second.ram


def test_dirty_incoming_and_repeat(executable):
    first = Oracle(executable)
    first.r[4:8] = [0xAAAAAAAA, 0xBBBBBBBB, 0xCCCCCCCC, 0xDDDDDDDD]
    first.poke(0x800C0E28, 2, 0xFFFE)
    first.poke(0x800A803C, 4, 0)
    first.poke(0x8009D254, 4, 0)
    first.run()
    assert calls(first, "func_8005B91C")[0][2] == (
        0, 0xFFFFFFFE, STACK - 0x20, 0)
    second = Oracle(executable)
    second.ram[:] = first.ram
    second.outputs_5b91c[0] = 0
    second.run()
    assert calls(second, "func_8005B91C")[0][2] == (
        0, 0xFFFFFFFE, STACK - 0x20, 0)


def main():
    executable = (sys.argv[1] if len(sys.argv) > 1 else
                  "build/extracted/disc1/SLUS_006.62")
    assert len(W) == 155 and END - BASE == len(W) * 4
    test_unaligned_and_arithmetic(executable)
    test_first_boundary(executable)
    test_full_and_controlled_returns(executable)
    test_dirty_incoming_and_repeat(executable)
    print("B43 func_8005218C oracle: PASS")
    print("  155/155 body words; 5/5 executable caller contexts")
    print("  7 func_8005B91C + 7 func_8005DBAC + 1 func_80052F24 calls")
    print("  delay slots, controlled state outputs/returns, partial boundaries")


if __name__ == "__main__":
    main()
