#!/usr/bin/env python3
"""Independent Phase 6E-B41 MIPS-I oracle for func_80053968.

The literal 120-word body below is verified against the SHA-1-exact retail
executable before a delay-slot-aware interpreter executes it.  The sole
direct caller and func_80053D2C's exact zero/nonzero return consumption are
also checked from executable words.  Production C is never loaded or called.
"""
import hashlib
import struct
import sys
from pe_exe_words import exe_words

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
BASE = 0x80053968
END = 0x80053B48
GP = 0x8009CD70
RAM_BASE = 0x80000000
RAM_END = 0x80200000
STACK = 0x801FFF00
RETURN = 0xDEADC0DE
MAX_STEPS = 4096

RECORD_BASE = 0x800C0EAC
RECORD_END = 0x800C1EAC
PRIMARY_ID_BASE = 0x800C0E48

# func_80053968, executable 0x80053968..0x80053B47, file 0x44168.
W = exe_words(0x80053968, 120)

# jr jump-table dispatch, sole jal, its delay slot, following jump, and the
# caller's return-normalizing delay slot.
CALLER_BASE = 0x80053DF0
CALLER_WORDS = [
    0x00400008, 0x00000000, 0x0C014E5A,
    0x02002021, 0x08014F93, 0x2C510001,
]
JUMP_TABLE_BASE = 0x8001125C
JUMP_TABLE = ([0x80053DF8] * 9 + [0x80053E08, 0x80053E4C] +
              [0x80053E08] * 4 + [0x80053E1C] * 3)


def u32(value):
    return value & 0xFFFFFFFF


def s32(value):
    value = u32(value)
    return value - 0x100000000 if value & 0x80000000 else value


class Oracle:
    def __init__(self, executable, db44_return=0x800A9001,
                 f52f70_return=7):
        data = open(executable, "rb").read()
        digest = hashlib.sha1(data).hexdigest()
        if digest != SHA1:
            raise SystemExit(f"FATAL: executable SHA-1 {digest} != {SHA1}")
        self.data = data
        self.taddr = struct.unpack_from("<I", data, 0x18)[0]
        self._verify_words(BASE, W, "func_80053968")
        self._verify_words(CALLER_BASE, CALLER_WORDS,
                           "func_80053D2C consumed-return site")
        self._verify_words(JUMP_TABLE_BASE, JUMP_TABLE,
                           "func_80053D2C type jump table")
        jal_word = 0x0C014E5A
        jal_hits = []
        for offset in range(0x800, len(data) - 3, 4):
            if struct.unpack_from("<I", data, offset)[0] == jal_word:
                jal_hits.append(self.taddr + offset - 0x800)
        if jal_hits != [0x80053DF8]:
            raise SystemExit(f"FATAL: func_80053968 jal census {jal_hits}")
        if struct.pack("<I", BASE) in data:
            raise SystemExit("FATAL: unexpected literal func_80053968 pointer")

        self.ram = bytearray(RAM_END - RAM_BASE)
        self.r = [0] * 32
        self.r[28] = GP
        self.r[29] = STACK
        self.r[31] = RETURN
        self.db44_return = u32(db44_return)
        self.f52f70_return = u32(f52f70_return)
        self.events = []
        self.calls = []
        self.control_issues = 0
        self.delay_executions = 0

    def _verify_words(self, base, words, label):
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

    def lwl(self, rt, address):
        effective = u32(address)
        aligned = effective & ~3
        word = self.peek(aligned, 4)
        lane = effective & 3
        masks = (0xFF000000, 0xFFFF0000, 0xFFFFFF00, 0xFFFFFFFF)
        shifts = (24, 16, 8, 0)
        keep = u32(~masks[lane])
        self.r[rt] = u32((self.r[rt] & keep) | (word << shifts[lane]))
        self.events.append(("read", "lwl", effective, aligned,
                            masks[lane], self.r[rt]))

    def lwr(self, rt, address):
        effective = u32(address)
        aligned = effective & ~3
        word = self.peek(aligned, 4)
        lane = effective & 3
        masks = (0xFFFFFFFF, 0x00FFFFFF, 0x0000FFFF, 0x000000FF)
        shifts = (0, 8, 16, 24)
        keep = u32(~masks[lane])
        self.r[rt] = u32((self.r[rt] & keep) | (word >> shifts[lane]))
        self.events.append(("read", "lwr", effective, aligned,
                            masks[lane], self.r[rt]))

    def swl(self, rt, address):
        effective = u32(address)
        aligned = effective & ~3
        old = self.peek(aligned, 4)
        lane = effective & 3
        masks = (0x000000FF, 0x0000FFFF, 0x00FFFFFF, 0xFFFFFFFF)
        shifts = (24, 16, 8, 0)
        new = u32((old & u32(~masks[lane])) |
                  ((self.r[rt] >> shifts[lane]) & masks[lane]))
        self.poke(aligned, 4, new)
        self.events.append(("write", "swl", effective, aligned,
                            masks[lane], self.r[rt]))

    def swr(self, rt, address):
        effective = u32(address)
        aligned = effective & ~3
        old = self.peek(aligned, 4)
        lane = effective & 3
        masks = (0xFFFFFFFF, 0xFFFFFF00, 0xFFFF0000, 0xFF000000)
        shifts = (0, 8, 16, 24)
        new = u32((old & u32(~masks[lane])) |
                  ((self.r[rt] << shifts[lane]) & masks[lane]))
        self.poke(aligned, 4, new)
        self.events.append(("write", "swr", effective, aligned,
                            masks[lane], self.r[rt]))

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
            elif function == 0x2B:     # sltu
                r[rd] = int(u32(r[rs]) < u32(r[rt]))
            elif function == 0x08:     # jr (handled by run)
                return
            else:
                raise SystemExit(f"FATAL: SPECIAL {function:02X} @{address:08X}")
        elif op == 0x09:               # addiu
            r[rt] = u32(r[rs] + simm)
        elif op == 0x0B:               # sltiu
            r[rt] = int(u32(r[rs]) < u32(simm))
        elif op == 0x0F:               # lui
            r[rt] = u32(immediate << 16)
        elif op == 0x21:               # lh
            r[rt] = self.load(u32(r[rs] + simm), 2, True, "lh")
        elif op == 0x22:               # lwl
            self.lwl(rt, u32(r[rs] + simm))
        elif op == 0x23:               # lw
            r[rt] = self.load(u32(r[rs] + simm), 4, False, "lw")
        elif op == 0x24:               # lbu
            r[rt] = self.load(u32(r[rs] + simm), 1, False, "lbu")
        elif op == 0x26:               # lwr
            self.lwr(rt, u32(r[rs] + simm))
        elif op == 0x29:               # sh
            self.store(u32(r[rs] + simm), 2, r[rt], "sh")
        elif op == 0x2A:               # swl
            self.swl(rt, u32(r[rs] + simm))
        elif op == 0x2B:               # sw
            self.store(u32(r[rs] + simm), 4, r[rt], "sw")
        elif op == 0x2E:               # swr
            self.swr(rt, u32(r[rs] + simm))
        else:
            raise SystemExit(f"FATAL: opcode {op:02X} @{address:08X}")
        r[0] = 0

    def run(self, resource_id, caller_a1=0x11111111,
            caller_a2=0x22222222, caller_a3=0x33333333):
        self.r[4] = u32(resource_id)
        self.r[5] = u32(caller_a1)
        self.r[6] = u32(caller_a2)
        self.r[7] = u32(caller_a3)
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

            if op == 1 and rt == 0:     # bltz, predicate sampled at issue
                self.control_issues += 1
                taken = s32(self.r[rs]) < 0
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
            if op == 2:                 # j
                self.control_issues += 1
                target = ((word & 0x03FFFFFF) << 2) | (address & 0xF0000000)
                delay()
                pc = (target - BASE) // 4
                continue
            if op == 3:                 # controlled translated dependency
                self.control_issues += 1
                target = ((word & 0x03FFFFFF) << 2) | (address & 0xF0000000)
                self.r[31] = u32(address + 8)
                delay()
                regs = tuple(u32(self.r[index]) for index in range(4, 8))
                if target == 0x8005DB44:
                    name = "func_8005DB44"
                    result = self.db44_return
                elif target == 0x80052F70:
                    name = "func_80052F70"
                    result = self.f52f70_return
                else:
                    raise SystemExit(f"FATAL: unexpected call {target:08X}")
                self.calls.append((name, regs))
                self.events.append(("call", name, regs))
                self.r[2] = result
                self.events.append(("return", name, result))
                pc += 2
                continue
            if op == 0 and function == 8:  # jr
                self.control_issues += 1
                target = self.r[rs]
                delay()
                if target != RETURN:
                    raise SystemExit(f"FATAL: unexpected jr {target:08X}")
                self.events.append(("return", "func_80053968", self.r[2]))
                if self.control_issues != self.delay_executions:
                    raise SystemExit("FATAL: delay-slot execution mismatch")
                return u32(self.r[2])

            self.one(word, address)
            pc += 1


def seed_tables(oracle, record_free, id_base, id_count, id_free):
    for slot in range(record_free):
        oracle.poke(RECORD_BASE + slot * 0x20, 1, 0x40 + slot)
    oracle.poke(RECORD_BASE + record_free * 0x20, 1, 0)
    oracle.poke(GP + 0x2D8, 4, id_base)
    oracle.poke(GP + 0x2E0, 4, id_count)
    for slot in range(id_count):
        oracle.poke(id_base + slot * 2, 2, 0 if slot == id_free else slot + 1)


def persistent_events(oracle):
    return [event for event in oracle.events
            if not (len(event) >= 4 and isinstance(event[2], int) and
                    STACK - 0x28 <= event[2] < STACK)]


def caller_consumes(executable, provider_return, resource_id=0x44,
                    a1=0x12345678, a2=0x89ABCDEF, a3=0x0BADF00D):
    oracle = Oracle(executable)
    oracle.r[16] = u32(resource_id)
    oracle.r[4] = 0xAAAAAAAA
    oracle.r[5] = u32(a1)
    oracle.r[6] = u32(a2)
    oracle.r[7] = u32(a3)
    oracle.one(CALLER_WORDS[3], 0x80053DFC)  # delay: a0=s0
    args = tuple(oracle.r[index] for index in range(4, 8))
    oracle.r[2] = u32(provider_return)
    oracle.one(CALLER_WORDS[5], 0x80053E04)  # sltiu s1,v0,1
    return oracle.r[17], args


def self_tests(executable):
    for offset in range(4):
        oracle = Oracle(executable)
        source = 0x80010004 + offset
        for index in range(12):
            oracle.poke(0x80010000 + index, 1, 0x10 + index)
        oracle.r[1] = source
        oracle.r[2] = 0xDEADBEEF
        oracle.one(0x88220003, BASE)      # lwl v0,3(at)
        oracle.one(0x98220000, BASE + 4)  # lwr v0,0(at)
        assert oracle.r[2] == int.from_bytes(
            bytes(0x10 + 4 + offset + i for i in range(4)), "little")

        destination = 0x80010104 + offset
        for index in range(12):
            oracle.poke(0x80010100 + index, 1, 0xA5)
        oracle.r[1] = destination
        oracle.r[2] = 0x44332211
        oracle.one(0xA8220003, BASE + 8)   # swl v0,3(at)
        oracle.one(0xB8220000, BASE + 12)  # swr v0,0(at)
        assert oracle.peek(destination, 4) == 0x44332211
        assert oracle.peek(destination - 1, 1) == 0xA5
        assert oracle.peek(destination + 4, 1) == 0xA5
    print("  interpreter self-tests: little-endian LWL/LWR/SWL/SWR offsets, guards OK")


def main():
    if len(sys.argv) != 2:
        raise SystemExit("usage: b41_oracle.py executable")
    executable = sys.argv[1]
    self_tests(executable)

    oracle = Oracle(executable)
    for slot in range(128):
        oracle.poke(RECORD_BASE + slot * 0x20, 1, 1)
    oracle.poke(GP + 0x2D8, 4, 0x800C2000)
    oracle.poke(GP + 0x2E0, 4, 1)
    oracle.poke(0x800C2000, 2, 0)
    assert oracle.run(1) == 0
    assert oracle.calls == []
    assert not [event for event in persistent_events(oracle)
                if event[0] == "write"]
    print("  S1: full 128-record destination scans then returns NULL without writes OK")

    oracle = Oracle(executable)
    seed_tables(oracle, 0, 0x800C2000, 3, -1)
    assert oracle.run(1) == 0
    assert oracle.calls == []
    assert not [event for event in persistent_events(oracle)
                if event[0] == "write"]
    print("  S2: full active ID table returns NULL without dependency calls/writes OK")

    source = 0x800A9001
    oracle = Oracle(executable, db44_return=source, f52f70_return=7)
    seed_tables(oracle, 2, 0x800C2000, 4, 1)
    source_bytes = bytes((index * 9 + 3) & 0xFF for index in range(32))
    for index, value in enumerate(source_bytes):
        oracle.poke(source + index, 1, value)
    result = oracle.run(0x44)
    destination = RECORD_BASE + 2 * 0x20
    assert result == destination
    assert bytes(oracle.ram[oracle.off(destination, 32):
                            oracle.off(destination, 32) + 32]) == source_bytes
    assert oracle.peek(GP + 0x2D8, 4) == PRIMARY_ID_BASE
    assert oracle.peek(GP + 0x2E0, 4) == 7
    assert oracle.peek(GP + 0x2E8, 4) == 0x8009D05C
    assert oracle.peek(GP + 0x2F4, 4) == 2
    assert oracle.peek(PRIMARY_ID_BASE + 2, 2) == 0x102
    assert oracle.calls[0][0] == "func_8005DB44"
    assert oracle.calls[0][1][0] == 0x43
    assert oracle.calls[1][0] == "func_80052F70"
    opcodes = [event[1] for event in oracle.events
               if event[0] in ("read", "write") and
               event[1] in ("lwl", "lwr", "swl", "swr")]
    assert opcodes == ((["lwl", "lwr"] * 4) +
                       (["swl", "swr"] * 4) +
                       (["lwl", "lwr"] * 4) +
                       (["swl", "swr"] * 4))
    print("  S3: success return, exact two-group copy, state writes, dependency order OK")

    oracle = Oracle(executable, db44_return=RECORD_BASE - 8,
                    f52f70_return=2)
    seed_tables(oracle, 0, PRIMARY_ID_BASE, 2, 0)
    original = bytearray((0x20 + index) & 0xFF for index in range(32))
    original[8] = 0                       # destination byte zero is free
    for index, value in enumerate(original):
        oracle.poke(RECORD_BASE - 8 + index, 1, value)
    assert oracle.run(1) == RECORD_BASE
    expected = original[0:16] + original[8:16] + original[24:32]
    assert bytes(oracle.ram[oracle.off(RECORD_BASE, 32):
                            oracle.off(RECORD_BASE, 32) + 32]) == expected
    print("  S4: overlapping source proves first-16 store precedes second-16 loads OK")

    oracle = Oracle(executable, db44_return=source, f52f70_return=4)
    seed_tables(oracle, 0, PRIMARY_ID_BASE, 4, 0)
    for slot in range(4):
        oracle.poke(PRIMARY_ID_BASE + slot * 2, 2, 0)
    for index, value in enumerate(source_bytes):
        oracle.poke(source + index, 1, value)
    first = oracle.run(1)
    second = oracle.run(1)
    assert first == RECORD_BASE and second == RECORD_BASE + 0x20
    assert oracle.peek(PRIMARY_ID_BASE, 2) == 0x100
    assert oracle.peek(PRIMARY_ID_BASE + 2, 2) == 0x101
    print("  S5: first/repeated invocation allocates successive record and ID slots OK")

    for provider_return, expected in ((0, 1), (1, 0),
                                      (0xFFFFFFFF, 0), (7, 0),
                                      (0x800C0EAC, 0)):
        got, args = caller_consumes(executable, provider_return)
        assert got == expected
        assert args == (0x44, 0x12345678, 0x89ABCDEF, 0x0BADF00D)
    assert JUMP_TABLE[:9] == [0x80053DF8] * 9
    print("  S6: sole caller types 1..9 and controlled 0/1/-1/positive/high returns normalize exactly OK")

    print(f"PASS: B41 exe SHA-1 {SHA1}; {len(W)}/120 body words, "
          "1/1 direct call site, caller jump table/consumed return, delay slots, "
          "signed/unsigned loops, exact unaligned accesses, ordered calls/returns, "
          "NULL/full/success/overlap/repeated paths")


if __name__ == "__main__":
    main()
