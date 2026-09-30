#!/usr/bin/env python3
"""Execute original VBlank setup through its CPU-registration call boundary.

The timer-mode pointer is relocated to fixture RAM to observe the bus write;
this proves the software write, not hardware counter timing. Both original
initializer and table-clear instructions execute without replaced callees.
"""
import hashlib,struct
from pe_m0000i_leaves_oracle import load_overlay
from pe_battle_hud_oracle import execute

def main():
    ex,_,_=load_overlay()
    digest=hashlib.sha256(ex[0x743B4-0xF800:0x7440C-0xF800]).hexdigest()
    assert digest=='a8d76ebfddf20244c3904501babf8e4409502f625eefcd21ea740d9fac6c76f2'
    for n in range(32):
        ram=bytearray(0x200000)
        ram[0x10000:0x10000+len(ex)-0x800]=ex[0x800:]
        def sw(a,v):struct.pack_into('<I',ram,a,v&0xFFFFFFFF)
        sw(0x956B0,0x80150000)
        sw(0x150000,n*0x1234567)
        for i in range(9):sw(0x9568C+i*4,(n+1)*0x1234567+i)
        sw(0x95688,0xAABBCCDD)
        r=execute(ram,0x800743B4,stop_at=(0x80073CC4,))
        assert struct.unpack_from('<I',ram,0x150000)[0]==0x107
        assert ram[0x9568C:0x956B0]==bytes(36)
        assert struct.unpack_from('<I',ram,0x95688)[0]==0xAABBCCDD
        assert struct.unpack_from('<I',ram,0x956B0)[0]==0x80150000
        assert r[4:6]==[0,0x8007440C]
    print('SHA256',digest)
    print('PASS 32 original initializer prefixes through source0 registration boundary')
if __name__=='__main__':main()
