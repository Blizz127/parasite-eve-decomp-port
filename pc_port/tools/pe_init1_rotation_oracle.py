#!/usr/bin/env python3
"""Execute authenticated retail 79754 words over synthetic trig inputs.
No production code imports; fixtures deliberately include signed and mixed angles.
"""
import hashlib
import pathlib
import struct

ROOT = pathlib.Path(__file__).resolve().parents[2]
START, END = 0x80079754, 0x800799E0
TRIG = {0: 0x10000000, 1: 0x0B500B50, 2: 0x0DDB0800,
        3: 0x08000DDB, 1024: 0x00001000}
CASES = [(0, 0, 0), (1024, 0, 0), (0, -1024, 0), (1024, 1024, 0),
         (1, -2, 3), (-1, 2, -3), (-32768, 1, 2)]


def signed(v):
    v &= 0xFFFFFFFF
    return v - 0x100000000 if v & 0x80000000 else v


def execute(exe, angles):
    ram = bytearray(0x200000)
    code = exe[START - 0x80010000 + 0x800:END - 0x80010000 + 0x800]
    ram[START & 0x1FFFFF:END & 0x1FFFFF] = code
    for index, word in TRIG.items():
        struct.pack_into('<I', ram, 0x966EC + index * 4, word)
    struct.pack_into('<hhh', ram, 0x130000, *angles)
    ram[0x140000:0x140020] = b'\xA5' * 32
    regs = [0] * 32
    regs[4], regs[5], regs[31] = 0x80130000, 0x80140000, 0xDEAD0000
    lo = 0

    def step(pc):
        nonlocal lo
        word = struct.unpack_from('<I', ram, pc & 0x1FFFFF)[0]
        op, rs, rt, rd = word >> 26, word >> 21 & 31, word >> 16 & 31, word >> 11 & 31
        imm = word & 65535
        simm = imm if imm < 32768 else imm - 65536
        addr = (regs[rs] + simm) & 0x1FFFFF
        jump = None
        if op == 0:
            fn, sh = word & 63, word >> 6 & 31
            if fn == 0: regs[rd] = regs[rt] << sh
            elif fn == 3: regs[rd] = signed(regs[rt]) >> sh
            elif fn == 8: jump = regs[rs]
            elif fn == 0x21: regs[rd] = regs[rs] + regs[rt]
            elif fn == 0x23: regs[rd] = regs[rs] - regs[rt]
            elif fn == 0x19: lo = (regs[rs] * regs[rt]) & 0xFFFFFFFF
            elif fn == 0x12: regs[rd] = lo
            else: raise AssertionError(hex(word))
        elif op == 1:
            assert rt == 1
            jump = pc + 4 + simm * 4 if signed(regs[rs]) >= 0 else pc + 8
        elif op == 2: jump = (pc & 0xF0000000) | ((word & 0x3FFFFFF) << 2)
        elif op == 0x0C: regs[rt] = regs[rs] & imm
        elif op == 0x0F: regs[rt] = imm << 16
        elif op == 0x21: regs[rt] = struct.unpack_from('<h', ram, addr)[0]
        elif op == 0x23: regs[rt] = struct.unpack_from('<I', ram, addr)[0]
        elif op == 0x29: struct.pack_into('<H', ram, addr, regs[rt] & 65535)
        else: raise AssertionError(hex(word))
        regs[:] = [r & 0xFFFFFFFF for r in regs]
        regs[0] = 0
        return jump

    pc = START
    for _ in range(1000):
        target = step(pc)
        if target is not None:
            assert step(pc + 4) is None
            pc = target
        else:
            pc += 4
        if pc == 0xDEAD0000:
            assert ram[0x140012:0x140020] == b'\xA5' * 14
            return struct.unpack_from('<9h', ram, 0x140000)
    raise AssertionError('instruction budget')


def main():
    exe = (ROOT / 'build/disc1.candidate.exe').read_bytes()
    assert hashlib.sha1(exe).hexdigest() == '452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
    for angles in CASES:
        print(angles, execute(exe, angles))
    print('PASS: seven retail 79754 executions; preserved padding/translations')


if __name__ == '__main__':
    main()
