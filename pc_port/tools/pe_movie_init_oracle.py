#!/usr/bin/env python3
"""Original movie initializer and completion flag; MoveImage is a call provider."""
import hashlib, struct, sys
from pe_m0000i_leaves_oracle import ROOT, find_disc, read_form1, execute
RANGES=((0xB0DBA,6),(0x1223F8,0x40),(0x1227E4,0xF0),(0xBCDC8,224))

def load():
    ex=(ROOT/'build/disc1.candidate.exe').read_bytes()
    assert hashlib.sha1(ex).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
    assert struct.unpack_from('<3H',ex,0x9315E-0xF800)==(927,965,969)
    overlay=read_form1(find_disc(ROOT),1978,4)
    assert hashlib.sha256(overlay).hexdigest()=='5ddd18d8a7f2a8180f92c1c9c072996e9705605e2daac8dc02a665a35f470ec0'
    return ex,overlay

def digest(r):
    h=14695981039346656037
    for a,n in RANGES:
        for b in r[a:a+n]:h=((h^b)*1099511628211)&0xFFFFFFFFFFFFFFFF
    return h

def main():
    ex,o=load();rows=[];flags=[]
    for mode in (0,1,2,3,255,256,257,258,0xFFFFFFFF):
        for active,missing,special,seed in ((0,0,0,37),(0,0,1,165),(0,1,0,37),(0,2,0,165),(1,0,0,37)):
            r=bytearray(0x200000);r[0x10000:0x10000+len(ex)-2048]=ex[2048:];r[0x120D00:0x122D00]=o
            for a,n in RANGES:r[a:a+n]=bytes((seed+i*17)&255 for i in range(n))
            def sw(a,v):struct.pack_into('<I',r,a,v)
            sw(0x150000,0 if missing==1 else 0x80122D00);sw(0x150004,0 if missing==2 else 0x80180000)
            r[0xB0DBA]=active;sw(0xB0CD8,0x08000000 if special else 0)
            regs=execute(r,0x801216C4,(mode,0x80150000),stop_at=(0x8007512C,));calls=[]
            while regs[31]:
                assert regs[31] in (0x801219A8,0x801219E8)
                rect=struct.unpack_from('<4h',r,regs[4]&0x1FFFFF);calls.append((*rect,regs[5],regs[6]))
                ret=regs[31];regs[2]=0
                regs=execute(r,ret,initial_regs=dict(enumerate(regs)),stop_at=(0x8007512C,))
            valid=(mode&255) in (1,2) and missing!=1 and not ((mode&255)==2 and missing==2) and not active
            assert regs[2]==int(valid)
            assert calls==([(320,0,192,256,512,0)]+([] if special else [(0,448,320,64,512,256)]) if valid else [])
            result=regs[2]; before=digest(r)
            r[0xBCDC8:0xBCEA8]=bytes([238])*224
            stops=(0x80074DC0,0x80073A44,0x80074D28,0x8007512C)
            regs=execute(r,0x80121A00,stop_at=stops);restore_calls=[]
            while regs[31]:
                ret=regs[31]
                ins=struct.unpack_from('<I',r,(ret-8)&0x1FFFFF)[0]
                fn=0x80000000|((ins&0x3FFFFFF)<<2)
                assert fn in stops
                if fn==0x8007512C:
                    rect=struct.unpack_from('<4h',r,regs[4]&0x1FFFFF)
                    restore_calls.append((fn,*rect,regs[5],regs[6]))
                else:
                    assert regs[4]==0
                    restore_calls.append((fn,))
                regs[2]=0
                regs=execute(r,ret,initial_regs=dict(enumerate(regs)),stop_at=stops)
            expected_calls=[(0x80074DC0,),(0x80073A44,),(0x80074D28,),
                            (0x8007512C,512,0,192,256,320,0)]
            if not special:expected_calls.append((0x8007512C,512,256,320,64,0,448))
            assert restore_calls==(expected_calls if valid or active else [])
            rows.append((mode,active,missing,special,seed,result,before,digest(r)))
    for mode in (0,1,2,255,256,257,0xFFFFFFFF):
        for active in (0,1,2,127,128,255):
            for prior in (0,1,165):
                r=bytearray(0x200000);r[0x120D00:0x122D00]=o;r[0xB0DBB]=active;r[0x1223F8]=prior
                execute(r,0x801223A8,(mode,));flags.append((mode,active,prior,r[0x1223F8]))
    lines=['/* Original movie init RAM; MoveImage call contract, not original GPU execution. */',
           'static const uint32_t MOVINIT_ranges[][2]={'+','.join('{'+f'0x{a:X}u,{n}u'+'}' for a,n in RANGES)+'};',
           'static const struct { uint32_t mode,active,missing,special,seed,result; uint64_t hash,restore_hash; } MOVINIT_cases[]={']
    lines += ['{'+','.join(f'{x}u' for x in row[:-2])+f',UINT64_C(0x{row[-2]:016X}),UINT64_C(0x{row[-1]:016X})'+'},' for row in rows]
    lines += ['};','static const uint32_t MOVINIT_flags[][4]={']
    lines += ['{'+','.join(f'{x}u' for x in row)+'},' for row in flags];lines+=['};']
    output='\n'.join(lines)+'\n';p=ROOT/'pc_port/tests/retail_movie_init_cases.h'
    if '--check' in sys.argv:assert p.read_text()==output
    else:p.write_text(output)
    print(f'PASS {len(rows)} original initialization/restoration pairs and {len(flags)} complete flag helpers')
if __name__=='__main__':main()
