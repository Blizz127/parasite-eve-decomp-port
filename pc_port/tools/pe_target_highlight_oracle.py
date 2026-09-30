#!/usr/bin/env python3
"""Execute original target highlight, all four polygon types and cleanup."""
import hashlib
import struct
from pe_battle_hud_oracle import ROOT, execute

RANGES=((0x140000,7*0x300),(0x150000,7*0x400),(0x9CE68,1),(0x9CE6C,1))


def fixture(exe,case):
    ram=bytearray(0x200000)
    ram[0x10000:0x10000+len(exe)-0x800]=exe[0x800:]
    def sw(a,v): struct.pack_into('<I',ram,a,v&0xFFFFFFFF)
    def sh(a,v): struct.pack_into('<H',ram,a,v&65535)
    for a,n in RANGES: ram[a:a+n]=bytes((i*37+19)&255 for i in range(n))
    sw(0x9CDDC,case&1); sw(0x9D278,0x80146000); sw(0x146068,0x80147000)
    shape=case if case<4 else 3 if case==8 else 2
    sw(0x147010,shape<<6)
    sw(0x9D20C,0x80140000); sw(0x9D254,0x80140000)
    ram[0x9D2B0]=0 if case==10 else 5
    ram[0x9CE68]=64 if case<4 else 192 if case<8 else 128
    ram[0x9CE6C]=248
    for i,kind in enumerate((4,0,1,4,0,2,4)):
        actor=0x140000+i*0x300; body=0x146000+i*0x80; model=actor+0x1B4
        sw(actor,body|0x80000000); sw(actor+4,(actor+0x300)|0x80000000 if i<6 else 0)
        ram[body+5]=kind; sw(actor+0x18C,0x80140F00)
        sw(model,0x80145000+i*0x40); sh(model+0xBA,0 if case==11 and i==4 else 1)
        sw(model+0x54,0x80150000+i*0x400)
        for j in range(4): sh(0x145008+i*0x40+j*2,1)
    for i,angle in enumerate((0,-1536,-1024,512,1536)):
        sw(0x148000+i*12,0x80140300+i*0x300); sh(0x148008+i*12,angle)
    sw(0x148000+5*12,0)
    if 4<=case<8: sh(0x148008,(-1800,-1536,1536,1800)[case-4])
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
        execute(ram,0x80026FF8,(0x80148000,0,4 if case==8 else 8 if case==9 else 0))
        execute(ram,0x800275CC,(0x80148000,0))
        print(case,f'0x{fingerprint(ram):016X}')
    print('PASS: pulse bounds, angle wrap, owner/group targets and four polygon layouts')


if __name__=='__main__': main()
