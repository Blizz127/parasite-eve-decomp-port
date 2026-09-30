#!/usr/bin/env python3
"""Authenticate retail libpress quantization and scale fixtures (not a pixel oracle)."""
import hashlib,struct,sys
from pe_movie_init_oracle import load,ROOT
from pe_m0000i_leaves_oracle import find_disc,read_form1
from b54kag_mdec_reset_oracle import QUANT_SHA256,SCALE_SHA256

def main():
    load();overlay=read_form1(find_disc(ROOT),1940,38)
    lines=['/* Authenticated retail libpress table command blocks. */']
    for name,address,sha in (('quant',0x8010DA0C,QUANT_SHA256),('scale',0x8010DA90,SCALE_SHA256)):
        data=overlay[address-0x8010BCF8:address-0x8010BCF8+132]
        assert hashlib.sha256(data[4:]).hexdigest()==sha
        lines.append(f'static const uint32_t MDECPIX_{name}[]={{'+','.join(f'0x{x:X}u' for x in struct.unpack('<33I',data))+'};')
    out='\n'.join(lines)+'\n';path=ROOT/'pc_port/tests/retail_mdec_pixel_tables.h'
    if '--check' in sys.argv:assert path.read_text()==out
    else:path.write_text(out)
    print('PASS authenticated retail MDEC tables (256 payload bytes)')
if __name__=='__main__':main()
