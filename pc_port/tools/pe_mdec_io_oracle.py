#!/usr/bin/env python3
"""Original libpress command flags and output issue graphs; wait/input providers explicit."""
import hashlib,itertools,struct,sys
from pe_movie_init_oracle import load,ROOT,execute
from pe_m0000i_leaves_oracle import find_disc,read_form1
from pe_cd_ack_oracle import Stops

def main():
    ex,_=load();overlay=read_form1(find_disc(ROOT),1940,38)
    assert hashlib.sha256(overlay[0xC27C-0xBCF8:0xC308-0xBCF8]).hexdigest()=='9b251c4da588b28ef1b1f26d4f73a06a56a451814e30e9391876a7ba265dea03'
    def fresh():
        r=bytearray(0x200000);r[0x10000:0x10000+len(ex)-2048]=ex[2048:];r[0x10BCF8:0x10BCF8+len(overlay)]=overlay
        return r
    ins=[];outs=[]
    for command,mode in itertools.product((0x30000020,0x3E000040,0x20000020,0x28000040),(0,1,2,3,4,0xFFFFFFFF)):
        r=fresh();struct.pack_into('<I',r,0x140000,command)
        stop=Stops((0x8010C1EC,));regs=execute(r,0x8010BFA0,args=(0x80140000,mode),stop_at=stop)
        assert stop.last==0x8010C1EC
        changed=struct.unpack_from('<I',r,0x140000)[0]
        ins.append((command,mode,changed,regs[4],regs[5]))
        regs[2]=0;execute(r,regs[31],initial_regs=dict(enumerate(regs)))
    for destination,words in itertools.product((0x80160000,0x80160003),(0,1,31,32,33,128,159,0xFFFFFFFF)):
        r=fresh()
        for slot,address in ((0x10DB54,0x80130000),(0x10DB30,0x80130004),(0x10DB28,0x80130008),(0x10DB2C,0x8013000C)):
            struct.pack_into('<I',r,slot,address)
        struct.pack_into('<I',r,0x130000,0x12345678)
        stop=Stops((0x8010C39C,));regs=execute(r,0x8010C01C,args=(destination,words),stop_at=stop)
        assert stop.last==0x8010C39C
        regs[2]=0;execute(r,regs[31],initial_regs=dict(enumerate(regs)))
        dpcr,chcr,madr,bcr=struct.unpack_from('<4I',r,0x130000)
        outs.append((destination,words,dpcr,chcr,madr,bcr))
    out='/* Original libpress graphs with input/wait providers; not hardware pixel oracles. */\n'
    for name,fields,rows in (('input','command,mode,changed,address,words',ins),('output','destination,words,dpcr,chcr,madr,bcr',outs)):
        out+=f'static const struct {{ uint32_t {fields}; }} MDECIO_{name}[]={{\n'+''.join('{'+','.join(f'0x{x:X}u' for x in row)+'},\n' for row in rows)+'};\n'
    p=ROOT/'pc_port/tests/retail_mdec_io_cases.h'
    if '--check' in sys.argv:assert p.read_text()==out
    else:p.write_text(out)
    print(f'PASS {len(ins)} original input-wrapper and{len(outs)} output-issuer graphs')
if __name__=='__main__':main()
