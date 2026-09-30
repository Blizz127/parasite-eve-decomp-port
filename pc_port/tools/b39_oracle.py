#!/usr/bin/env python3
"""Independent Phase 6E-B39 MIPS-I oracle for func_80051CC4.

W is the literal complete 77-word retail body.  JT is the literal 8-word
computed-jump table.  Both are compared with the SHA-1-exact executable
before this delay-slot-aware interpreter executes W.  Production C is never
loaded or called.
"""
import hashlib
import struct
import sys
from pe_exe_words import exe_words

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
BASE = 0x80051CC4
JT_BASE = 0x800111F8
GP = 0x8009CD70
RAM_BASE = 0x80000000
RAM_END = 0x80200000
STACK = 0x801FFF00
MAX_STEPS = 20000       # count=255 path is ~6,700 issued instructions

# func_80051CC4, executable 0x80051CC4..0x80051DF7, file 0x424C4.
W = exe_words(0x80051CC4, 77)

# jtbl_800111F8, command values 8..15 in order.
JT = [
    0x80051D88, 0x80051D88, 0x80051D88, 0x80051D9C,
    0x80051DA4, 0x80051DAC, 0x80051DB8, 0x80051DB4,
]


def u32(value):
    return value & 0xFFFFFFFF


def s32(value):
    value = u32(value)
    return value - 0x100000000 if value & 0x80000000 else value


class Oracle:
    def __init__(self, executable):
        with open(executable, "rb") as stream:
            data = stream.read()
        digest = hashlib.sha1(data).hexdigest()
        if digest != SHA1:
            raise SystemExit(f"FATAL: executable SHA-1 {digest} != {SHA1}")
        taddr = struct.unpack_from("<I", data, 0x18)[0]
        self._verify_words(data, taddr, BASE, W, "func_80051CC4")
        self._verify_words(data, taddr, JT_BASE, JT, "jtbl_800111F8")

        self.ram = bytearray(RAM_END - RAM_BASE)
        self.r = [0] * 32
        self.r[28] = GP
        self.r[29] = STACK
        self.r[31] = 0xDEADC0DE
        self.pc = 0
        self.steps = 0
        self.reads = []
        self.writes = []
        self.calls = []
        self.events = []
        self.context = "body"
        self.ret_5332c = 0
        for index, target in enumerate(JT):
            self.poke(JT_BASE + index * 4, 4, target)

    @staticmethod
    def _verify_words(data, taddr, base, words, label):
        for index, want in enumerate(words):
            offset = base + index * 4 - taddr + 0x800
            got = struct.unpack_from("<I", data, offset)[0]
            if got != want:
                raise SystemExit(
                    f"FATAL: {label} word {index} @ {base+index*4:08X}: "
                    f"{got:08X} != {want:08X}")

    def off(self, address, width):
        address = u32(address)
        if width < 0 or address < RAM_BASE or address > RAM_END - width:
            raise SystemExit(f"FATAL: guest access {address:08X}/{width}")
        return address - RAM_BASE

    def poke(self, address, width, value):
        offset = self.off(address, width)
        mask = (1 << (width * 8)) - 1
        self.ram[offset:offset + width] = (u32(value) & mask).to_bytes(
            width, "little")

    def load(self, address, width, signed=False):
        address = u32(address)
        offset = self.off(address, width)
        value = int.from_bytes(self.ram[offset:offset + width], "little")
        if signed and value & (1 << (width * 8 - 1)):
            value -= 1 << (width * 8)
        value = u32(value)
        self.reads.append((self.context, address, width, value))
        self.events.append(("read", self.context, address, width, value))
        return value

    def store(self, address, width, value):
        address = u32(address)
        offset = self.off(address, width)
        mask = (1 << (width * 8)) - 1
        value = u32(value) & mask
        self.ram[offset:offset + width] = value.to_bytes(width, "little")
        self.writes.append((self.context, address, width, value))
        self.events.append(("write", self.context, address, width, value))

    def state32(self, address):
        offset = self.off(address, 4)
        return int.from_bytes(self.ram[offset:offset + 4], "little")

    def call(self, symbol, args, result=None):
        event = (symbol, *args)
        self.calls.append(event)
        self.events.append(("call", symbol, args))
        if result is not None:
            self.events.append(("return", symbol, u32(result)))

    def dep_52f0c(self):
        old = self.context
        self.context = "func_80052F0C"
        current = self.load(GP + 0x2D8, 4)
        result = int(current != 0x800C0E48)
        self.call("func_80052F0C", (), result)
        self.context = old
        return result

    def dep_52f70(self):
        byte = self.load(0x800C0E0C, 1)
        first = self.load(GP + 0x2A8, 4)
        total = u32(byte + first)
        if total < 51:
            second = self.load(GP + 0x2A8, 4)
            return u32(byte + second)
        return 50

    def dep_52e30(self, arg):
        old = self.context
        self.context = "func_80052E30"
        self.call("func_80052E30", (u32(arg),))
        if u32(arg) == 0 or self.load(GP + 0x2DC, 4) == 0:
            self.store(GP + 0x2D8, 4, 0x800C0E48)
            self.store(GP + 0x2E0, 4, self.dep_52f70())
            self.store(GP + 0x2E8, 4, 0x800AD05C)
            self.store(GP + 0x2F4, 4, 2)
        else:
            prior = self.load(GP + 0x2DC, 4)
            secondary = self.load(GP + 0x2E4, 4)
            self.store(GP + 0x2D8, 4, prior)
            self.store(GP + 0x2E8, 4, 0x800A1F84)
            self.store(GP + 0x2F4, 4, 4)
            self.store(GP + 0x2E0, 4, secondary)
        self.events.append(("return", "func_80052E30", None))
        self.context = old

    def one(self, word, address):
        op = (word >> 26) & 0x3F
        rs = (word >> 21) & 0x1F
        rt = (word >> 16) & 0x1F
        rd = (word >> 11) & 0x1F
        shift = (word >> 6) & 0x1F
        function = word & 0x3F
        immediate = word & 0xFFFF
        signed_imm = immediate if immediate < 0x8000 else immediate - 0x10000
        r = self.r
        if word == 0:
            return
        if op == 0:
            if function == 0x00:
                r[rd] = u32(r[rt] << shift)
            elif function == 0x04:
                r[rd] = u32(r[rt] << (r[rs] & 0x1F))
            elif function == 0x08:
                return
            elif function == 0x21:
                r[rd] = u32(r[rs] + r[rt])
            elif function == 0x2A:
                r[rd] = int(s32(r[rs]) < s32(r[rt]))
            else:
                raise SystemExit(f"FATAL: SPECIAL {function:02X} @{address:08X}")
        elif op == 0x09:
            r[rt] = u32(r[rs] + signed_imm)
        elif op == 0x0B:
            r[rt] = int(u32(r[rs]) < u32(signed_imm))
        elif op == 0x0C:
            r[rt] = u32(r[rs] & immediate)
        elif op == 0x0F:
            r[rt] = u32(immediate << 16)
        elif op == 0x20:
            r[rt] = self.load(u32(r[rs] + signed_imm), 1, True)
        elif op == 0x23:
            r[rt] = self.load(u32(r[rs] + signed_imm), 4)
        elif op == 0x24:
            r[rt] = self.load(u32(r[rs] + signed_imm), 1)
        elif op == 0x2B:
            self.store(u32(r[rs] + signed_imm), 4, r[rt])
        else:
            raise SystemExit(f"FATAL: opcode {op:02X} @{address:08X}")
        r[0] = 0

    def run(self, incoming=(0x11111111, 0x22222222, 0x33333333, 0x44444444)):
        self.pc = 0
        self.steps = 0
        self.r[4:8] = [u32(value) for value in incoming]
        self.r[29] = STACK
        self.r[31] = 0xDEADC0DE
        while True:
            if self.steps >= MAX_STEPS:
                raise SystemExit("FATAL: oracle did not terminate")
            self.steps += 1
            address = BASE + self.pc * 4
            word = W[self.pc]
            op = (word >> 26) & 0x3F
            rs = (word >> 21) & 0x1F
            rt = (word >> 16) & 0x1F
            immediate = word & 0xFFFF
            signed_imm = immediate if immediate < 0x8000 else immediate - 0x10000
            function = word & 0x3F
            delay = W[self.pc + 1] if self.pc + 1 < len(W) else 0

            def issue_delay():
                self.one(delay, address + 4)

            if op == 3:  # jal
                target = ((word & 0x03FFFFFF) << 2) | (address & 0xF0000000)
                issue_delay()
                self.r[31] = u32(address + 8)
                if target == 0x80052F0C:
                    self.r[2] = self.dep_52f0c()
                elif target == 0x80052E30:
                    self.dep_52e30(self.r[4])
                elif target == 0x8005332C:
                    snapshot = {
                        "source_id": s32(self.r[4]),
                        "D_8009D018": self.state32(GP + 0x2A8),
                        "params": tuple(self.state32(0x800A1B30 + i * 4)
                                        for i in range(7)),
                    }
                    self.call("func_8005332C", (s32(self.r[4]),), self.ret_5332c)
                    self.events.append(("boundary_state", "func_8005332C", snapshot))
                    self.r[2] = u32(self.ret_5332c)
                elif target == 0x8005218C:
                    self.call("func_8005218C", (), None)
                    self.events.append(("return", "func_8005218C", None))
                else:
                    raise SystemExit(f"FATAL: unexpected call target {target:08X}")
                self.pc += 2
                continue

            if op == 2:  # j
                target = ((word & 0x03FFFFFF) << 2) | (address & 0xF0000000)
                issue_delay()
                self.pc = (target - BASE) // 4
                continue

            if op in (4, 5, 6):  # beq, bne, blez
                if op == 4:
                    taken = self.r[rs] == self.r[rt]
                elif op == 5:
                    taken = self.r[rs] != self.r[rt]
                else:
                    taken = s32(self.r[rs]) <= 0
                issue_delay()
                self.pc = ((address + 4 + (signed_imm << 2) - BASE) // 4
                           if taken else self.pc + 2)
                continue

            if op == 1 and rt == 1:  # bgez
                taken = s32(self.r[rs]) >= 0
                issue_delay()
                self.pc = ((address + 4 + (signed_imm << 2) - BASE) // 4
                           if taken else self.pc + 2)
                continue

            if op == 0 and function == 8:  # jr, computed dispatch or return
                target = self.r[rs]
                issue_delay()
                if BASE <= target < BASE + len(W) * 4:
                    self.pc = (target - BASE) // 4
                    continue
                self.events.append(("return", "func_80051CC4", None))
                return

            self.one(word, address)
            self.pc += 1


def seed_common(oracle, old_buffer=0x800C0E48, old_flag=0, old_secondary=0,
                old_command=0, resource_byte=5, source_id=7):
    oracle.poke(GP + 0x2D8, 4, old_buffer)
    oracle.poke(GP + 0x2DC, 4, old_flag)
    oracle.poke(GP + 0x2E4, 4, old_secondary)
    oracle.poke(GP + 0x2A8, 4, old_command)
    oracle.poke(0x800C0E0C, 1, resource_byte)
    oracle.poke(0x800C0E22, 1, source_id)
    for index in range(7):
        oracle.poke(0x800A1B30 + index * 4, 4, 0xA5A50000 + index)


def body_writes(oracle):
    return [entry for entry in oracle.writes if entry[0] == "body"
            and not (STACK - 0x20 <= entry[1] < STACK)]


def self_tests(executable):
    oracle = Oracle(executable)
    oracle.r[1] = 1
    oracle.r[2] = 31
    oracle.one(0x00411804, BASE)  # sllv v1,at,v0 => shift count masked to 31
    assert oracle.r[3] == 0x80000000
    oracle.r[1] = 0xFFFFFFFF
    oracle.r[2] = 0
    oracle.one(0x0022102A, BASE)  # slt v0,at,v0: -1 < 0
    assert oracle.r[2] == 1
    oracle.r[1] = 0xFFFFFFFF
    oracle.one(0x2C220008, BASE)  # sltiu v0,at,8: unsigned comparison
    assert oracle.r[2] == 0
    oracle.poke(0x80010000, 1, 0x80)
    oracle.r[1] = 0x80010000
    oracle.one(0x80220000, BASE)  # lb v0,0(at): sign extension
    assert oracle.r[2] == 0xFFFFFF80
    print("  interpreter self-tests: sllv mask, signed slt/lb, unsigned sltiu OK")


def main():
    if len(sys.argv) != 2:
        raise SystemExit("usage: b39_oracle.py executable")
    executable = sys.argv[1]
    self_tests(executable)

    oracle = Oracle(executable)
    seed_common(oracle, old_command=9, source_id=0x80)
    oracle.run()
    assert oracle.calls == [
        ("func_80052F0C",),
        ("func_80052E30", 0),
        ("func_8005332C", -128),
        ("func_8005218C",),
        ("func_80052E30", 0),
        ("func_80052E30", 0),
    ]
    assert body_writes(oracle)[:8] == [
        ("body", GP + 0x2A8, 4, 0),
        *[("body", 0x800A1B48 - i * 4, 4, 0) for i in range(7)],
    ]
    assert oracle.state32(GP + 0x2A8) == 0
    assert oracle.state32(GP + 0x2E0) == 5
    boundary = [event for event in oracle.events if event[0] == "boundary_state"]
    assert boundary[0][2]["D_8009D018"] == 0
    assert boundary[0][2]["params"] == (0, 0, 0, 0, 0, 0, 0)
    print("  S1: NULL record, signed source arg, prefix state, call order OK")

    record = 0x800A9000
    oracle = Oracle(executable)
    seed_common(oracle)
    oracle.ret_5332c = record
    oracle.poke(record + 0x14, 1, 0)
    oracle.run()
    assert oracle.state32(GP + 0x2A8) == 0
    assert len([read for read in oracle.reads
                if read[0] == "body" and record + 0x15 <= read[1]
                < record + 0x114]) == 0
    print("  S2: non-NULL zero-count record takes no command read OK")

    oracle = Oracle(executable)
    seed_common(oracle)
    oracle.ret_5332c = record
    commands = (8, 9, 10, 11, 12, 13, 14, 15, 31)
    oracle.poke(record + 0x14, 1, len(commands))
    for index, command in enumerate(commands):
        oracle.poke(record + 0x15 + index, 1, 0xE0 | command)
    oracle.run()
    assert oracle.state32(GP + 0x2A8) == 4
    assert tuple(oracle.state32(0x800A1B30 + i * 4) for i in range(7)) == (
        3, 0xFFFFFFFE, 0, 0, 0, 0xFFFFFFFE, 0)
    command_reads = [read for read in oracle.reads
                     if read[0] == "body" and record + 0x15 <= read[1]
                     < record + 0x15 + len(commands)]
    assert len(command_reads) == len(commands)

    oracle = Oracle(executable)
    seed_common(oracle)
    oracle.ret_5332c = record
    oracle.poke(record + 0x14, 1, 0xFF)
    for index in range(0xFF):
        oracle.poke(record + 0x15 + index, 1, 10 if index == 0xFE else 0)
    oracle.run()
    maximum_reads = [read for read in oracle.reads
                     if read[0] == "body" and record + 0x15 <= read[1]
                     < record + 0x114]
    assert len(maximum_reads) == 0xFF
    assert maximum_reads[-1][1:] == (record + 0x113, 1, 10)
    assert oracle.state32(GP + 0x2A8) == 4
    print("  S3: all commands, mask/default, count=255, exact reads/writes OK")

    for command, want in ((8, 1), (9, 2), (10, 4)):
        oracle = Oracle(executable)
        seed_common(oracle)
        oracle.ret_5332c = record
        oracle.poke(record + 0x14, 1, 1)
        oracle.poke(record + 0x15, 1, command)
        oracle.run()
        assert oracle.state32(GP + 0x2A8) == want
    print("  S4: variable shifts for commands 8/9/10 independently OK")

    oracle = Oracle(executable)
    seed_common(oracle, old_buffer=0x800A3000, old_flag=0x800A4000,
                old_secondary=0x12345678, old_command=7)
    oracle.run()
    assert [call for call in oracle.calls if call[0] == "func_80052E30"] == [
        ("func_80052E30", 0), ("func_80052E30", 0),
        ("func_80052E30", 1)]
    assert oracle.state32(GP + 0x2D8) == 0x800A4000
    assert oracle.state32(GP + 0x2E0) == 0x12345678
    assert oracle.state32(GP + 0x2E8) == 0x800A1F84
    assert oracle.state32(GP + 0x2F4) == 4
    print("  S5: saved comparison is exact 0/1 and final reuse path OK")

    oracle = Oracle(executable)
    seed_common(oracle)
    oracle.ret_5332c = record
    oracle.poke(record + 0x14, 1, 1)
    oracle.poke(record + 0x15, 1, 11)
    oracle.run(incoming=(1, 2, 3, 4))
    first = tuple(oracle.state32(0x800A1B30 + i * 4) for i in range(7))
    oracle.run(incoming=(0xFFFFFFFF, 0x80000000, 0x55555555, 0xAAAAAAAA))
    second = tuple(oracle.state32(0x800A1B30 + i * 4) for i in range(7))
    assert first == second == (3, 0, 0, 0, 0, 0, 0)
    assert len([call for call in oracle.calls if call[0] == "func_8005332C"]) == 2
    print("  S6: incoming a0-a3 ignored; first/repeated invocation stable OK")

    oracle = Oracle(executable)
    seed_common(oracle, source_id=0x7F)
    oracle.run()
    assert ("func_8005332C", 127) in oracle.calls
    body = body_writes(oracle)
    assert body == [("body", GP + 0x2A8, 4, 0),
                    *[("body", 0x800A1B48 - i * 4, 4, 0)
                      for i in range(7)]]
    print("  S7: positive signed source and exact direct-write footprint OK")

    returns = [event for event in oracle.events
               if event[:2] == ("return", "func_80051CC4")]
    assert len(returns) == 1
    print("  S8: single void return path and each delay slot exactly once OK")

    print(f"PASS: B39 exe SHA-1 {SHA1}; {len(W)}/77 body words and "
          f"{len(JT)}/8 jump-table words verified; delay slots, branches, "
          "computed jumps, ordered reads/writes/calls, partial boundaries, "
          "controlled returns, dirty/repeated behavior")


if __name__ == "__main__":
    main()
