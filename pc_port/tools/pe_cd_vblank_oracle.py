#!/usr/bin/env python3
"""Original CD VBlank command issue/retry graphs; VSync query is the sole provider."""
import itertools,struct,sys,hashlib
from pe_movie_init_oracle import load,ROOT,execute
from pe_movie_frame_oracle import digest
from pe_cd_ack_oracle import Stops
RANGES=((0x9AFB4,0x28),(0x9B294,4),(0x9B554,0x154),(0xA3478,12),(0x130000,8))
def main():
    ex,_=load();rows=[]
    params=struct.unpack_from('<32I',ex,0x9B1FC-0xF800);clear=struct.unpack_from('<32I',ex,0x9B0FC-0xF800)
    for lane,state,phase,pending,flag,cb in itertools.product((1,2,3),range(11,18),range(21,25),(-1,0,1,2),(0,1),(0,1,2,3,4)):
        r=bytearray(0x200000);r[0x10000:0x10000+len(ex)-2048]=ex[2048:]
        def sw(a,v):struct.pack_into('<I',r,a,v&0xFFFFFFFF)
        for a,n in RANGES:r[a:a+n]=bytes(n)
        sw(0x9B574,lane);sw(0x9B578,state);sw(0x9B57C,phase);sw(0x9B598,pending)
        sw(0x9B594,flag);sw(0x9B5A0,2);sw(0x9B58C,0xFFFFFFFF);sw(0x9B59C,0xFFFFFFFF)
        r[0x9B588]=r[0x9B58A]=r[0x9B58B]=flag;sw(0x9B6A4,flag)
        sw(0x9B554,0 if cb==2 else 1);sw(0xA36A0,0 if cb==0 else 0x8007F7E8 if cb>=3 else 0x80170000)
        sw(0xA3608,1 if cb==3 else 0);sw(0xA3604,7);sw(0xA35E8,11);r[0xA35EC]=14;sw(0xA35F4,0x80140000)
        r[0x9B558]=14;sw(0x9B560,0x80140000);sw(0x140000,0x87654321)
        sw(0x1FEFF0,0x12345678);sw(0x9AFC0,0)
        for i in range(32):sw(0x9B5A4+i*4,flag)
        for a,v in ((0x9B27C,0x80130000),(0x9B280,0x80130001),(0x9B284,0x80130002),(0x9B288,0x80130003),(0x9B28C,0x80130004)):sw(a,v)
        struct.pack_into('<H',r,0x945E6,0)
        stops=Stops((0x80073A44,0x80170000));pc=0x8007FE24;initial=None;b=0
        while True:
            stops.last=0;regs=execute(r,pc,initial_regs=initial,stop_at=stops,instruction_budget=20000)
            if not stops.last:break
            if stops.last==0x80170000:b=stops.last;break
            regs[2]=0;pc=regs[31];initial=dict(enumerate(regs))
        rows.append((lane,state,phase,pending&0xFFFFFFFF,flag,cb,b,digest(r,RANGES)))
    lines=['/* Original VBlank -> reset/issue graphs; VSync(-1) returns0 from provider. */',
      'static const uint32_t CDVB_ranges[][2]={'+','.join('{'+f'0x{a:X}u,{n}u'+'}' for a,n in RANGES)+'};',
      'static const uint32_t CDVB_params[]={'+','.join(f'{x}u' for x in params)+'};',
      'static const uint32_t CDVB_clear[]={'+','.join(f'{x}u' for x in clear)+'};',
      'static const struct { uint32_t lane,state,phase,pending,flag,cb,boundary; uint64_t hash; } CDVB_cases[]={']
    lines+=['{'+','.join(f'{x}u' for x in row[:-1])+f',UINT64_C(0x{row[-1]:016X})'+'},' for row in rows];lines+=['};']
    out='\n'.join(lines)+'\n';p=ROOT/'pc_port/tests/retail_cd_vblank_cases.h'
    if '--check' in sys.argv:assert p.read_text()==out
    else:p.write_text(out)
    print(f'PASS {len(rows)} original CD VBlank graphs/prefixes ({sum(r[6]==0 for r in rows)} complete)')
    for a,b in ((0x7FE24,0x800F4),(0x800F4,0x80164),(0x7F7E8,0x7F88C)):
        print(f'{a:X}..{b:X} {(b-a)//4} words {hashlib.sha256(ex[a-0xF800:b-0xF800]).hexdigest()}')
if __name__=='__main__':main()
