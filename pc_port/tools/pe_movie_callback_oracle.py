#!/usr/bin/env python3
"""Original DMA1 callback graph with the real stream-handler active guard.

Only MDEC output and GPU calls are providers. Native stops at output, so
retain that exact prefix as well as the complete original graph.
"""
import itertools
import struct
import sys
from pe_movie_init_oracle import load, ROOT, execute
from pe_movie_frame_oracle import digest
RANGES=((0xB0CD0,2),(0xB0DBB,1),(0x1223F4,8),(0x1228CC,64),
        (0xB9F00,0x6300))
STOPS=(0x8007C564,0x8010C01C,0x8007506C)

def main():
    ex,o=load();rows=[]
    specs=list(itertools.product((0,1,2,128,255),(0,1,32768),(0,1),(0,1),(0,1,2),(0,1)))
    for wide,pending,bank,buffer,flag,last in specs:
        r=bytearray(0x200000);r[0x10000:0x10000+len(ex)-2048]=ex[2048:];r[0x120D00:0x122D00]=o
        for a,n in RANGES:r[a:a+n]=bytes(n)
        def sw(a,v):struct.pack_into('<I',r,a,v&0xFFFFFFFF)
        def sh(a,v):struct.pack_into('<H',r,a,v&65535)
        for a,n in RANGES[-1:]:r[a:a+n]=bytes((37+i*17)&255 for i in range(n))
        width=24 if wide else 16
        r[0xB0DBB]=wide;sh(0xB0CD0,pending);r[0x1223F8]=flag
        r[0x1228E0]=buffer;r[0x1228F2]=bank
        sw(0x1228D8,0x80160000);sw(0x1228DC,0x80161000)
        for i in (0,1):
            sh(0x1228E2+i*8,32);sh(0x1228E4+i*8,40+i*8)
            sh(0x1228E6+i*8,width*(1 if last else 3));sh(0x1228E8+i*8,4)
        sh(0x1228F4,32);sh(0x1228F6,40+bank*8);sh(0x1228F8,width);sh(0x1228FA,4)
        sw(0x9CDDC,bank);sw(0x956EC,0);sw(0xB89F4,1)
        regs=execute(r,0x801214D4,stop_at=STOPS[1:]);prefix=None;calls=[];upload=None
        while regs[31]:
            ret=regs[31];ins=struct.unpack_from('<I',r,(ret-8)&0x1FFFFF)[0]
            fn=0x80000000|((ins&0x3FFFFFF)<<2);assert fn in STOPS
            if prefix is None:
                prefix=(STOPS.index(fn)+1 if fn!=0x8007506C else 0,digest(r,RANGES),regs[4],regs[5])
            calls.append(fn)
            if fn==0x8007506C:
                upload=(*struct.unpack_from('<4h',r,regs[4]&0x1FFFFF),regs[5])
            elif fn==0x8010C01C:
                assert regs[4]==(0x80160000+(buffer^1)*0x1000) and regs[5]==width*2
            regs[2]=0
            regs=execute(r,ret,initial_regs=dict(enumerate(regs)),stop_at=STOPS[1:])
        assert calls==([] if last else [0x8010C01C])+[0x8007506C]
        assert upload==(32,40+bank*8,width,4,0x80160000+buffer*0x1000)
        kind,h,a,b=prefix
        if kind!=2:a=b=0
        rows.append((wide,pending,bank,buffer,flag,last,kind,a,b,h,digest(r,RANGES)))
    lines=['/* Original callback with real stream guard; MDEC output/GPU providers. */',
        'static const uint32_t MOVCB_ranges[][2]={'+','.join('{'+f'0x{a:X}u,{n}u'+'}' for a,n in RANGES)+'};',
        'static const struct { uint32_t wide,pending,bank,buffer,flag,last,boundary,arg0,arg1; uint64_t prefix_hash,full_hash; } MOVCB_cases[]={']
    lines += ['{'+','.join(f'{v}u' for v in row[:-2])+f',UINT64_C(0x{row[-2]:016X}),UINT64_C(0x{row[-1]:016X})'+'},' for row in rows]
    lines+=['};'];output='\n'.join(lines)+'\n';p=ROOT/'pc_port/tests/retail_movie_callback_cases.h'
    if '--check' in sys.argv:assert p.read_text()==output
    else:p.write_text(output)
    print(f'PASS {len(rows)} original callback graphs; {sum(r[6]==0 for r in rows)} have no unported native callee')
if __name__=='__main__':main()
