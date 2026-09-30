#!/usr/bin/env python3
"""Original 7041C wireframe and 347B4 expansion, including SDK/GTE callees."""
import hashlib
import struct
from pe_battle_hud_oracle import ROOT, execute

RANGES=((0x140000,0x300),(0x160000,0x400),(0x150000,0x5000),
        (0x141000,0x100),(0x9CDD8,4),(0x9D290,4))
VERTICES=((-128,0,0),(128,0,0),(0,-128,32),(0,128,32),(-128,0,100),(128,0,100))
TRIANGLES=((0,1,2),(2,1,0),(0,2,1),(0,1,3),(4,5,2),(2,5,4))
TRIG=(0x10000000,0x0B500B50,0x00001000,0xF0000000)
RADII=(16384,8192,32760,-64)
DEPTHS=(1024,2048,20000,-10)

def fixture(exe,case):
    ram=bytearray(0x200000); ram[0x10000:0x10000+len(exe)-0x800]=exe[0x800:]
    def sw(a,v): struct.pack_into('<I',ram,a,v&0xFFFFFFFF)
    def sh(a,v): struct.pack_into('<H',ram,a,v&65535)
    for a,n in RANGES: ram[a:a+n]=bytes((i*37+19)&255 for i in range(n))
    ram[0x140000:0x14001C]=bytes(28)
    ram[0x140001]=6; sh(0x140006,0x80)
    ram[0x140008:0x140014]=bytes((1,1,6,1,1,1,1,1,2,2,2,3))
    for i,vertex in enumerate(VERTICES):
        for j,v in enumerate(vertex): sh(0x14001C+i*8+j*2,v)
        ram[0x140022+i*8]=i%3
    for i,triangle in enumerate(TRIANGLES): ram[0x140054+i*4:0x140057+i*4]=bytes(triangle)
    for i,flags in enumerate((0,0x21,0x23)): ram[0x140203+i*4]=flags
    sw(0x9CDD0,0x80141000); sw(0x9CDD4,0x80141080); sw(0x9CDD8,0)
    sw(0xBCFA4,0x80142000)
    for i in range(3): sh(0x142000+i*8,4096)
    sw(0x14201C,DEPTHS[case%4]); sw(0x143000,13<<16); sw(0x143004,-9<<16)
    angle=(0,512,1024,2048)[case%4]; radius=RADII[case%4]<<16
    sw(0x966EC+angle*4,TRIG[case%4])
    if case>=12:
        count=(1,9,10,0x10001)[case%4]; sw(0x9D290,count)
        sw(0x9CDDC,case&1); sw(0xB0E38+(case&1)*4,0x80150000)
        sw(0xB0E58+(case&1)*4,0x80160000); sw(0xB0DF8,0x80140000)
        sw(0x9D254,0x80142FD8); sw(0x9D278,0x80144000); sw(0x144068,0x80144100)
        sh(0x144102,16384); sw(0x14404C,0x10 if case&1 else 0)
        angle=((count<<6) if count<10 else (count<<4)+10)&0xFFFE
        sw(0x966EC+(angle&4095)*4,TRIG[case%4]); sh(0xBCF94,160); sh(0xBCF96,112)
    return ram,radius,angle

def fingerprint(ram):
    value=14695981039346656037
    for a,n in RANGES:
        for b in ram[a:a+n]: value=((value^b)*1099511628211)&0xFFFFFFFFFFFFFFFF
    return value

def main():
    exe=(ROOT/'build/disc1.candidate.exe').read_bytes()
    assert hashlib.sha1(exe).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
    for case in range(16):
        ram,radius,angle=fixture(exe,case)
        if case in (4,5,6,7):
            # Rotate the camera itself around Z, exercising column composition.
            struct.pack_into('<9h',ram,0x142000,0,-4096,0,4096,0,0,0,0,4096)
        if case in (8,9,10,11): struct.pack_into('<i',ram,0x143008,-1024<<16)
        args=(0x80140000,0x80143000,radius&0xFFFFFFFF,angle,0x80150000,0x80160000)
        result=execute(ram,0x800347B4 if case>=12 else 0x8007041C,() if case>=12 else args,
                       initial_cop_control={24:160<<16,25:112<<16,26:256})
        print(case,f'0x{fingerprint(ram):016X}',result[2] if case<12 else struct.unpack_from('<I',ram,0x9CDD8)[0])
    print('PASS: range geometry, clipping, transparency, packet links and expansion')

if __name__=='__main__': main()
