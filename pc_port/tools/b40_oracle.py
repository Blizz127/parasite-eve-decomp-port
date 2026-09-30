#!/usr/bin/env python3
"""Independent Phase 6E-B40 MIPS-I oracle for func_8005332C.

W is the literal complete 42-word retail body.  It is compared word-for-word
with the SHA-1-exact executable before this delay-slot-aware interpreter runs
it.  The 46 direct executable call sites and all eight callback-mediated
``jalr`` sites are also checked from literal words.  Production C is never
loaded or called.
"""
import hashlib
import struct
import sys
from pe_exe_words import exe_words

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
BASE = 0x8005332C
GP = 0x8009CD70
RAM_BASE = 0x80000000
RAM_END = 0x80200000
STACK = 0x801FFF00
RETURN = 0xDEADC0DE
MAX_STEPS = 128

# func_8005332C, executable 0x8005332C..0x800533D3, file 0x43B2C.
W = exe_words(0x8005332C, 42)

# address, caller, preceding word, jal, delay slot, first return consumer.
DIRECT_CALLS = [
    (0x800431A8, "func_800430A0", 0x00008821, 0x0C014CCB, 0x02002021, 0x90420005),
    (0x8004338C, "func_8004324C", 0x80840E20, 0x0C014CCB, 0x00000000, 0x00409021),
    (0x800444C4, "func_80044444", 0x00408821, 0x0C014CCB, 0x02202021, 0x02202021),
    (0x80044618, "func_80044444", 0x00000000, 0x0C014CCB, 0x02202021, 0x10400017),
    (0x80044A80, "func_80044924", 0x8F900194, 0x0C014CCB, 0x02002021, 0x02002021),
    (0x80044BC4, "func_80044B0C", 0x8F900194, 0x0C014CCB, 0x02002021, 0x02002021),
    (0x80045224, "func_800451D0", 0x8F84021C, 0x0C014CCB, 0x00000000, 0x10400017),
    (0x80045BC4, "func_80045A98", 0x80420E22, 0x0C014CCB, 0x00402021, 0x24040002),
    (0x80045BF4, "func_80045A98", 0x24040001, 0x0C014CCB, 0x00402021, 0x02202021),
    (0x80046054, "func_80045FA4", 0x00002021, 0x0C014CCB, 0x00402021, 0x02402021),
    (0x8004677C, "func_800466C0", 0x00002821, 0x0C014CCB, 0x02802021, 0x90420005),
    (0x80047948, "func_8004790C", 0x00000000, 0x0C014CCB, 0x02002021, 0x8F8301B8),
    (0x80047C6C, "func_80047C50", 0x00408021, 0x0C014CCB, 0x02002021, 0x24040004),
    (0x80047EF4, "func_80047E94", 0x24040001, 0x0C014CCB, 0x00402021, 0x90450014),
    (0x8004817C, "func_800480AC", 0x82040000, 0x0C014CCB, 0x00000000, 0x00402021),
    (0x800482F4, "func_80048254", 0x02202021, 0x0C014CCB, 0x00402021, 0x00521021),
    (0x800484E0, "func_80048254", 0x02202021, 0x0C014CCB, 0x00402021, 0x00521021),
    (0x8004894C, "func_80048918", 0xAFB3001C, 0x0C014CCB, 0xAFB10014, 0x8F83019C),
    (0x800495E8, "func_800494AC", 0x00409821, 0x0C014CCB, 0x02602021, 0x8F830254),
    (0x800497CC, "func_800494AC", 0x00008821, 0x0C014CCB, 0x02002021, 0x90420005),
    (0x80049BE0, "func_800494AC", 0x00002021, 0x0C014CCB, 0x00402021, 0x3C06800A),
    (0x8004A1DC, "func_8004A0C8", 0x2405FFFC, 0x0C014CCB, 0x02002021, 0x00408821),
    (0x8004A5E8, "func_8004A570", 0x00002021, 0x0C014CCB, 0x00402021, 0x3C06800A),
    (0x8004A9C4, "func_8004A9A0", 0xAFB1004C, 0x0C014CCB, 0x00402021, 0x00408821),
    (0x8004AD04, "func_8004A9A0", 0x80840E20, 0x0C014CCB, 0x00000000, 0x12220007),
    (0x8004AD1C, "func_8004A9A0", 0x80840E22, 0x0C014CCB, 0x00000000, 0x16220009),
    (0x8004C738, "func_8004C608", 0x80840E22, 0x0C014CCB, 0x00000000, 0x10400133),
    (0x8004C76C, "func_8004C608", 0x00000000, 0x0C014CCB, 0x02002021, 0x10400126),
    (0x8004C7A8, "func_8004C608", 0x02002021, 0x0C014CCB, 0x00402021, 0x10400117),
    (0x8004C87C, "func_8004C608", 0x00408021, 0x0C014CCB, 0x02002021, 0x104000E2),
    (0x8004C904, "func_8004C608", 0x02002021, 0x0C014CCB, 0x00402021, 0x104000C0),
    (0x8004C930, "func_8004C608", 0x00002021, 0x0C014CCB, 0x02002021, 0x104000B5),
    (0x8004C9A0, "func_8004C608", 0x24040001, 0x0C014CCB, 0x00402021, 0x00501021),
    (0x8004DF04, "func_8004DD64", 0x2442DF74, 0x0C014CCB, 0xAE020030, 0xAF820294),
    (0x8004F9B8, "func_8004F9A0", 0x00002021, 0x0C014CCB, 0x00402021, 0xAF8201B0),
    (0x8004FAB0, "func_8004FA10", 0x24040001, 0x0C014CCB, 0x00402021, 0xAF8201B0),
    (0x8004FB10, "func_8004FAF8", 0x24040001, 0x0C014CCB, 0x00402021, 0x02002021),
    (0x80050400, "func_8005033C", 0x00000000, 0x0C014CCB, 0x02002021, 0x90430014),
    (0x800508FC, "func_80050878", 0x8F900194, 0x0C014CCB, 0x02002021, 0x02002021),
    (0x80051574, "func_80051510", 0x80840E20, 0x0C014CCB, 0x00000000, 0x00401821),
    (0x80051724, "func_800516B4", 0x80840E20, 0x0C014CCB, 0x00000000, 0x00401821),
    (0x800517E8, "func_80051770", 0x80840E20, 0x0C014CCB, 0x00000000, 0x00401821),
    (0x80051914, "func_800518A8", 0x80840E20, 0x0C014CCB, 0x00000000, 0x00401821),
    (0x800519A0, "func_80051980", 0x80840E20, 0x0C014CCB, 0x00000000, 0x00403821),
    (0x80051D08, "func_80051CC4", 0x80840E22, 0x0C014CCB, 0x00000000, 0x00403021),
    (0x80051E88, "func_80051E64", 0x80840E22, 0x0C014CCB, 0x00000000, 0x00403821),
]

# The callback slot can mediate calls to this ABI, but all three executable
# writers install func_800532B4, not func_8005332C.  The target-address pair
# is used only by func_8005AFFC's equality check.
INDIRECT_WORDS = [
    (0x8005B000, 0x3C028005), (0x8005B004, 0x2442332C),
    (0x8005B034, 0x0060F809), (0x8005B038, 0x00000000),
    (0x8005B048, 0x0060F809), (0x8005B04C, 0x00408021),
    (0x8005B14C, 0x0040F809), (0x8005B150, 0x00008821),
    (0x8005B160, 0x0060F809), (0x8005B164, 0x00408021),
    (0x8005B294, 0x0040F809), (0x8005B298, 0x00042403),
    (0x8005B31C, 0x0040F809), (0x8005B320, 0x00042403),
    (0x8005B3F0, 0x0040F809), (0x8005B3F4, 0x00042403),
    (0x8005B478, 0x0040F809), (0x8005B47C, 0x00042403),
    (0x8005B628, 0xAF820344), (0x8005B78C, 0xAF820344),
    (0x8005B844, 0xAF820344),
]


def u32(value):
    return value & 0xFFFFFFFF


def s32(value):
    value = u32(value)
    return value - 0x100000000 if value & 0x80000000 else value


class Oracle:
    def __init__(self, executable, dependency_return=0):
        data = open(executable, "rb").read()
        digest = hashlib.sha1(data).hexdigest()
        if digest != SHA1:
            raise SystemExit(f"FATAL: executable SHA-1 {digest} != {SHA1}")
        self.data = data
        self.taddr = struct.unpack_from("<I", data, 0x18)[0]
        self._verify_words(BASE, W, "func_8005332C")
        for address, caller, pre, jal, delay, post in DIRECT_CALLS:
            self._verify_words(address - 4, [pre, jal, delay, post],
                               f"{caller} call {address:08X}")
        for address, word in INDIRECT_WORDS:
            self._verify_words(address, [word], f"indirect word {address:08X}")

        self.ram = bytearray(RAM_END - RAM_BASE)
        self.r = [0] * 32
        self.r[28] = GP
        self.r[29] = STACK
        self.r[31] = RETURN
        self.dependency_return = u32(dependency_return)
        self.reads = []
        self.writes = []
        self.calls = []
        self.events = []
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

    def load(self, address, width, signed=False):
        address = u32(address)
        offset = self.off(address, width)
        value = int.from_bytes(self.ram[offset:offset + width], "little")
        if signed and value & (1 << (width * 8 - 1)):
            value -= 1 << (width * 8)
        value = u32(value)
        self.reads.append((address, width, value))
        self.events.append(("read", address, width, value))
        return value

    def store(self, address, width, value):
        address = u32(address)
        offset = self.off(address, width)
        mask = (1 << (width * 8)) - 1
        value &= mask
        self.ram[offset:offset + width] = value.to_bytes(width, "little")
        self.writes.append((address, width, value))
        self.events.append(("write", address, width, value))

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
            elif function == 0x21:     # addu
                r[rd] = u32(r[rs] + r[rt])
            elif function == 0x2A:     # slt
                r[rd] = int(s32(r[rs]) < s32(r[rt]))
            elif function == 0x08:     # jr (handled by run)
                return
            else:
                raise SystemExit(f"FATAL: SPECIAL {function:02X} @{address:08X}")
        elif op == 0x09:               # addiu
            r[rt] = u32(r[rs] + simm)
        elif op == 0x0B:               # sltiu (sign-extended immediate)
            r[rt] = int(u32(r[rs]) < u32(simm))
        elif op == 0x0F:               # lui
            r[rt] = u32(immediate << 16)
        elif op == 0x21:               # lh, bytewise little-endian model
            r[rt] = self.load(u32(r[rs] + simm), 2, True)
        elif op == 0x23:               # lw
            r[rt] = self.load(u32(r[rs] + simm), 4)
        elif op == 0x2B:               # sw
            self.store(u32(r[rs] + simm), 4, r[rt])
        else:
            raise SystemExit(f"FATAL: opcode {op:02X} @{address:08X}")
        r[0] = 0

    def run(self, resource_id):
        self.r[4] = u32(resource_id)
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

            if op == 1 and rt == 0:     # bltz, sampled at issue
                self.control_issues += 1
                taken = s32(self.r[rs]) < 0
                delay()
                pc = ((address + 4 + (simm << 2) - BASE) // 4
                      if taken else pc + 2)
                continue
            if op == 4:                 # beq, including beqz
                self.control_issues += 1
                taken = self.r[rs] == self.r[rt]
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
            if op == 3:                 # jal func_8005DB44
                self.control_issues += 1
                target = ((word & 0x03FFFFFF) << 2) | (address & 0xF0000000)
                delay()
                if target != 0x8005DB44:
                    raise SystemExit(f"FATAL: unexpected call {target:08X}")
                args = (u32(self.r[4]),)
                self.calls.append(("func_8005DB44", *args))
                self.events.append(("call", "func_8005DB44", args))
                self.r[2] = self.dependency_return
                self.events.append(("return", "func_8005DB44",
                                    self.dependency_return))
                pc += 2
                continue
            if op == 0 and function == 8:  # jr
                self.control_issues += 1
                target = self.r[rs]
                delay()
                if target != RETURN:
                    raise SystemExit(f"FATAL: unexpected jr {target:08X}")
                self.events.append(("return", "func_8005332C", self.r[2]))
                if self.control_issues != self.delay_executions:
                    raise SystemExit("FATAL: delay-slot execution mismatch")
                return u32(self.r[2])

            self.one(word, address)
            pc += 1


def seed(oracle, resource_id, count, entry, base=0x800C0E48):
    oracle.poke(GP + 0x2D8, 4, base)
    oracle.poke(GP + 0x2E0, 4, count)
    if resource_id >= 0:
        oracle.poke(u32(base + (resource_id << 1)), 2, entry)


def body_reads(oracle):
    return [read for read in oracle.reads
            if not (STACK - 0x18 <= read[0] < STACK)]


def body_writes(oracle):
    return [write for write in oracle.writes
            if not (STACK - 0x18 <= write[0] < STACK)]


def self_tests(executable):
    oracle = Oracle(executable)
    oracle.r[1] = 0xFFFFFFFF
    oracle.r[2] = 0
    oracle.one(0x0022102A, BASE)       # slt v0,at,v0
    assert oracle.r[2] == 1
    oracle.r[1] = 0xFFFFFFFF
    oracle.one(0x2C2200FF, BASE)       # sltiu v0,at,255
    assert oracle.r[2] == 0
    oracle.poke(0x80010001, 2, 0x8001)
    oracle.r[1] = 0x80010001
    oracle.one(0x84220000, BASE)       # unaligned lh v0,0(at)
    assert oracle.r[2] == 0xFFFF8001

    oracle = Oracle(executable)
    seed(oracle, -1, 1, 0)
    assert oracle.run(-1) == 0
    assert body_reads(oracle) == []
    assert oracle.control_issues == oracle.delay_executions
    print("  interpreter self-tests: signed slt/bltz, sltiu, little-endian unaligned lh, delays OK")


def main():
    if len(sys.argv) != 2:
        raise SystemExit("usage: b40_oracle.py executable")
    executable = sys.argv[1]
    self_tests(executable)

    for resource_id, count in ((0, 0), (1, 1), (0, 0x80000000),
                               (0x7FFFFFFF, 0x7FFFFFFF)):
        oracle = Oracle(executable)
        oracle.poke(GP + 0x2D8, 4, 0x12345678)
        oracle.poke(GP + 0x2E0, 4, count)
        assert oracle.run(resource_id) == 0
        assert oracle.calls == []
    print("  S1: negative/equal/out-of-range and signed-count return paths OK")

    for entry, want in ((0x100, 0x800C0EAC), (0x17F, 0x800C1E8C)):
        oracle = Oracle(executable)
        seed(oracle, 0, 1, entry)
        assert oracle.run(0) == want
        assert body_reads(oracle) == [
            (GP + 0x2E0, 4, 1), (GP + 0x2D8, 4, 0x800C0E48),
            (0x800C0E48, 2, entry)]
        assert body_writes(oracle) == []
    print("  S2: direct 0x100..0x17F static-record boundaries and read order OK")

    for entry, dep_arg, dep_ret in ((1, 0, 0), (1, 0, 0x801FFFF0),
                                    (0xFF, 0xFE, 0x800A8123)):
        oracle = Oracle(executable, dep_ret)
        seed(oracle, 0, 1, entry)
        assert oracle.run(0) == dep_ret
        assert oracle.calls == [("func_8005DB44", dep_arg)]
        returns = [event for event in oracle.events if event[0] == "return"]
        assert returns == [
            ("return", "func_8005DB44", dep_ret),
            ("return", "func_8005332C", dep_ret)]
    print("  S3: delegated 1..255 arguments, NULL/high controlled returns, forwarding OK")

    for entry, want in ((0x200, 0x800A1E64), (0x208, 0x800A1F64)):
        oracle = Oracle(executable)
        seed(oracle, 0, 1, entry)
        assert oracle.run(0) == want
        assert oracle.calls == []
    print("  S4: alternate 0x200..0x208 static-record boundaries OK")

    for entry in (0, -1, -0x8000, 0x180, 0x1FF, 0x209, 0x7FFF):
        oracle = Oracle(executable)
        seed(oracle, 0, 1, entry)
        assert oracle.run(0) == 0
        assert oracle.calls == []
    print("  S5: all invalid halfword classes and signed extension return NULL OK")

    oracle = Oracle(executable)
    seed(oracle, 4, 5, 0x101, base=0x800C1000)
    first = oracle.run(4)
    second = oracle.run(4)
    assert first == second == 0x800C0ECC
    assert body_writes(oracle) == []
    assert len([event for event in oracle.events
                if event[:2] == ("return", "func_8005332C")]) == 2
    print("  S6: last valid index and repeated invocation are stable/read-only OK")

    assert len(DIRECT_CALLS) == 46
    assert len({caller for _, caller, *_ in DIRECT_CALLS}) == 33
    assert len([row for row in INDIRECT_WORDS
                if row[1] in (0x0040F809, 0x0060F809)]) == 8
    print("  S7: 46 direct sites/33 callers and eight callback jalr sites verified")

    print(f"PASS: B40 exe SHA-1 {SHA1}; {len(W)}/42 body words, "
          f"{len(DIRECT_CALLS)}/46 direct call sites, callback words, "
          "delay slots, signed/unsigned branches, ordered reads/calls/returns, "
          "controlled dependency returns, NULL/direct/delegated/alternate paths")


if __name__ == "__main__":
    main()
