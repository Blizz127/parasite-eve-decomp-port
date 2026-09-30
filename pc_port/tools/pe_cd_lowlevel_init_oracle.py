#!/usr/bin/env python3
"""Original low-level startup graph and public already-initialized guard."""
import itertools,struct,sys,hashlib
from pe_movie_init_oracle import load,ROOT,execute
from pe_movie_frame_oracle import digest
from pe_cd_ack_oracle import Stops
RANGES=((0x945E4,0x32),(0x9568C,36),(0x9AFB4,0x28),(0x9B294,4),(0x9B554,0x50),(0x9B6B8,4),(0xA36A0,16),(0xA3478,12),(0x130000,8),(0x130020,2),(0x150000,0x200))
def main():
    ex,_=load();rows=[];guards=[]
    def fresh(seed):
        r=bytearray(0x200000);r[0x10000:0x10000+len(ex)-2048]=ex[2048:]
        for a,n in RANGES:r[a:a+n]=bytes((seed+i*17)&255 for i in range(n))
        return r
    for seed,old,mask,audio,tick in itertools.product((37,165),(0,0x8007C13C),(0,9),(0,1),(0,1)):
        r=fresh(seed)
        def sw(a,v):struct.pack_into('<I',r,a,v&0xFFFFFFFF)
        def sh(a,v):struct.pack_into('<H',r,a,v)
        sh(0x945E4,1);sh(0x945E6,0);sh(0x94614,9);sw(0x945F0,old);sw(0x9AFC0,0)
        sw(0x9566C,0x80130030);sw(0x130038,0x800740D0);sw(0x13003C,0x80073E28);sw(0x130044,0x80074478)
        sw(0x95674,0x80130020);sh(0x130020,mask)
        for a,v in ((0x9B27C,0x80130000),(0x9B280,0x80130001),(0x9B284,0x80130002),(0x9B288,0x80130003),(0x9B28C,0x80130004),(0x9B290,0x80150000)):sw(a,v)
        r[0x130003]=0;sh(0x1501B8,audio);sh(0x1501BA,0)
        sw(0x9B200,1);sw(0x9B224,1)
        r[0x9568C:0x956B0]=bytes(36)
        stops=Stops((0x80073C5C,0x80071A74,0x80073A44));pc=0x8007F994;initial=None;console=[]
        while True:
            stops.last=0;regs=execute(r,pc,initial_regs=initial,stop_at=stops,instruction_budget=10000)
            if not stops.last:break
            assert stops.last!=0x80073A44
            console.append((stops.last,regs[4],regs[5] if stops.last==0x80071A74 else 0))
            regs[2]=0;pc=regs[31];initial=dict(enumerate(regs))
        assert console==[(0x80073C5C,0x80011C08,0),(0x80071A74,0x80011C14,0x8009B298)] and regs[2]==1
        if tick:
            pc=0x8007440C;initial=None
            while True:
                stops.last=0;regs=execute(r,pc,initial_regs=initial,stop_at=stops,instruction_budget=20000)
                if not stops.last:break
                assert stops.last==0x80073A44
                regs[2]=1;pc=regs[31];initial=dict(enumerate(regs))
        rows.append((seed,old,mask,audio,tick,digest(r,RANGES)))
    for seed,guard in itertools.product((37,165),(1,2,0x80000000,0xFFFFFFFF)):
        r=fresh(seed);struct.pack_into('<I',r,0x9B554,guard)
        before=digest(r,RANGES);regs=execute(r,0x8007EC14)
        assert regs[2]==0 and digest(r,RANGES)==before
        guards.append((seed,guard,regs[2],before))
    lines=['/* Complete original low-level init/following tick; command rejection fixtures explicit. */',
      'static const uint32_t CDLOW_ranges[][2]={'+','.join('{'+f'0x{a:X}u,{n}u'+'}' for a,n in RANGES)+'};',
      'static const struct { uint32_t seed,old,mask,audio,tick; uint64_t hash; } CDLOW_cases[]={']
    lines+=['{'+','.join(f'{x}u' for x in row[:-1])+f',UINT64_C(0x{row[-1]:016X})'+'},' for row in rows];lines+=['};','static const struct { uint32_t seed,guard,result; uint64_t hash; } CDLOW_guards[]={']
    lines+=['{'+','.join(f'{x}u' for x in row[:-1])+f',UINT64_C(0x{row[-1]:016X})'+'},' for row in guards];lines+=['};']
    out='\n'.join(lines)+'\n';p=ROOT/'pc_port/tests/retail_cd_lowlevel_init_cases.h'
    if '--check' in sys.argv:assert p.read_text()==out
    else:p.write_text(out)
    print(f'PASS {len(rows)} original low-level init/following-tick graphs and {len(guards)} public guard graphs')
    for a,b in ((0x7F994,0x7FA2C),(0x7EC14,0x7ED58)):
        print(f'{a:X}..{b:X} {(b-a)//4} words {hashlib.sha256(ex[a-0xF800:b-0xF800]).hexdigest()}')
if __name__=='__main__':main()
