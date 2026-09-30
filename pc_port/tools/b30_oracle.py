#!/usr/bin/env python3
"""Independent Phase 6E-B30 oracle for func_80042C78."""
import hashlib
import struct
import sys
from pe_exe_words import exe_words

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
BASE = 0x80042C78
GP = 0x8009CD70
RAM_BASE = 0x80000000
RAM_END = 0x80200000

W = exe_words(0x80042C78, 16)

def u32(x): return x & 0xFFFFFFFF

class Oracle:
    def __init__(self, path):
        data = open(path, "rb").read()
        got = hashlib.sha1(data).hexdigest()
        if got != SHA1:
            raise SystemExit(f"FATAL: executable SHA-1 {got} != {SHA1}")
        taddr = struct.unpack_from("<I", data, 0x18)[0]
        for i, want in enumerate(W):
            got_word = struct.unpack_from("<I", data,
                                          BASE + 4*i - taddr + 0x800)[0]
            if got_word != want:
                raise SystemExit(f"FATAL: word {i} @0x{BASE+4*i:08X}: "
                                 f"{got_word:08X} != {want:08X}")
        print(f"exe SHA-1 {SHA1} OK; func_80042C78: {len(W)} words verified")
        self.r = [0] * 32
        self.r[28] = GP
        self.r[29] = 0x801FFF00
        self.ram = bytearray(0x200000)
        self.reads = []
        self.writes = []
        self.calls = []

    def off(self, a, n):
        if not (RAM_BASE <= a and a+n <= RAM_END):
            raise SystemExit(f"FATAL: guest access 0x{a:08X} size {n}")
        return a - RAM_BASE

    def load(self, a, n):
        o = self.off(a, n)
        v = int.from_bytes(self.ram[o:o+n], "little")
        self.reads.append((a, n, v))
        return v

    def store(self, a, n, v):
        o = self.off(a, n)
        v = u32(v) & ((1 << (8*n))-1)
        self.ram[o:o+n] = v.to_bytes(n, "little")
        self.writes.append((a, n, v))

    def run(self):
        pc = 0
        while pc < len(W):
            addr = BASE + 4*pc
            w = W[pc]
            op = w >> 26
            rs = (w >> 21) & 31
            rt = (w >> 16) & 31
            fn = w & 63
            imm = w & 0xFFFF
            simm = imm if imm < 0x8000 else imm - 0x10000
            delay = W[pc+1] if pc+1 < len(W) else 0

            def execute(word, word_addr):
                word_op = word >> 26
                word_rs = (word >> 21) & 31
                word_rt = (word >> 16) & 31
                word_rd = (word >> 11) & 31
                word_imm = word & 0xFFFF
                word_simm = word_imm if word_imm < 0x8000 else word_imm - 0x10000
                if word == 0:
                    return
                if word_op == 9:
                    self.r[word_rt] = u32(self.r[word_rs] + word_simm)
                elif word_op == 0x2B:
                    self.store(u32(self.r[word_rs] + word_simm), 4,
                               self.r[word_rt])
                elif word_op == 0x23:
                    self.r[word_rt] = self.load(u32(self.r[word_rs] + word_simm), 4)
                else:
                    raise SystemExit(f"FATAL: opcode {word_op:02X} @{word_addr:08X}")
                self.r[0] = 0

            if op == 3:  # jal
                if addr != 0x80042C98:
                    raise SystemExit(f"FATAL: unexpected call @{addr:08X}")
                execute(delay, addr + 4)
                self.calls.append(("func_80042CC4", self.r[4], self.r[5]))
                pc += 2
                continue
            if op == 0 and fn == 8:  # jr ra
                execute(delay, addr + 4)
                return
            execute(w, addr)
            pc += 1
        raise SystemExit("FATAL: oracle fell off body")

def main():
    if len(sys.argv) != 2:
        raise SystemExit("usage: b30_oracle.py executable")
    o = Oracle(sys.argv[1])
    o.run()
    assert o.calls == [("func_80042CC4", 0x90, 0xFF)]
    assert o.writes == [
        (0x801FFEF8, 4, 0),
        (0x8009CED8, 4, 0), (0x8009CEE0, 4, 0),
        (0x8009CEE4, 4, 0), (0x8009CEDC, 4, 0x20),
        (0x8009CEEC, 4, 0x48),
    ]
    print("PASS: B30 transcription, delay-slot arguments, prefix writes, and return state")

if __name__ == "__main__":
    main()
