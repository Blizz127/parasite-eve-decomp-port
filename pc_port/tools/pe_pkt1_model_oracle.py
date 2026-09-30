#!/usr/bin/env python3
"""Execute retail packet construction and page relocation, independent of C."""
import hashlib
from pathlib import Path
import struct
ROOT = Path(__file__).resolve().parents[2]

def signed(x):
    x &= 0xffffffff
    return x - 0x100000000 if x & 0x80000000 else x

def run(ram, regs, pc, end):
    def step(pc):
        w = struct.unpack_from('<I', ram, pc & 0x1fffff)[0]
        op, rs, rt, rd, sh = w >> 26, w >> 21 & 31, w >> 16 & 31, w >> 11 & 31, w >> 6 & 31
        imm = w & 65535
        si = imm if imm < 32768 else imm - 65536
        a = (regs[rs] + si) & 0x1fffff
        jump = None
        if op == 0:
            fn = w & 63
            if fn == 0: regs[rd] = regs[rt] << sh
            elif fn == 2: regs[rd] = regs[rt] >> sh
            elif fn == 3: regs[rd] = signed(regs[rt]) >> sh
            elif fn == 8: jump = regs[rs]
            elif fn == 0x21: regs[rd] = regs[rs] + regs[rt]
            elif fn == 0x23: regs[rd] = regs[rs] - regs[rt]
            elif fn == 0x2a: regs[rd] = int(signed(regs[rs]) < signed(regs[rt]))
            else: raise AssertionError(hex(w))
        elif op == 2: jump = (pc & 0xf0000000) | (w & 0x3ffffff) << 2
        elif op in (4,5,6):
            take = regs[rs] == regs[rt] if op == 4 else regs[rs] != regs[rt] if op == 5 else signed(regs[rs]) <= 0
            jump = pc + 4 + si * 4 if take else pc + 8
        elif op == 9: regs[rt] = regs[rs] + si
        elif op == 10: regs[rt] = int(signed(regs[rs]) < si)
        elif op == 11: regs[rt] = int(regs[rs] < (si & 0xffffffff))
        elif op == 12: regs[rt] = regs[rs] & imm
        elif op == 14: regs[rt] = regs[rs] ^ imm
        elif op in (0x23,0x24,0x25): regs[rt] = struct.unpack_from({0x23:'<I',0x24:'<B',0x25:'<H'}[op],ram,a)[0]
        elif op in (0x28,0x29,0x2b):
            fmt,mask = {0x28:('<B',255),0x29:('<H',65535),0x2b:('<I',0xffffffff)}[op]
            struct.pack_into(fmt,ram,a,regs[rt]&mask)
        else: raise AssertionError(hex(w))
        regs[:] = [x & 0xffffffff for x in regs]
        regs[0] = 0
        return jump
    for _ in range(100000):
        if pc == end: return
        j = step(pc)
        if j is None: pc += 4
        else:
            assert step(pc+4) is None
            pc = j
    raise AssertionError('budget')

def fixture(exe, enabled=1):
    ram = bytearray(0x200000)
    ram[0x10000:0x10000+len(exe)-0x800] = exe[0x800:]
    def sh(a,v): struct.pack_into('<H',ram,a,v)
    def sw(a,v): struct.pack_into('<I',ram,a,v)
    for k in range(4):
        sh(0x130008+2*k,2)
        ram[0x13001f+24*k] = (11,16,21,26)[k]
    sh(0x130018,5)
    for i in range(56): ram[0x13009c+i] = (i*37+19)&255
    ram[0x140000:0x140800] = b'\xa5'*2048
    regs=[0]*32
    regs[16],regs[12],regs[5],regs[6],regs[29] = 0x80120000,0x80130000,0x80130000,0x80140000,0x80110000
    sw(0x11005c,enabled);sw(0x110058,0x80110100)
    run(ram,regs,0x8003d078,0x8003d5d8)
    return ram,regs[17]

def fnv(data):
    h=14695981039346656037
    for b in data: h=((h^b)*1099511628211)&0xffffffffffffffff
    return h

def main():
    exe=(ROOT/'build/disc1.candidate.exe').read_bytes()
    assert hashlib.sha1(exe).hexdigest() == '452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
    for enabled in (0,1):
        ram,end=fixture(exe,enabled)
        print('construct',enabled,hex(end),hex(fnv(ram[0x140000:0x140800])))
    for x,y,clut in ((0x3c0,256,448),(0x200,128,450),(0x280,0,448),(-64,-128,-1)):
        ram,_=fixture(exe)
        regs=[0]*32
        regs[4:8]=[0x80120000,x&0xffffffff,y&0xffffffff,0]
        regs[29]=0x80110000
        struct.pack_into('<I',ram,0x110010,clut&0xffffffff)
        run(ram,regs,0x8003d94c,0)
        print('relocate',x,y,clut,hex(fnv(ram[0x140000:0x140800])))
    ram,_=fixture(exe)
    struct.pack_into('<H',ram,0x14001a,31)
    before=ram[:]
    regs=[0]*32;regs[4:8]=[0x80120000,0x3c0,128,0];regs[29]=0x80110000
    run(ram,regs,0x8003d94c,0)
    assert ram==before
    print('PASS retail sentinel leaves all RAM unchanged')
if __name__=='__main__': main()
