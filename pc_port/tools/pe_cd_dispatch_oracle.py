#!/usr/bin/env python3
"""Original interrupt/data-ready callback chain into real RAM stream assembly."""
import struct,sys
from pe_movie_init_oracle import load,ROOT,execute
from pe_movie_frame_oracle import digest
from pe_cd_ack_oracle import Stops
from pe_cd_stream_oracle import RANGES as STREAM_RANGES
RANGES=STREAM_RANGES+((0x9AFC4,12),(0x9B554,64),(0xA3520,32))
SEEDS=((0x9AFB8,0x80080778),(0x9AFB4,0),(0x9B554,1),(0x9B558,0),
       (0x9B624,1),(0xA36A8,0x8007F88C),(0xB8AB4,0x800813E8),
       (0xC0DC8,0x80150000),(0xC20C4,8),(0xC0DB8,0x80160000),
       (0xA801C,1),(0xA8020,1),(0xB8620,3),(0xB6914,4),
       (0xB89F4,0),(0x9B17C,1),(0x9B07C,0))

def main():
    ex,o=load();rows=[]
    for kind in range(13):
      for seed in (37,165):
       for index in (0,3):
        r=bytearray(0x200000);r[0x10000:0x10000+len(ex)-2048]=ex[2048:]
        def sw(a,v):struct.pack_into('<I',r,a,v&0xFFFFFFFF)
        def sh(a,v):struct.pack_into('<H',r,a,v&65535)
        for a,n in RANGES:r[a:a+n]=bytes(n)
        r[0x150000:0x155000]=bytes([0xCC])*0x5000
        for a,v in SEEDS:sw(a,v)
        sh(0x945E6,1);sw(0xB0CC8,0);sw(0xB0CCC,0)
        sw(0xBE998,0);sw(0xBE9E4,0);sw(0xC0DC0,0);sw(0xC0DBC,0);sw(0xBCD7C,0)
        for i in range(8):sh(0x150000+i*32,0)
        for i in range(2048):r[0x160000+i]=(seed+i*17)&255
        for i,v in enumerate((0x160,3<<10,0,1,7)):sh(0x160000+i*2,v)
        tag=1;size=1;payload=[2,0x80,0x20,0x82,0,0,0,0]
        if kind==1:sh(0x160006,2)
        if kind==2:sw(0xB89F4,1)
        if kind==3:sw(0x9B554,0)
        if kind==4:sw(0xB8AB4,0)
        if kind==5:sw(0x9AFB8,0)
        if kind==6:sh(0x160000,0x161)
        if kind==7:tag=5;size=2;payload[0]=16
        if kind==8:sw(0xB8AB4,0x80170000)
        if kind==9:tag=4
        if kind==10:tag=3;sw(0x9AFB4,0x80080164)
        if kind==11:sw(0x9B624,0)
        if kind==12:sw(0x9B624,4);size=8
        for a,v in ((0x9B27C,0x80130000),(0x9B280,0x80130001),
          (0x9B284,0x80130002),(0x9B288,0x80130003),(0x9B32C,0x80130000),
          (0x9B338,0x80130003),(0x9B33C,0x80130010),(0x9B340,0x80130014),
          (0x9B35C,0x80130018),(0x9B34C,0x8013001C)):sw(a,v)
        r[0x130000:0x130020]=bytes(32);r[0x130000]=index
        sw(0x1FEF68,0x55667788)
        stops=Stops((0x8007AAE0,0x8007AB08,0x8007AB18,0x8007AB3C,0x8007AB5C,
          0x8007ABB8,0x80073A44,0x80170000))
        pc=0x8007C13C;initial=None;pending=tag;pos=0;boundary=0;arg0=arg1=0
        while True:
            stops.last=0;regs=execute(r,pc,initial_regs=initial,stop_at=stops,instruction_budget=200000)
            pc=stops.last
            if not pc:break
            if pc in (0x8007E704,0x80170000,0x80080164):
                boundary=pc;arg0,arg1=regs[4:6];break
            if pc==0x80073A44:
                regs[2]=0;pc=regs[31];initial=dict(enumerate(regs));continue
            if pc in (0x8007AAE0,0x8007AB08,0x8007AB18):regs[2]=pending
            elif pc==0x8007AB3C:regs[2]=1|(0x20 if pos<size else 0)
            elif pc==0x8007AB5C:regs[2]=payload[pos];pos+=1
            else:r[regs[2]&0x1FFFFF]=regs[3]&255;pending&=~regs[3]&7
            pc+=4;initial=dict(enumerate(regs))
        rows.append((kind,seed,index,tag,size,boundary,arg0,arg1,digest(r,RANGES)))
    lines=['/* Full original data-ready chain; CD byte reads and VSync are providers. */',
      'static const uint32_t CDDISP_ranges[][2]={'+','.join('{'+f'0x{a:X}u,{n}u'+'}' for a,n in RANGES)+'};',
      'static const uint32_t CDDISP_seeds[][2]={'+','.join('{'+f'0x{a:X}u,0x{v:X}u'+'}' for a,v in SEEDS)+'};',
      'static const struct { uint32_t kind,seed,index,tag,size,boundary,arg0,arg1; uint64_t hash; } CDDISP_cases[]={']
    lines+=['{'+','.join(f'{x}u' for x in row[:-1])+f',UINT64_C(0x{row[-1]:016X})'+'},' for row in rows];lines+=['};']
    out='\n'.join(lines)+'\n';p=ROOT/'pc_port/tests/retail_cd_dispatch_cases.h'
    if '--check' in sys.argv:assert p.read_text()==out
    else:p.write_text(out)
    print(f'PASS {len(rows)} original interrupt/data-ready graphs and boundary prefixes')
if __name__=='__main__':main()
