#!/usr/bin/env python3
"""Execute installed original source2 registration wrapper and complete worker."""
import itertools,struct,sys,hashlib
from pe_movie_init_oracle import load,ROOT,execute
from pe_movie_frame_oracle import digest
RANGES=((0x945E4,0x32),)
def main():
    ex,_=load();rows=[]
    for guard,old,new,mask,registered in itertools.product((0,1),(0,0x8007C13C,0x80170000),(0,0x8007C13C,0x80170000),(0,4,0x555,0x7FF,0xFFFF),(0,4,0xFFFF)):
        r=bytearray(0x200000);r[0x10000:0x10000+len(ex)-2048]=ex[2048:]
        def sw(a,v):struct.pack_into('<I',r,a,v)
        def sh(a,v):struct.pack_into('<H',r,a,v)
        r[0x945E4:0x94616]=bytes((37+i*17)&255 for i in range(0x32))
        sh(0x945E4,guard);sw(0x945F0,old);sh(0x94614,registered)
        sw(0x9566C,0x80130010);sw(0x130018,0x800740D0)
        sw(0x95674,0x80130000);sh(0x130000,mask)
        regs=execute(r,0x80073CC4,(2,new),instruction_budget=1000)
        rows.append((guard,old,new,mask,registered,regs[2],struct.unpack_from('<H',r,0x130000)[0],digest(r,RANGES)))
    lines=['/* Installed original 73CC4 -> 740D0 source2 graphs; I_MASK redirected to RAM. */','static const uint32_t CDREG_ranges[][2]={{0x945E4u,0x32u}};',
       'static const struct { uint32_t guard,old,next,mask,registered,result,final_mask; uint64_t hash; } CDREG_cases[]={']
    lines+=['{'+','.join(f'{x}u' for x in row[:-1])+f',UINT64_C(0x{row[-1]:016X})'+'},' for row in rows];lines+=['};']
    out='\n'.join(lines)+'\n';p=ROOT/'pc_port/tests/retail_cd_registration_cases.h'
    if '--check' in sys.argv:assert p.read_text()==out
    else:p.write_text(out)
    print(f'PASS {len(rows)} complete original source2 registration graphs')
    for a,b in ((0x73CC4,0x73CF4),(0x740D0,0x74218)):
        print(f'{a:X}..{b:X} {(b-a)//4} words {hashlib.sha256(ex[a-0xF800:b-0xF800]).hexdigest()}')
if __name__=='__main__':main()
