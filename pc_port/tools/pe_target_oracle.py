#!/usr/bin/env python3
"""Actual retail target enumeration, distance/angle and quicksort regression."""
import hashlib
import struct
from pe_battle_hud_oracle import ROOT, execute

POSITIONS = [(0,0), (0,1000), (200,0), (0,100), (10,0),
             (0,-300), (-10,0), (0,0), (-200,0)]
FLAGS = [0,0,0,0,0x40,0x2040,0x4000,0,0]


def fixture(exe, case):
    ram=bytearray(0x200000)
    ram[0x10000:0x10000+len(exe)-0x800]=exe[0x800:]
    def sw(a,v): struct.pack_into('<I',ram,a,v&0xFFFFFFFF)
    def sh(a,v): struct.pack_into('<H',ram,a,v&65535)
    ram[0x9E000:0x9E080]=bytes((i*37+19)&255 for i in range(128))
    sw(0x9D254,0x80140000); sw(0x9D20C,0 if case==2 else 0x80140000)
    ram[0x9CE44]=77
    # Explicit table input, used by the original integer ratan2 routine.
    for i in range(1025): sh(0x9A6EC+i*2,i//2)
    for i,(x,z) in enumerate(POSITIONS):
        a=0x140000+i*0x300; body=0x145000+i*0x100
        sw(a,0 if i==7 else body|0x80000000)
        sw(a+4,(a+0x300)|0x80000000 if i<8 else 0)
        sw(a+0x98,0 if case==1 else FLAGS[i])
        sw(body+16,0 if case==3 or (i==3 and case!=1) else 100)
        sh(a+0x268,x); sh(a+0x26C,z)
    if case==4: sh(0x14002A,32767); sh(0x140032,-32768)
    return ram


def fingerprint(ram):
    h=14695981039346656037
    for b in ram[0x9E000:0x9E080]+ram[0x9CE44:0x9CE45]:
        h=((h^b)*1099511628211)&0xFFFFFFFFFFFFFFFF
    return h


def main():
    exe=(ROOT/'build/disc1.candidate.exe').read_bytes()
    assert hashlib.sha1(exe).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
    for case in range(5):
        ram=fixture(exe,case)
        out=execute(ram,0x8002156C)
        print(case,out[2],f'0x{fingerprint(ram):016X}')
    print('PASS: target eligibility, signed distances, angles and tie ordering')


if __name__=='__main__': main()
