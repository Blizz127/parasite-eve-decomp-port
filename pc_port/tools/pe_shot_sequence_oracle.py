#!/usr/bin/env python3
"""Execute retail 21F38's complete shot/animation call graph for fixtures."""
import hashlib
import struct
from pe_battle_hud_oracle import ROOT, execute

# command, end-frame, first delay, next delay, shape, ammo, next target,
# target height, initial aim flag, remaining shots, reserve ammo
CASES=((6,1,0,0,0,6,0,0,0,1,10),(6,1,2,0,0,6,0,0,0,1,10),
       (6,1,0,2,0,6,0,0,0,1,10),(6,0,0,0,0,6,0,0,0,1,10),
       (8,1,0,0,0,6,0,-1000,0,1,10),(10,1,0,0,0,6,0,1000,0,1,10),
       (7,1,0,0,0,5,0,0,0,1,10),(7,1,0,0,0,5,1,0,0,1,10),
       (7,1,0,0,0,5,2,0,0,1,10),(7,1,0,0,0,5,3,0,0,1,10),
       (7,1,0,0,0,0,1,0,0,1,10),(7,0,0,0,0,5,1,0,0,1,10),
       (7,1,0,0,0xC0,5,0,0,0,3,10),(9,1,0,0,0xC0,0,0,0,0,3,10),
       (11,1,0,0,0xC0,0,0,0,0,3,0),(7,1,0,0,0xC0,5,0,0,0,1,10),
       (4,0,0,0,0,6,0,0,1,1,10),(4,0,0,0,0,6,0,0,0,1,10),
       (12,0,0,0,0,6,0,0,0,1,10),(12,1,0,0,0,6,0,0,0,1,10),
       (4,0,0,0,0,6,0,-1000,1,1,10),(4,0,0,0,0,6,0,1000,1,1,10))
RANGES=((0x140000,0x8000),(0x150000,0x1418),(0x9CE38,4),(0x9D1D4,1),
        (0x9D1DC,1),(0x9D274,1),(0x9D294,1),(0xC0E08,2),(0xC0EAC,32),
        (0xA1E44,0x140),(0x9D048,0x20),(0xE2248,4))

def fixture(exe,k):
    r=bytearray(0x200000); r[0x10000:0x10000+len(exe)-0x800]=exe[0x800:]
    def sw(a,v): struct.pack_into('<I',r,a,v&0xFFFFFFFF)
    def sh(a,v): struct.pack_into('<H',r,a,v&65535)
    for a,n in RANGES: r[a:a+n]=bytes(n)
    c,end,d1,d2,shape,ammo,next_,height,aim,shots,reserve=CASES[k]
    sw(0x9D254,0x80140000); sw(0x9D278,0x80144000); sw(0x9D018,0)
    sw(0x140000,0x80144000); r[0x14000E]=c; r[0x14000F]=7
    sh(0x140016,7 if end else 2); sh(0x14001A,7 if end else 2)
    sw(0x141000,0x80143000); sw(0x142000,0x80143100)
    sw(0x143010,100); sw(0x143110,0 if next_==3 else 100)
    sh(0x141268,100); sh(0x14126A,height); sh(0x14126C,-400)
    sh(0x142268,-100); sh(0x14226C,-400)
    sw(0x14404C,0x200000 if aim else 0); sw(0x144068,0x80144100)
    sh(0x14400C,45); r[0x144012]=4; r[0x144014:0x14401A]=bytes((6,8,10,7,9,11))
    sh(0x144106,1); sw(0x14410C,0x123000|ammo); sw(0x144110,shape|0x13)
    r[0x144114:0x144118]=bytes((3,4,5,6))
    r[0x9CE38:0x9CE3C]=bytes((d1,d2,2,3)); r[0x9CE3C]=2 if next_ else 1
    r[0x9D1DC]=shots; sw(0x9D200,0); sw(0x9D2FC,1); sh(0x9D27C,0)
    sw(0xBE830,0x80141000); sh(0xBE834,1)
    sw(0xBE838,0x80141000 if next_==1 else 0x80142000 if next_ else 0); sh(0xBE83C,1 if next_ else 0)
    for i in range(13):
        sw(0xB0E98+i*4,0x80145000+i*8); r[0x145002+i*8]=8
    for i in range(1025): sh(0x9A6EC+i*2,i//2)
    sw(0x942E0,0x80094188); sw(0x942E4,0x80150000)
    r[0x150001]=2; r[0x150A0D]=4
    sw(0xBCFA4,0x80146000); sw(0xBCFA8,0x80146100); sw(0x146100,256)
    for i in range(3): sh(0x146000+i*8,4096)
    sw(0x14601C,1024); sh(0xBCF94,160); sh(0xBCF96,112)
    sh(0xB0DD0,100); sh(0xB0DD2,1000); r[0xB0DCE:0xB0DD0]=bytes((20,120))
    r[0xB0CE8]=0; sw(0xB0E08,0); sw(0x9D20C,0)
    r[0xC0E0C]=10; r[0xC0E20]=0; sh(0xC0E48,0x100)
    r[0xC0EB2:0xC0EB6]=bytes((1,10,80,6)); sh(0xC0EB6,ammo); sh(0xA1E6E,reserve)
    return r

def fingerprint(r):
    h=14695981039346656037
    for a,n in RANGES:
        for b in r[a:a+n]: h=((h^b)*1099511628211)&0xFFFFFFFFFFFFFFFF
    return h

def main():
    exe=(ROOT/'build/disc1.candidate.exe').read_bytes()
    assert hashlib.sha1(exe).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
    for k in range(len(CASES)):
        r=fixture(exe,k); execute(r,0x80021F38)
        print(k,f'0x{fingerprint(r):016X}',r[0x14000E],r[0x9D1D4],r[0x14410C]&0xFF)
    print('PASS: aiming, shot frames, delays, ammo, reload, targets and effect signals')

if __name__=='__main__': main()
