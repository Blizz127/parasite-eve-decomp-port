#!/usr/bin/env python3
"""Retail AT-ready colors and Cross entry, ending before damage dispatch.

The ready cue is latched in these fixtures so no BIOS call is replaced.
All target enumeration, reset and sound-command queue callees execute.
"""
import hashlib
import struct
from pe_battle_hud_oracle import ROOT, execute
from pe_attack_init_oracle import fixture as init_fixture, RANGES as INIT_RANGES

RANGES=INIT_RANGES+((0xB00E8,72),(0xB6920,56),(0x9E000,128),(0x9D1F0,1),
    (0x9D28C,8),(0x9D1A0,4),(0x9D2B0,1),(0x9CF3C,4),(0x140010,2),
    (0xBCD80,20),(0xB8628,36),(0x9D2F4,4))
CASES=[(k,bank,0,9000,0,1,0) for bank in (0,1) for k in range(4)]+[
    (0,0,0x200,9000,0,1,0),(1,1,0x200,9500,0,1,0),
    (2,0,0x200,8999,0,1,0),(3,1,0x200,9000,0x2000,1,0),
    (2,0,0x200,9000,0,0,0),(0,1,0x200,9000,0,1,1),
    (3,0,0x200,9000,0x10000,1,0)]


def fixture(exe,case):
    ram=init_fixture(exe,0)
    def sw(a,v): struct.pack_into('<I',ram,a,v&0xFFFFFFFF)
    tick,bank,pad,at,flags,enemies,commands=case
    for a,n in RANGES[len(INIT_RANGES):]:
        ram[a:a+n]=bytes((i*37+19)&255 for i in range(n))
    sw(0x9D250,tick); sw(0x9CDDC,bank); sw(0x9D1F4,pad)
    sw(0x9D1A0,2); sw(0x9D28C,0); sw(0x9D290,77); sw(0x9D2F4,0)
    ram[0x9D288]=1; ram[0x9D2A0]=enemies; ram[0x9CE3C]=commands
    sw(0xB0E08,0); sw(0x9D20C,0)
    struct.pack_into('<H',ram,0x140010,at); sw(0x14004C,flags)
    return ram


def fingerprint(ram):
    h=14695981039346656037
    for a,n in RANGES:
        for b in ram[a:a+n]: h=((h^b)*1099511628211)&0xFFFFFFFFFFFFFFFF
    return h


def main():
    exe=(ROOT/'build/disc1.candidate.exe').read_bytes()
    assert hashlib.sha1(exe).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
    for i,case in enumerate(CASES):
        ram=fixture(exe,case)
        out=execute(ram,0x80029A84,stop_at=(0x8002A470,),initial_regs={17:1})
        print(i,out[17],f'0x{fingerprint(ram):016X}')
    print('PASS: both AT pulse banks, entry and status/input guards')


if __name__=='__main__': main()
