#!/usr/bin/env python3
"""Independent Phase 6E-B29 oracle for func_80053D2C.

The 80 instruction words below are an independent transcription.  They are
cross-checked against the SHA-verified executable before the transcription is
executed.  Production C, including func_80053D2C, is never called.
"""
import hashlib
import struct
import sys
from pe_exe_words import exe_words

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
BASE = 0x80053D2C
GP = 0x8009CD70
RAM_BASE = 0x80000000
RAM_END = 0x80200000

W = exe_words(0x80053D2C, 80)

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
            got = struct.unpack_from("<I", data, BASE + i*4 - taddr + 0x800)[0]
            if got != want:
                raise SystemExit(f"FATAL: word {i} @ {BASE+i*4:08X}: "
                                 f"{got:08X} != {want:08X}")
        print(f"exe SHA-1 {got and SHA1} OK; func_80053D2C: {len(W)} words verified")
        self.ram = bytearray(0x200000)
        self.r = [0] * 32
        self.r[28] = GP
        self.r[29] = 0x801FFF00
        self.reads = []
        self.writes = []
        self.calls = []
        self.pc = 0
        self.steps = 0
        self.ret_53968 = 0
        self.ret_53B48 = 0
        self.db44_a1 = 0

    def off(self, a, n):
        if not (RAM_BASE <= a and a+n <= RAM_END):
            raise SystemExit(f"FATAL: guest access {a:08X}/{n}")
        return a - RAM_BASE
    def load(self, a, n, signed=False):
        o = self.off(a, n)
        v = int.from_bytes(self.ram[o:o+n], "little")
        if signed and v & (1 << (n*8-1)): v -= 1 << (n*8)
        self.reads.append((a, n, u32(v)))
        return u32(v)
    def store(self, a, n, v):
        o = self.off(a, n); v = u32(v) & ((1 << (n*8))-1)
        self.ram[o:o+n] = v.to_bytes(n, "little")
        self.writes.append((a, n, v))
    def poke(self, a, n, v):
        o = self.off(a, n); self.ram[o:o+n] = (u32(v) & ((1 << (n*8))-1)).to_bytes(n, "little")

    def one(self, w, addr):
        op=(w>>26)&63; rs=(w>>21)&31; rt=(w>>16)&31; rd=(w>>11)&31
        sa=(w>>6)&31; fn=w&63; imm=w&0xffff; simm=imm if imm<0x8000 else imm-0x10000
        r=self.r
        if w == 0: return
        if op == 0:
            if fn == 0: r[rd]=u32(r[rt]<<sa)
            elif fn == 3: r[rd]=u32(s32(r[rt])>>sa)
            elif fn == 8: pass
            elif fn == 0x21: r[rd]=u32(r[rs]+r[rt])
            elif fn == 0x23: r[rd]=u32(r[rs]-r[rt])
            elif fn == 0x2b: r[rd]=int(u32(r[rs])<u32(r[rt]))
            else: raise SystemExit(f"FATAL: SPECIAL {fn:02X} @{addr:08X}")
        elif op == 9: r[rt]=u32(r[rs]+simm)
        elif op == 0x0a: r[rt]=int(s32(r[rs])<simm)
        elif op == 0x0b: r[rt]=int(u32(r[rs])<u32(simm))
        elif op == 0x0f: r[rt]=u32(imm<<16)
        elif op == 0x20: r[rt]=self.load(u32(r[rs]+simm),1,True)
        elif op == 0x21: r[rt]=self.load(u32(r[rs]+simm),2,True)
        elif op == 0x23: r[rt]=self.load(u32(r[rs]+simm),4)
        elif op == 0x24: r[rt]=self.load(u32(r[rs]+simm),1)
        elif op == 0x29: self.store(u32(r[rs]+simm),2,r[rt])
        elif op == 0x2b: self.store(u32(r[rs]+simm),4,r[rt])
        else: raise SystemExit(f"FATAL: opcode {op:02X} @{addr:08X}")
        r[0]=0

    def run(self, arg, record_type=None, table_words=(0,0,0,0)):
        self.r[4]=u32(arg); self.poke(0x800C0E48, 4, 0x800C0E48)
        for i,v in enumerate(table_words): self.poke(0x800C0E48+2*i,2,v)
        self.poke(0x8009D050,4,len(table_words))  # only oracle backing for lw gp+2E0
        self.poke(0x8009D048,4,0x800C0E48)
        rec=0x800B8100
        db_index=u32(arg-1)
        alt=u32(rec-0x800A8028-(db_index<<5))
        self.db44_a1 = alt
        self.poke(0x800A8034,4,alt)
        self.poke(0x800A8038,4,u32(alt+((db_index+1)<<5)))
        self.poke(rec+6,1,record_type or 0)
        targets = [0x80053DF8]*9 + [0x80053E08, 0x80053E4C]
        targets += [0x80053E08]*4 + [0x80053E1C]*3
        for i, target in enumerate(targets): self.poke(0x8001125C+4*i,4,target)
        self.pc=0; self.steps=0
        while self.pc < len(W):
            if self.steps > 500: raise SystemExit("FATAL: oracle did not terminate")
            self.steps += 1; addr=BASE+self.pc*4; w=W[self.pc]
            op=(w>>26)&63; rs=(w>>21)&31; rt=(w>>16)&31; imm=w&0xffff
            simm=imm if imm<0x8000 else imm-0x10000; fn=w&63
            delay=W[self.pc+1] if self.pc+1<len(W) else 0
            def d(): self.one(delay,addr+4)
            if op==0 and fn==8:
                if addr == 0x80053DF0:
                    d(); self.pc=(self.r[2]-BASE)//4; continue
                d(); return self.r[2]
            if op==2:
                target=((w&0x3ffffff)<<2)|(addr&0xf0000000); d(); self.pc=(target-BASE)//4; continue
            if op in (4,5):
                take=(self.r[rs]==self.r[rt]) if op==4 else (self.r[rs]!=self.r[rt]); d()
                self.pc=((addr+4+(simm<<2)-BASE)//4) if take else self.pc+2; continue
            if op==1 and rt==0: take=s32(self.r[rs])<0; d(); self.pc=((addr+4+(simm<<2)-BASE)//4) if take else self.pc+2; continue
            if op==3 or (op==0 and fn==9):
                target=((w&0x3ffffff)<<2)|(addr&0xf0000000) if op==3 else None
                d()  # a call's delay slot establishes its argument registers
                if addr==0x80053DB4:
                    self.calls.append(("func_8005DB44",self.r[4]))
                    self.r[5]=self.db44_a1; self.r[2]=rec
                elif addr==0x80053DF8:
                    self.calls.append(("func_80053968",self.r[16])); self.r[2]=self.ret_53968
                elif addr==0x80053E1C:
                    self.calls.append(("func_80053B48",tuple(self.r[4:8])))
                    self.r[2]=self.ret_53B48
                else: raise SystemExit(f"FATAL: unexpected call @{addr:08X}")
                self.pc += 2; continue
            self.one(w,addr); self.pc += 1
        raise SystemExit("FATAL: oracle fell off body")

def main():
    if len(sys.argv)!=2: raise SystemExit("usage: b29_oracle.py executable")
    o=Oracle(sys.argv[1])
    assert o.run(0x100, table_words=(0,0)) == 0
    assert o.writes[-1] == (0x800C0E48,2,0x100)
    o=Oracle(sys.argv[1]); assert o.run(0x100, table_words=(7,8)) == 1
    o=Oracle(sys.argv[1]); assert o.run(0x44, record_type=10, table_words=(7,0)) == 0
    assert o.writes[-1] == (0x800C0E4A,2,0x44)
    o=Oracle(sys.argv[1]); o.ret_53968=0; assert o.run(0x44, record_type=1) == 1
    assert o.calls == [("func_8005DB44",0x43),("func_80053968",0x44)]
    for controlled in (1, 0xFFFFFFFF, 7):
        o=Oracle(sys.argv[1]); o.ret_53968=controlled
        assert o.run(0x44, record_type=1) == 0
    for controlled in (0, 1, 7, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF):
        o=Oracle(sys.argv[1]); o.ret_53B48=controlled
        assert o.run(0x44, record_type=16) == controlled
        assert o.calls[-1] == (
            "func_80053B48", (0x800B8100, o.db44_a1, 0, 0))
    print("PASS: B29 independent transcription, delay slots, table dispatch, writes, exact 53968 zero/nonzero consumption, exact 53B48 argument/return forwarding, and boundaries")

if __name__ == "__main__": main()
