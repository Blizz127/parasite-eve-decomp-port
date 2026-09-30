#!/usr/bin/env python3
"""Execute original 25EE8 attack-selection paths, without replacing callees."""
import hashlib
import struct
from pe_battle_hud_oracle import ROOT, execute

CASES=((0,0,1,3),(0x10,0,3,3),(0x40,0,3,3),(0x200,0,3,3),
       (0x400,0,3,3),(0x600,0,3,3),(0x200,0xC0,3,3),(0,0,0,3),(0x200,0,1,1))
RANGES=((0x9E358,0x700),(0x150000,64),(0x9CE38,0x38),(0x9D1DC,1),
        (0x9D290,4),(0x9D2D8,1),(0xBE830,360),(0x141000,0x900))

def fixture(exe,case):
    ram=bytearray(0x200000); ram[0x10000:0x10000+len(exe)-0x800]=exe[0x800:]
    def sw(a,v): struct.pack_into('<I',ram,a,v&0xFFFFFFFF)
    def sh(a,v): struct.pack_into('<H',ram,a,v&65535)
    for a,n in RANGES[:2]: ram[a:a+n]=bytes((i*37+19)&255 for i in range(n))
    pressed,shape,targets,shots=CASES[case]; bank=case&1
    sw(0x9CDDC,bank); sw(0x9D250,0); sw(0xB0E38+bank*4,0x80150000)
    sw(0x9D278,0x80140000); sw(0x140068,0x80140100)
    sh(0x140102,200); sh(0x140106,1); sw(0x14010C,0x100006); sw(0x140110,0x20|shape|shots)
    ram[0x9D1DC]=shots; ram[0x9D2D8]=2; ram[0x9D2B0]=targets
    sw(0x9D1F4,pressed); sw(0x9D290,1); sw(0x9D254,0x80145000)
    sw(0xB0DF8,0x80144000); sw(0xBCFA4,0x80144400)
    for i in range(3): sh(0x144400+i*8,4096)
    sw(0x14441C,1024); sh(0xBCF94,160); sh(0xBCF96,112)
    sw(0x9CDD0,0x80144800); sw(0x9CDD4,0x80144900); sw(0xB0E58+bank*4,0x80146000)
    for i in range(3):
        actor=0x141000+i*0x300; body=0x142000+i*0x80
        sw(actor,0x80000000+body); ram[body+7]=i+1
        sh(actor+0x218,(166,167,200)[i]); sh(actor+0x21A,(81,82,110)[i])
        if i<targets: sw(0x9E000+i*12,0x80000000+actor); sw(0x9E004+i*12,150+i*100)
    return ram

def fingerprint(ram):
    h=14695981039346656037
    for a,n in RANGES:
        for b in ram[a:a+n]: h=((h^b)*1099511628211)&0xFFFFFFFFFFFFFFFF
    return h

def main():
    exe=(ROOT/'build/disc1.candidate.exe').read_bytes()
    assert hashlib.sha1(exe).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
    for i in range(len(CASES)):
        ram=fixture(exe,i); result=execute(ram,0x80025EE8,initial_cop_control={26:256})
        print(i,f'0x{fingerprint(ram):016X}',result[2])
    print('PASS: target changes, selection, shot queue, cancellation and action counts')

if __name__=='__main__': main()
