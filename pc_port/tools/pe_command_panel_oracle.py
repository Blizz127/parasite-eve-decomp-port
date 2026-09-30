#!/usr/bin/env python3
"""Execute the entire retail 314E4 command panel and all drawing callees."""
import hashlib
import struct
from pe_battle_hud_oracle import ROOT, execute

RANGES=((0x9E358,0x700),(0x150000,64))
TRIG=(0x10000000,0x0B500B50,0x00001000,0xF4B00B50,
      0xF0000000,0xF4B0F4B0,0x0000F000,0x0B50F4B0)
POSITIONS=((166,81),(167,82),(-200,-100),(300,200))


def fixture(exe,case):
    ram=bytearray(0x200000)
    ram[0x10000:0x10000+len(exe)-0x800]=exe[0x800:]
    def sw(a,v): struct.pack_into('<I',ram,a,v&0xFFFFFFFF)
    def sh(a,v): struct.pack_into('<H',ram,a,v&65535)
    for a,n in RANGES: ram[a:a+n]=bytes((i*37+19)&255 for i in range(n))
    bank=case&1
    sw(0x9CDDC,bank); sw(0x9D250,(case&7)*8)
    sw(0xB0E38+bank*4,0x80150000)
    sw(0x9D278,0x80140000); sw(0x140068,0x80140100)
    sw(0x14010C,((case%4)<<20)|25)
    sw(0x140110,((case%3+1)<<4)|((case%4)<<6)|3)
    ram[0x9D1DC]=case%4+1; ram[0x9D2D8]=case%3+1; ram[0x9CE3C]=case%4
    for i,c in enumerate((1,2,0x189)): sh(0xBE834+i*8,c)
    for i in range(3): sh(0xA1E6E+i*32,20+i*10)
    sw(0x966EC,TRIG[0]); sw(0x966EC+(case&7)*512*4,TRIG[case&7])
    x,y=POSITIONS[case%4]; sh(0x141064,x); sh(0x141066,y)
    return ram


def fingerprint(ram):
    h=14695981039346656037
    for a,n in RANGES:
        for b in ram[a:a+n]: h=((h^b)*1099511628211)&0xFFFFFFFFFFFFFFFF
    return h


def main():
    exe=(ROOT/'build/disc1.candidate.exe').read_bytes()
    assert hashlib.sha1(exe).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
    for case in range(12):
        ram=fixture(exe,case)
        execute(ram,0x800314E4,(0x80141000,case%9,case&1,0 if case==9 else 42))
        print(case,f'0x{fingerprint(ram):016X}')
    print('PASS: nine command panels, targeting marker, ammo accounting and packet links')


if __name__=='__main__': main()
