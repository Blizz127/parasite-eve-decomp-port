#!/usr/bin/env python3
"""Independent Phase 6E-B42 MIPS-I oracle for func_80053B48.

The literal 121-word body is checked against the SHA-1-exact executable and
then executed by this delay-slot-aware interpreter.  Both direct executable
call sites and their exact return consumers are independently checked.
Production C is never loaded or called.
"""
import hashlib
import struct
import sys
from pe_exe_words import exe_words

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
BASE = 0x80053B48
END = 0x80053D2C
GP = 0x8009CD70
RAM_BASE = 0x80000000
RAM_END = 0x80200000
STACK = 0x801FFF00
RETURN = 0xDEADC0DE
MAX_STEPS = 4096

TABLE = 0x800C2000
INPUT = 0x800A9000
CATEGORY_BASE = 0x800A1E64

# func_80053B48, executable 0x80053B48..0x80053D2B, file 0x44348.
W = exe_words(0x80053B48, 121)

# func_80053D2C exact B42 call/post-call words.  The common return move is a
# separate verified word because the jump skips over the intervening paths.
C1_BASE = 0x80053E1C
C1 = [0x0C014ED2, 0x00000000, 0x08014F93, 0x00408821]
C1_RETURN_BASE = 0x80053E4C
C1_RETURN = [0x02201021]

# func_8005833C call through its complete return-consumer tail.
C2_BASE = 0x80058408
C2 = exe_words(0x80058408, 19)


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
        self.verify_words(BASE, W, "func_80053B48")
        self.verify_words(C1_BASE, C1, "func_80053D2C B42 consumer")
        self.verify_words(C1_RETURN_BASE, C1_RETURN,
                          "func_80053D2C return move")
        self.verify_words(C2_BASE, C2, "func_8005833C B42 consumer")

        jal_word = 0x0C014ED2
        jal_hits = []
        for offset in range(0x800, len(data) - 3, 4):
            if struct.unpack_from("<I", data, offset)[0] == jal_word:
                jal_hits.append(self.taddr + offset - 0x800)
        if jal_hits != [0x80053E1C, 0x80058408]:
            raise SystemExit(f"FATAL: func_80053B48 jal census {jal_hits}")
        if struct.pack("<I", BASE) in data:
            raise SystemExit("FATAL: unexpected literal func_80053B48 pointer")
        for index in range(len(data) // 4 - 1):
            first, second = struct.unpack_from("<II", data, index * 4)
            op1, rt1 = (first >> 26) & 0x3F, (first >> 16) & 0x1F
            op2 = (second >> 26) & 0x3F
            rs2, rt2 = (second >> 21) & 0x1F, (second >> 16) & 0x1F
            if (op1 == 0x0F and rt1 != 0 and (first & 0xFFFF) == 0x8005
                    and op2 in (0x09, 0x0D) and rs2 == rt1 and rt2 == rt1
                    and (second & 0xFFFF) == 0x3B48):
                raise SystemExit("FATAL: unexpected constructed B42 pointer")

        self.ram = bytearray(RAM_END - RAM_BASE)
        self.r = [0] * 32
        self.r[28] = GP
        self.r[29] = STACK
        self.r[31] = RETURN
        self.events = []
        self.control_issues = 0
        self.delay_executions = 0

    def verify_words(self, base, words, label):
        for index, want in enumerate(words):
            address = base + index * 4
            offset = address - self.taddr + 0x800
            got = struct.unpack_from("<I", self.data, offset)[0]
            if got != want:
                raise SystemExit(
                    f"FATAL: {label} word {index} @ {address:08X}: "
                    f"{got:08X} != {want:08X}")

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
        self.events.append(("read", opcode, address, width, value))
        return value

    def store(self, address, width, value, opcode="store"):
        address = u32(address)
        self.poke(address, width, value)
        mask = (1 << (width * 8)) - 1
        self.events.append(("write", opcode, address, width, value & mask))

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
            elif function == 0x21:     # addu
                r[rd] = u32(r[rs] + r[rt])
            elif function == 0x23:     # subu
                r[rd] = u32(r[rs] - r[rt])
            elif function == 0x2A:     # slt
                r[rd] = int(s32(r[rs]) < s32(r[rt]))
            elif function == 0x2B:     # sltu
                r[rd] = int(u32(r[rs]) < u32(r[rt]))
            elif function == 0x08:     # jr (handled by run)
                return
            else:
                raise SystemExit(f"FATAL: SPECIAL {function:02X} @{address:08X}")
        elif op == 0x09:               # addiu
            r[rt] = u32(r[rs] + simm)
        elif op == 0x0A:               # slti
            r[rt] = int(s32(r[rs]) < simm)
        elif op == 0x0B:               # sltiu
            r[rt] = int(u32(r[rs]) < u32(simm))
        elif op == 0x0C:               # andi
            r[rt] = r[rs] & immediate
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
        else:
            raise SystemExit(f"FATAL: opcode {op:02X} @{address:08X}")
        r[0] = 0

    def run(self, record, a1=0x11111111, a2=0x22222222,
            a3=0x33333333):
        self.r[4] = u32(record)
        self.r[5] = u32(a1)
        self.r[6] = u32(a2)
        self.r[7] = u32(a3)
        self.r[29] = STACK
        self.r[31] = RETURN
        pc = 0
        steps = 0
        while True:
            if steps >= MAX_STEPS:
                raise SystemExit("FATAL: oracle did not terminate")
            steps += 1
            address = BASE + pc * 4
            word = W[pc]
            op = (word >> 26) & 0x3F
            rs = (word >> 21) & 0x1F
            rt = (word >> 16) & 0x1F
            immediate = word & 0xFFFF
            simm = immediate if immediate < 0x8000 else immediate - 0x10000
            function = word & 0x3F

            def delay():
                self.delay_executions += 1
                self.one(W[pc + 1], address + 4)

            if op == 1 and rt in (0, 1):  # bltz/bgez, sampled at issue
                self.control_issues += 1
                negative = s32(self.r[rs]) < 0
                taken = negative if rt == 0 else not negative
                delay()
                pc = ((address + 4 + (simm << 2) - BASE) // 4
                      if taken else pc + 2)
                continue
            if op in (4, 5):            # beq/bne, sampled at issue
                self.control_issues += 1
                equal = self.r[rs] == self.r[rt]
                taken = equal if op == 4 else not equal
                delay()
                pc = ((address + 4 + (simm << 2) - BASE) // 4
                      if taken else pc + 2)
                continue
            if op == 6:                 # blez, sampled at issue
                self.control_issues += 1
                taken = s32(self.r[rs]) <= 0
                delay()
                pc = ((address + 4 + (simm << 2) - BASE) // 4
                      if taken else pc + 2)
                continue
            if op == 2:                 # j
                self.control_issues += 1
                target = ((word & 0x03FFFFFF) << 2) | (address & 0xF0000000)
                delay()
                pc = (target - BASE) // 4
                continue
            if op == 0 and function == 8:  # jr
                self.control_issues += 1
                target = self.r[rs]
                delay()
                if target != RETURN:
                    raise SystemExit(f"FATAL: unexpected jr {target:08X}")
                self.events.append(("return", "func_80053B48", self.r[2]))
                if self.control_issues != self.delay_executions:
                    raise SystemExit("FATAL: delay-slot execution mismatch")
                return u32(self.r[2])

            self.one(word, address)
            pc += 1


def seed(oracle, record_type, amount, category, table_values,
         old_value=10, base_value=20, delta=0):
    oracle.poke(INPUT + 6, 1, record_type)
    oracle.poke(INPUT + 0xA, 2, amount)
    oracle.poke(GP + 0x2D8, 4, TABLE)
    oracle.poke(GP + 0x2E0, 4, len(table_values))
    for index, value in enumerate(table_values):
        oracle.poke(TABLE + index * 2, 2, value)
    fixed = CATEGORY_BASE + category * 0x20
    oracle.poke(fixed + 9, 1, base_value)
    oracle.poke(fixed + 0xA, 2, old_value)
    oracle.poke(fixed + 0x12, 2, delta)


def writes(oracle):
    return [event for event in oracle.events if event[0] == "write"]


def caller_53d2c(executable, provider_return, record=INPUT,
                  a1=0x12345678, a2=0x89ABCDEF, a3=0x0BADF00D):
    oracle = Oracle(executable)
    oracle.r[4:8] = [record, a1, a2, a3]
    args = tuple(u32(value) for value in oracle.r[4:8])
    oracle.events.append(("call", "func_80053B48", args))
    oracle.r[2] = u32(provider_return)
    oracle.events.append(("return", "func_80053B48", oracle.r[2]))
    oracle.one(C1[3], C1_BASE + 0xC)       # j delay: s1=v0
    oracle.one(C1_RETURN[0], C1_RETURN_BASE)  # v0=s1
    return oracle.r[2], args, oracle.events


def caller_5833c(executable, provider_return, record=INPUT,
                 index=3, a1=0x12345678, a2=0x89ABCDEF,
                 a3=0x0BADF00D):
    oracle = Oracle(executable)
    oracle.r[16] = record                  # s0
    oracle.r[17] = index                   # s1
    oracle.r[4:8] = [0xAAAAAAAA, a1, a2, a3]
    oracle.poke(0x800A1FD4 + 2 * index, 2, 0x5555)
    oracle.one(C2[1], C2_BASE + 4)         # jal delay: a0=s0
    args = tuple(u32(value) for value in oracle.r[4:8])
    oracle.events.append(("call", "func_80053B48", args))
    oracle.r[2] = u32(provider_return)
    oracle.events.append(("return", "func_80053B48", oracle.r[2]))
    oracle.one(C2[2], C2_BASE + 8)         # v1=v0
    taken = oracle.r[3] != 0               # bnez sampled before delay
    oracle.one(C2[4], C2_BASE + 0x10)      # delay: v0=v1
    if not taken:
        for offset in range(5, 9):
            oracle.one(C2[offset], C2_BASE + offset * 4)
        oracle.one(C2[10], C2_BASE + 0x28)  # j delay: v0=v1
    return oracle.r[2], args, oracle.events, oracle.peek(0x800A1FD4 + 2*index, 2)


def self_tests(executable):
    oracle = Oracle(executable)
    oracle.r[1] = 0xFFFFFFFF
    oracle.one(0x282203E8, BASE)       # slti v0,at,1000
    assert oracle.r[2] == 1
    oracle.r[1] = 0x80000000
    oracle.r[2] = 1
    oracle.one(0x0022182A, BASE + 4)   # slt v1,at,v0
    assert oracle.r[3] == 1
    oracle.r[1] = 0x1234ABCD
    oracle.one(0x3022FFFF, BASE + 8)   # andi v0,at,0xffff
    assert oracle.r[2] == 0xABCD
    oracle.poke(INPUT, 2, 0xFEDC)
    oracle.r[1] = INPUT
    oracle.one(0x94220000, BASE + 12)  # lhu v0,0(at)
    assert oracle.r[2] == 0xFEDC
    print("  interpreter self-tests: slti/slt/andi/lhu and signed predicates OK")


def main():
    if len(sys.argv) != 2:
        raise SystemExit("usage: b42_oracle.py executable")
    executable = sys.argv[1]
    self_tests(executable)

    mapping = [(1, 0), (2, 0), (3, 0), (4, 0), (5, 0),
               (6, 1), (7, 2), (16, 0), (17, 1), (18, 2)]
    for record_type, category in mapping:
        oracle = Oracle(executable)
        seed(oracle, record_type, 3, category, [0x200 + category])
        assert oracle.run(INPUT) == 0
        assert oracle.peek(CATEGORY_BASE + category * 0x20 + 0xA, 2) == 13
    for record_type in (0, 8, 15, 19, 0xFF):
        oracle = Oracle(executable)
        oracle.poke(INPUT + 6, 1, record_type)
        oracle.poke(GP + 0x2D8, 4, 0)
        oracle.poke(GP + 0x2E0, 4, 0x80000000)
        assert oracle.run(INPUT) == 1
        assert not writes(oracle)
        assert [event[2:4] for event in oracle.events if event[0] == "read"] == [(INPUT + 6, 1)]
    print("  S1: true ABI and complete 1..7/16..18 type classification/unsupported returns OK")

    oracle = Oracle(executable)
    seed(oracle, 17, 7, 1, [0xFFFF, 0x111, 0x201, 0x222],
         old_value=11, base_value=30, delta=-4)
    assert oracle.run(INPUT) == 0
    assert oracle.peek(TABLE + 4, 2) == 0x201
    assert writes(oracle) == [("write", "sh", CATEGORY_BASE + 0x2A, 2, 18)]
    print("  S2: signed-halfword existing-ID scan and exact single update width/order OK")

    oracle = Oracle(executable)
    seed(oracle, 18, 1, 2, [0x111, 0x222, 0x333, 0])
    assert oracle.run(INPUT) == 0
    assert oracle.peek(TABLE + 6, 2) == 0x202
    assert writes(oracle) == [
        ("write", "sh", TABLE + 6, 2, 0x202),
        ("write", "sh", CATEGORY_BASE + 0x4A, 2, 11),
    ]
    oracle = Oracle(executable)
    oracle.poke(INPUT + 6, 1, 16)
    oracle.poke(INPUT + 0xA, 2, 1)
    oracle.poke(GP + 0x2D8, 4, RAM_END - 2)
    oracle.poke(GP + 0x2E0, 4, 1)
    oracle.poke(RAM_END - 2, 2, 0)
    oracle.poke(CATEGORY_BASE + 9, 1, 20)
    oracle.poke(CATEGORY_BASE + 0xA, 2, 2)
    oracle.poke(CATEGORY_BASE + 0x12, 2, 0)
    assert oracle.run(INPUT) == 0
    assert oracle.peek(RAM_END - 2, 2) == 0x200
    print("  S3: missing-ID second scan, last/max-slot insertion, and ROM write order OK")

    oracle = Oracle(executable)
    seed(oracle, 16, 5, 0, [0x111, 0x222, 0x333], old_value=7)
    assert oracle.run(INPUT) == 1
    assert writes(oracle) == [("write", "sh", CATEGORY_BASE + 0xA, 2, 12)]
    assert oracle.peek(CATEGORY_BASE + 0xA, 2) == 12
    oracle = Oracle(executable)
    seed(oracle, 16, 4, 0, [], old_value=6)
    assert oracle.run(INPUT) == 1
    assert writes(oracle) == [("write", "sh", CATEGORY_BASE + 0xA, 2, 10)]
    print("  S4: zero/full-table return one preserves the proven later partial-state write OK")

    cases = [
        (7, 3, 20, 0, 10, 1),
        (7, 3, 5, 0, 5, 2),
        (996, 3, 255, 745, 999, 1),
        (997, 3, 255, 745, 999, 2),
        (0, 0, 0, -1, 0xFFFF, 2),
        (0xFFFF, 2, 20, 0, 1, 1),
    ]
    for old, amount, base_value, delta, expected, write_count in cases:
        oracle = Oracle(executable)
        seed(oracle, 16, amount, 0, [0x200], old,
             base_value, delta)
        assert oracle.run(INPUT) == 0
        assert oracle.peek(CATEGORY_BASE + 0xA, 2) == expected
        assert len(writes(oracle)) == write_count
    print("  S5: every threshold branch, signed negative, cap, and 16-bit wrap path OK")

    oracle = Oracle(executable)
    seed(oracle, 16, 1, 0, [0, 0], old_value=100,
         base_value=200)
    assert oracle.run(INPUT) == 0
    oracle.events = []
    assert oracle.run(INPUT) == 0
    assert oracle.peek(TABLE, 2) == 0x200 and oracle.peek(TABLE + 2, 2) == 0
    assert oracle.peek(CATEGORY_BASE + 0xA, 2) == 102
    assert writes(oracle) == [("write", "sh", CATEGORY_BASE + 0xA, 2, 102)]
    print("  S6: repeated invocation reuses mapping and accumulates current guest state OK")

    controlled = (0, 1, 7, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF)
    for value in controlled:
        got1, args1, events1 = caller_53d2c(executable, value)
        assert got1 == u32(value)
        assert args1 == (INPUT, 0x12345678, 0x89ABCDEF, 0x0BADF00D)
        assert events1 == [("call", "func_80053B48", args1),
                           ("return", "func_80053B48", u32(value))]
        got2, args2, events2, mapping_value = caller_5833c(
            executable, value)
        assert got2 == u32(value)
        assert args2 == (INPUT, 0x12345678, 0x89ABCDEF, 0x0BADF00D)
        assert events2[:2] == [("call", "func_80053B48", args2),
                              ("return", "func_80053B48", u32(value))]
        assert mapping_value == (0 if value == 0 else 0x5555)
    print("  S7: both callers forward 0/1/7/7FFFFFFF/80000000/FFFFFFFF exactly; arguments and zero side effect checked OK")

    print(f"PASS: B42 exe SHA-1 {SHA1}; {len(W)}/121 body words, "
          "2/2 direct call sites, both exact return consumers, delay slots, "
          "signed/unsigned loops, ordered reads/writes/returns, complete type, "
          "existing/insert/full/threshold/repeated paths")


if __name__ == "__main__":
    main()
