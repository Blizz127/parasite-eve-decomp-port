#!/usr/bin/env python3
"""Original DMA issuer, complete idle/request-ready paths with MMIO shadow inputs."""
import itertools,struct,sys
from pe_movie_init_oracle import load,ROOT,execute
from pe_cd_ack_oracle import Stops

def main():
    ex,_=load();rows=[]
    for interrupt,enable,words in itertools.product((0,1,0x101,2),(0x80,0x8C),(8,504)):
        ram=bytearray(0x200000);ram[0x10000:0x10000+len(ex)-2048]=ex[2048:]
        def sw(a,v):struct.pack_into('<I',ram,a,v)
        sw(0x9B348,0x80130000);sw(0x130000,enable<<16|0x1234)
        sw(0x9B344,0x80130004);sw(0x130004,0x12345678)
        sw(0x9B32C,0x80130008);ram[0x130008]=0x40
        sw(0x1FF010,0x11000000);sw(0x1FF014,interrupt);sw(0x1FF018,0)
        # Only fixed DMA register accesses need physical MMIO providers.
        stops=Stops((0x8007CEEC,0x8007CFDC,0x8007CFE4,0x8007D024,0x8007D028))
        pc=0x8007CEAC;initial=None;args=(3,0x80150000,0,words);mmio={0x1F8010B8:0}
        while True:
            stops.last=0;regs=execute(ram,pc,args=args,initial_regs=initial,stop_at=stops)
            if not stops.last:break
            at=stops.last
            if at==0x8007CEEC:regs[2]=mmio[regs[2]+0x1088]
            elif at==0x8007CFDC:mmio[regs[5]]=regs[18]
            elif at in (0x8007CFE4,0x8007D024):mmio[regs[5]]=regs[2]
            elif at==0x8007D028:regs[2]=mmio[regs[5]]
            pc=at+4;initial=dict(enumerate(regs));args=()
        rows.append((interrupt,enable,words,struct.unpack_from('<I',ram,0x130000)[0],struct.unpack_from('<I',ram,0x130004)[0],mmio[0x1F8010B0],mmio[0x1F8010B4],mmio[0x1F8010B8]))
    out='/* Original idle/request-ready DMA issuer graphs; MMIO shadow providers. */\nstatic const struct { uint32_t interrupt,enable,words,dicr,dpcr,madr,bcr,chcr; } CDDMA_cases[]={\n'+''.join('{'+','.join(f'0x{x:X}u' for x in r)+'},\n' for r in rows)+'};\n'
    p=ROOT/'pc_port/tests/retail_cd_dma_issue_cases.h'
    if '--check' in sys.argv:assert p.read_text()==out
    else:p.write_text(out)
    print(f'PASS {len(rows)} complete original DMA issuer graphs')
if __name__=='__main__':main()
