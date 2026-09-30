#!/usr/bin/env python3
"""Original SDK state initialization and complete rejected-command startup path."""
import itertools,struct,sys,hashlib
from pe_movie_init_oracle import load,ROOT,execute
from pe_movie_frame_oracle import digest
from pe_cd_ack_oracle import Stops
RANGES=((0x945E4,0x32),(0x9AFB4,0x28),(0x9B294,4),(0x9B554,0x50),(0x130000,8),(0x130020,2))
def main():
    ex,_=load();rows=[];states=[]
    for seed in (0,37,165,255):
      r=bytearray(0x200000);r[0x10000:0x10000+len(ex)-2048]=ex[2048:]
      r[0x9B554:0x9B5A4]=bytes((seed+i*17)&255 for i in range(0x50))
      execute(r,0x8007FA2C,instruction_budget=3000)
      states.append((seed,digest(r,((0x9B554,0x50),))))
    for seed,old,mask,debug in itertools.product((37,165),(0,0x8007C13C,0x80170000),(0,9,0xFFFF),(0,0xFFFFFFFF)):
        r=bytearray(0x200000);r[0x10000:0x10000+len(ex)-2048]=ex[2048:]
        def sw(a,v):struct.pack_into('<I',r,a,v)
        def sh(a,v):struct.pack_into('<H',r,a,v)
        for a,n in RANGES:r[a:a+n]=bytes((seed+i*17)&255 for i in range(n))
        sh(0x945E4,1);sh(0x94614,9);sw(0x945F0,old);sw(0x9AFC0,debug)
        sw(0x9566C,0x80130030);sw(0x130038,0x800740D0);sw(0x13003C,0x80073E28)
        sw(0x95674,0x80130020);sh(0x130020,mask)
        for a,v in ((0x9B27C,0x80130000),(0x9B280,0x80130001),(0x9B284,0x80130002),(0x9B288,0x80130003),(0x9B28C,0x80130004)):sw(a,v)
        r[0x130003]=0
        # Force the real issuer's missing-parameter branch for Nop and Init.
        sw(0x9B200,1);sw(0x9B224,1)
        stops=Stops((0x80073C5C,0x80071A74));pc=0x8007BBFC;initial=None;calls=[]
        while True:
            stops.last=0;regs=execute(r,pc,initial_regs=initial,stop_at=stops,instruction_budget=5000)
            if not stops.last:break
            calls.append((stops.last,regs[4],regs[5] if stops.last==0x80071A74 else 0))
            regs[2]=0;pc=regs[31];initial=dict(enumerate(regs))
        assert calls==[(0x80073C5C,0x80011C08,0),(0x80071A74,0x80011C14,0x8009B298)]
        assert regs[2]==0xFFFFFFFF
        rows.append((seed,old,mask,debug,regs[2],digest(r,RANGES)))
    lines=['/* Original initial state and rejected-command initializer graphs; console calls are providers. */',
      'static const uint32_t CDINIT_ranges[][2]={'+','.join('{'+f'0x{a:X}u,{n}u'+'}' for a,n in RANGES)+'};',
      'static const struct { uint32_t seed,old,mask,debug,result; uint64_t hash; } CDINIT_cases[]={']
    lines+=['{'+','.join(f'{x}u' for x in row[:-1])+f',UINT64_C(0x{row[-1]:016X})'+'},' for row in rows];lines+=['};','static const struct { uint32_t seed; uint64_t hash; } CDINIT_states[]={']
    lines+=['{'+f'{seed}u,UINT64_C(0x{h:016X})'+'},' for seed,h in states];lines+=['};']
    out='\n'.join(lines)+'\n';p=ROOT/'pc_port/tests/retail_cd_initialization_cases.h'
    if '--check' in sys.argv:assert p.read_text()==out
    else:p.write_text(out)
    print(f'PASS {len(rows)} original rejected-command startup graphs, {len(states)} SDK state graphs')
    for a,b in ((0x7BBFC,0x7BDDC),(0x7FA2C,0x7FB04)):
        print(f'{a:X}..{b:X} {(b-a)//4} words {hashlib.sha256(ex[a-0xF800:b-0xF800]).hexdigest()}')
if __name__=='__main__':main()
