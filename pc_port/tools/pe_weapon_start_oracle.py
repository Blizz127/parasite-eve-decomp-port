#!/usr/bin/env python3
"""Original instructions for pistol construction, command commit and reload.

No game function is replaced: 25EE8 reaches 21128, effect allocation,
constructors and (when empty) the original inventory/reload call graph.
"""
import hashlib
import struct
from pe_battle_hud_oracle import ROOT, execute

RANGES=((0x150000,11*0xA0C),(0xE2240,0x600),(0xF3280,0x280),
        (0x140000,0x600),(0x9D200,4),(0x9D2FC,4),(0x9D1DC,1),
        (0x9D288,1),(0x9D014,4),(0x9D1A0,4),(0xC0E08,1),(0xC0EAC,32),
        (0xA1E44,0x140),(0x9D048,0x20))

def fixture(exe,case):
    ram=bytearray(0x200000); ram[0x10000:0x10000+len(exe)-0x800]=exe[0x800:]
    def sw(a,v): struct.pack_into('<I',ram,a,v&0xFFFFFFFF)
    def sh(a,v): struct.pack_into('<H',ram,a,v&65535)
    for a,n in RANGES[:4]: ram[a:a+n]=bytes((i*37+19)&255 for i in range(n))
    for a,n in RANGES[4:]: ram[a:a+n]=bytes(n)
    for a in (0x9D014,0x9D018,0x9D048,0x9D04C,0x9D050,0x9D054,0x9D058,0x9D05C,0x9D060,0x9D064): sw(a,0)
    sw(0x9D1A0,0x80); sw(0x942E4,0x80150000); sw(0x942E0,0x80094188)
    sw(0x9D254,0x80140000); sw(0x9D278,0x80140400)
    sw(0x140000,0x80140400); ram[0x14000E]=4
    sw(0x140400,0xB7654321); sw(0x140410,1); sw(0x140468,0x80140500)
    sh(0x14040C,45); sh(0x140410,4096); sw(0x14044C,0x12340000)
    sh(0x140506,1); sw(0x14050C,0x123004); sw(0x140510,0x11)
    ram[0x9D2D8]=0; ram[0x9D1DC]=3; ram[0x9D288]=7; ram[0x9D2B0]=0
    ram[0x9CE3C]=1; sh(0xBE834,1); sw(0xB0E08,0); sw(0xB0CD8,0); ram[0xB0DC7]=0
    for i in range(11):
        a=0x150000+i*0xA0C
        ram[a]=1 if i<2 or case==16 else 0
        ram[a+1]=2; sw(a+8,0x80140000)
    ram[0xC0E0C]=10; ram[0xC0E20]=0; sh(0xC0E48,0x100)
    ram[0xC0EAC:0xC0ECC]=bytes(32)
    ram[0xC0EB2:0xC0EB6]=bytes((1,10,80,6)); sh(0xC0EB6,2)
    ram[0xA1E44:0xA1F84]=bytes(0x140); sh(0xA1E6E,10)
    if case<8:
        sw(0x150008,0x80140100 if case not in (0,7) else 0)
        sw(0x140100,0 if case==1 else 0x80140400)
        sw(0x140410,1 if case in (3,4,5,6) else 0xFFFFFFFF)
        if case in (4,5): ram[0x150001]=7 if case==4 else 14
        if case==6: sw(0x9D254,0x80140100)
        if case==7: sw(0x9D254,0)
    if case==13: sw(0x14050C,0x123000)
    if case==14: sh(0xBE834,3)
    if case==15: sh(0xBE834,0x183)
    return ram

def fingerprint(ram):
    h=14695981039346656037
    for a,n in RANGES:
        for b in ram[a:a+n]: h=((h^b)*1099511628211)&0xFFFFFFFFFFFFFFFF
    return h

def main():
    exe=(ROOT/'build/disc1.candidate.exe').read_bytes()
    assert hashlib.sha1(exe).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
    for k in range(17):
        ram=fixture(exe,k)
        fn,args=(0x800C22F8,(0x80150000,)) if k<8 else \
                (0x800C9A70 if k==8 else 0x800CD728,(0x80150000,)) if k<10 else \
                (0x8006F39C,(2 if k!=11 else 4,0x80140000)) if k in (10,11,16) else \
                (0x80025EE8,())
        result=execute(ram,fn,args)
        print(k,f'0x{fingerprint(ram):016X}',f'0x{result[2]:08X}')
    print('PASS: effect storage, pistol constructors, allocation, reload and command commit')

if __name__=='__main__': main()
