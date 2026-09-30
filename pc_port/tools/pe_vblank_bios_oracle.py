#!/usr/bin/env python3
"""Execute the two original BIOS veneers through their table-call boundary.

This verifies ABI forwarding/service selection, not a replacement BIOS ROM.
"""
import hashlib
from pe_m0000i_leaves_oracle import load_overlay
from pe_battle_hud_oracle import execute

def main():
    ex,_,_=load_overlay()
    source=ex[0x73C74-0xF800:0x73C80-0xF800]+ex[0x73C84-0xF800:0x73C90-0xF800]
    digest=hashlib.sha256(source).hexdigest()
    assert digest=='88a578c1ecfaeee8cbbae622a3423ee1d4231699ad9ad4d96959f527588a3887'
    for entry,table,service in ((0x80073C74,0xB0,0x5B),(0x80073C84,0xC0,0xA)):
        for n in range(32):
            ram=bytearray(0x200000);ram[0x10000:0x10000+len(ex)-0x800]=ex[0x800:]
            args=(n%4,(n*0x13579BDF)&0xFFFFFFFF)
            r=execute(ram,entry,args,stop_at=(table,))
            assert r[9]==service and tuple(r[4:6])==args and r[31]==0
    print('SHA256',digest)
    print('PASS 64 original BIOS veneer forwarding cases')
if __name__=='__main__':main()
