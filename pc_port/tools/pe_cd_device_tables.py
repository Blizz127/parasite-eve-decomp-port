#!/usr/bin/env python3
"""Authenticate original SDK tables for native device-driven integration tests."""
import struct,sys
from pe_movie_init_oracle import load,ROOT

def main():
    ex,_=load();lines=['/* Authenticated original Disc1 SDK command tables. */',
      'static const struct { uint32_t address,values[32]; } CDDEV_tables[]={']
    for a in (0x9B07C,0x9B17C,0x9B0FC,0x9B1FC,0x9B5A4,0x9B624):
        values=struct.unpack_from('<32I',ex,a-0xF800)
        lines.append('{'+f'0x{a:X}u,'+'{'+','.join(f'0x{x:X}u' for x in values)+'}},')
    lines.append('};')
    values=struct.unpack_from('<26I',ex,0x11D0C-0xF800)
    lines.append('static const uint32_t CDDEV_command_jumps[]={'+','.join(f'0x{x:X}u' for x in values)+'};')
    out='\n'.join(lines)+'\n';p=ROOT/'pc_port/tests/retail_cd_device_tables.h'
    if '--check' in sys.argv:assert p.read_text()==out
    else:p.write_text(out)
    print('PASS authenticated6 original command tables (192 words) and26 command jumps')
if __name__=='__main__':main()
