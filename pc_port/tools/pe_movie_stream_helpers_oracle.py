#!/usr/bin/env python3
"""Original movie retry helpers and stream teardown; critical/DMA-slot providers explicit."""
import struct,sys
from pe_movie_init_oracle import load,ROOT,execute
from pe_movie_frame_oracle import digest
from pe_cd_ack_oracle import Stops
RANGES=((0xA3490,12),(0x130000,12))
def main():
    ex,_=load()
    def fresh():
        r=bytearray(0x200000);r[0x10000:0x10000+len(ex)-2048]=ex[2048:];return r
    def sw(r,a,v):struct.pack_into('<I',r,a,v&0xFFFFFFFF)
    conv=[];inverse=[];retry=[];close=[]
    values=(-2147483648,-2147483499,-10000,-4510,-225,-151,-150,-149,-1,0,1,74,75,4349,4350,4499,4500,189742,2147483497,2147483647)
    for value in values:
        r=fresh();sw(r,0x130000,0xA5A5A5A5)
        regs=execute(r,0x8007A930,(value&0xFFFFFFFF,0x80130000))
        assert regs[2]==0x80130000
        conv.append((value&0xFFFFFFFF,struct.unpack_from('<I',r,0x130000)[0]))
    for loc in (0,0x000200,0x005974,0x009999,0xFFFFFF,0xABCDEF,0x594959,0xFF00FF):
        r=fresh();sw(r,0x130000,loc)
        regs=execute(r,0x8007AA34,(0x80130000,));inverse.append((loc,regs[2]))
        for guard in (0,1,0xFFFFFFFF):
            for dest in (0x80130000,0x800A3490,0x800A3491,0x800A3493,0x800A3494):
                r=fresh()
                for a,n in RANGES:r[a:a+n]=bytes([0xA5])*n
                sw(r,0xA3490,loc);sw(r,0xA3494,0x12345678);sw(r,0xA8020,guard)
                regs=execute(r,0x8007C2A0,(dest,));retry.append((loc,guard,dest,regs[2],digest(r,RANGES)))
    for lane in (0,1,2,0xFFFFFFFF):
        for alias in (0,1):
            r=fresh();sw(r,0x9AFD8,lane);sw(r,0x9AFB8,0x89ABCDEF);sw(r,0xB8AB4,0x12345678)
            sw(r,0x9AF1C,0x80130000);sw(r,0x9AF28,0x80130000 if alias else 0x80130004)
            r[0x130000:0x130008]=bytes([0xA5])*8
            stops=Stops((0x80072714,0x80072724,0x80073CF4));pc=0x8007A2A4;initial=None;calls=[]
            while True:
                stops.last=0;regs=execute(r,pc,initial_regs=initial,stop_at=stops)
                if not stops.last:break
                calls.append(stops.last)
                if stops.last==0x80073CF4:assert regs[4:6]==[3,0]
                regs[2]=1 if stops.last==0x80072714 else 0;pc=regs[31];initial=dict(enumerate(regs))
            assert calls==[0x80072714,0x80073CF4,0x80072724]
            close.append((lane,alias,*struct.unpack_from('<2I',r,0x130000),struct.unpack_from('<I',r,0x9AFB8)[0],struct.unpack_from('<I',r,0xB8AB4)[0]))
    out='/* Original retry graphs; shutdown critical/DMA-slot call providers. */\n'
    for name,fields,rows in (('convert','lba,word',conv),('inverse','word,result',inverse),('close','lane,alias,first,second,low_callback,high_callback',close)):
        out+=f'static const struct {{ uint32_t {fields}; }} MOVSTREAM_{name}[]={{\n'+''.join('{'+','.join(f'0x{v:X}u' for v in row)+'},\n' for row in rows)+'};\n'
    out+='static const uint32_t MOVSTREAM_ranges[][2]={'+','.join('{'+f'0x{a:X}u,{n}u'+'}' for a,n in RANGES)+'};\n'
    out+='static const struct { uint32_t word,guard,dest,result; uint64_t hash; } MOVSTREAM_retry[]={\n'+''.join('{'+','.join(f'0x{v:X}u' for v in row[:-1])+f',UINT64_C(0x{row[-1]:016X})'+'},\n' for row in retry)+'};\n'
    p=ROOT/'pc_port/tests/retail_movie_stream_helpers_cases.h'
    if '--check' in sys.argv:assert p.read_text()==out
    else:p.write_text(out)
    print(f'PASS {len(conv)} signed conversions, {len(inverse)} inverse, {len(retry)} complete retry and {len(close)} teardown graphs')
if __name__=='__main__':main()
