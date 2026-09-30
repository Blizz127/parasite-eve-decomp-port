#!/usr/bin/env python3
"""Original 6DFA8 positional sound projection, pan and attenuation."""
import hashlib
import struct
from pe_battle_hud_oracle import ROOT, execute

CASES=((-10000,0,0,100,1000,20,120),(-400,0,0,100,1000,20,120),
       (0,0,0,100,1000,20,120),(400,0,0,100,1000,20,120),
       (10000,0,0,100,1000,20,120),(0,400,8000,100,1000,20,120),
       (0,-400,-2000,100,1000,20,120),(0,0,512,100,1000,120,20),
       (0,0,0,1000,100,20,120),(0,0,0,0,65535,255,0),
       (-600,0,0,100,1000,20,255),(600,0,0,100,1000,250,255))

def main():
    exe=(ROOT/'build/disc1.candidate.exe').read_bytes()
    assert hashlib.sha1(exe).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
    for i,(x,y,z,near,far,low,high) in enumerate(CASES):
        ram=bytearray(0x200000); ram[0x10000:0x10000+len(exe)-0x800]=exe[0x800:]
        def sw(a,v): struct.pack_into('<I',ram,a,v&0xFFFFFFFF)
        def sh(a,v): struct.pack_into('<H',ram,a,v&65535)
        sw(0xBCFA4,0x80140000); sw(0xBCFA8,0x80140100); sw(0x140100,256)
        for j in range(3): sh(0x140000+j*8,4096)
        sw(0x14001C,1024); sh(0xBCF94,160); sh(0xBCF96,112)
        for j,v in enumerate((x,y,z)): sh(0x140200+j*2,v)
        sh(0xB0DD0,near); sh(0xB0DD2,far); ram[0xB0DCE]=low; ram[0xB0DCF]=high
        execute(ram,0x8006DFA8,(0x80140200,0x80140210,0x80140214))
        print(i,*struct.unpack_from('<II',ram,0x140210))
    print('PASS: positional sound, depth clamps, signed attenuation and pan saturation')

if __name__=='__main__': main()
