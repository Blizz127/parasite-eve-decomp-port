#!/usr/bin/env python3
"""Original acknowledge graph with instruction-level CD response read providers."""
import itertools,struct,sys
from pe_movie_init_oracle import load,ROOT,execute
from pe_movie_frame_oracle import digest
RANGES=((0x9AFC4,12),(0x9B294,3),(0xA3460,24))
JUMPS=(0x8007AE5C,0x8007AE10,0x8007AD10,0x8007AEDC,0x8007AF5C)
class Stops(set):
    last=0
    def __contains__(self,pc):
        if super().__contains__(pc):self.last=pc;return True
        return False

def main():
    ex,o=load();rows=[]
    specs=[(*v,0,0) for v in itertools.product(range(1,6),(0,1,2,8,12),(0,1,16,128,29),(0,1),(0,16))]
    specs += [(0,0,0,0,0,0,0),(5,2,29,1,0,1,0),(6,2,16,0,0,0,0),(7,0,0,0,16,0,0),(3,8,0,1,0,0,1)]
    for tag,size,first,table,old,debug,dirty in specs:
        r=bytearray(0x200000);r[0x10000:0x10000+len(ex)-2048]=ex[2048:]
        def sw(a,v):struct.pack_into('<I',r,a,v&0xFFFFFFFF)
        for a,n in RANGES:r[a:a+n]=bytes((37+i*17)&255 for i in range(n))
        sw(0x9AFC4,old);sw(0x9AFCC,0xFFFFFFFF);sw(0x9AFC0,debug)
        r[0x9AFD5]=2;sw(0x9B184,table);sw(0x9B084,table)
        if dirty:sw(0x11B94,0x80170000)
        for a,v in ((0x9B27C,0x80130000),(0x9B280,0x80130001),(0x9B284,0x80130002),(0x9B288,0x80130003)):sw(a,v)
        payload=bytes([first]+[(37+i*17)&255 for i in range(1,16)])[:size]
        pos=0;pending=tag;boundary=0
        stops=Stops((0x8007AAE0,0x8007AB08,0x8007AB18,0x8007AB3C,0x8007AB5C,0x8007ABB8,0x80071A74,0x80073C5C,0x80170000))
        pc=0x8007AAB4;initial=None
        while True:
            stops.last=0
            regs=execute(r,pc,initial_regs=initial,stop_at=stops)
            pc=stops.last
            if not pc:break
            if pc in (0x80071A74,0x80073C5C,0x80170000):boundary=(1 if pc==0x80071A74 else 2 if pc==0x80073C5C else 3);break
            if pc in (0x8007AAE0,0x8007AB08,0x8007AB18):regs[2]=pending
            elif pc==0x8007AB3C:regs[2]=1|(0x20 if pos<size else 0)
            elif pc==0x8007AB5C:regs[2]=payload[pos];pos+=1
            else:
                assert pc==0x8007ABB8
                r[regs[2]&0x1FFFFF]=regs[3]&255;pending&=~regs[3]&7
            pc+=4;initial=dict(enumerate(regs))
        rows.append((tag,size,first,table,old,debug,dirty,boundary,0 if boundary else regs[2],pos,digest(r,RANGES)))
    lines=['/* Original acknowledge graph; CD reads/acknowledgment are explicit providers. */',
      'static const uint32_t CDACK_ranges[][2]={'+','.join('{'+f'0x{a:X}u,{n}u'+'}' for a,n in RANGES)+'};',
      'static const uint32_t CDACK_jumps[]={'+','.join(f'0x{x:X}u' for x in JUMPS)+'};',
      'static const struct { uint32_t tag,size,first,table,old,debug,dirty,boundary,result,consumed; uint64_t hash; } CDACK_cases[]={']
    lines+=['{'+','.join(f'{x}u' for x in row[:-1])+f',UINT64_C(0x{row[-1]:016X})'+'},' for row in rows];lines+=['};']
    output='\n'.join(lines)+'\n';p=ROOT/'pc_port/tests/retail_cd_ack_cases.h'
    if '--check' in sys.argv:assert p.read_text()==output
    else:p.write_text(output)
    print(f'PASS {len(rows)} original acknowledge graphs/prefixes with CD register providers')
if __name__=='__main__':main()
