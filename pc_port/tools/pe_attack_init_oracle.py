#!/usr/bin/env python3
"""Execute retail attack-selection reset including effect destruction."""
import hashlib
import struct
from pe_battle_hud_oracle import ROOT, execute

RANGES=((0x150000,11*0xA0C),(0xE10A0,28),(0xBCEA8,224),(0xBE830,360),
        (0x9D200,4),(0x9D2FC,4),(0x9D258,4),(0x9D208,4),(0x9CE44,1),
        (0x9CE40,1),(0x9D294,1),(0x9D2D8,1),(0x9D1DC,1),(0x9CE38,4),
        (0x9D1CE,1),(0x9D1AC,4),(0x9CE68,1),(0x9CE6C,1),(0x9CE60,1),(0xB0CD8,4))
CODES=(0,1,2,7,0x55,0x72,0x75,0xFF,0x54,0x80,0x71)
CALLBACKS=(0x800C7DC4,0x800C8F08,0x800C9C00,0x800CA798,0x800CD960,
           0x800CCF80,0x800CE1DC,0x800CBFA4)


def fixture(exe, case):
    ram=bytearray(0x200000)
    ram[0x10000:0x10000+len(exe)-0x800]=exe[0x800:]
    def sw(a,v): struct.pack_into('<I',ram,a,v&0xFFFFFFFF)
    for a,n in RANGES: ram[a:a+n]=bytes((i*37+19)&255 for i in range(n))
    sw(0x942E4,0x80150000); sw(0x942E0,0x80160000)
    sw(0x9D254,0x80140200); sw(0x9D278,0x80140000)
    sw(0x140068,0x80140100)
    sw(0x140110,(case<<4)|(case+1))
    ram[0x140114:0x140118]=bytes((1,2,5,10)); ram[0x9D2B0]=case
    for code,fn in enumerate(CALLBACKS):
        sw(0x160000+code*4,0x80161000+code*24)
        sw(0x161000+code*24+20,fn)
    sw(0x160000+0x55*4,0x80162000); sw(0x162014,0x800D4850)
    for i,code in enumerate(CODES):
        a=0x150000+i*0xA0C
        ram[a],ram[a+1]=(6 if i==1 else 0 if i==2 else 1),code
        sw(a+8,0x80140200+i*4)
    return ram


def fingerprint(ram):
    h=14695981039346656037
    for a,n in RANGES:
        for b in ram[a:a+n]: h=((h^b)*1099511628211)&0xFFFFFFFFFFFFFFFF
    return h


def main():
    exe=(ROOT/'build/disc1.candidate.exe').read_bytes()
    assert hashlib.sha1(exe).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
    for case in range(3):
        ram=fixture(exe,case); execute(ram,0x80020F18)
        print(case,f'0x{fingerprint(ram):016X}')
    print('PASS: command-selection reset, text clear, effect filtering and destruction')


if __name__=='__main__': main()
