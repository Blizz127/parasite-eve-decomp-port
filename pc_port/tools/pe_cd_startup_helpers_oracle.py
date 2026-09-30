#!/usr/bin/env python3
"""Complete original audio setup, mode setter and installed VBlank wrapper."""
import itertools,struct,sys,hashlib
from pe_movie_init_oracle import load,ROOT,execute
from pe_movie_frame_oracle import digest
RANGES=((0x130000,0x200),(0x131000,4))
def main():
    ex,_=load();audio=[];slots=[];modes=[]
    def ram():
        r=bytearray(0x200000);r[0x10000:0x10000+len(ex)-2048]=ex[2048:];return r
    for seed,left,right in itertools.product((37,165),(0,1,0xFFFF),(0,1,0xFFFF)):
        r=ram()
        for a,n in RANGES:r[a:a+n]=bytes((seed+i*17)&255 for i in range(n))
        struct.pack_into('<HH',r,0x1301B8,left,right)
        for a,v in ((0x9B290,0x80130000),(0x9B27C,0x80131000),(0x9B280,0x80131001),(0x9B284,0x80131002),(0x9B288,0x80131003)):struct.pack_into('<I',r,a,v)
        regs=execute(r,0x8007BAC0,instruction_budget=1000);assert regs[2]==0
        audio.append((seed,left,right,digest(r,RANGES)))
    for slot,old,new in itertools.product(range(8),(0,0x8007FE24,0x80170000),(0,0x8007FE24,0x80170000)):
        r=ram();r[0x9568C:0x956B0]=bytes((37+i*17)&255 for i in range(36))
        for a,v in ((0x9568C+slot*4,old),(0x9566C,0x80132000),(0x132014,0x80074478)):struct.pack_into('<I',r,a,v)
        regs=execute(r,0x80073D58,(slot,new))
        slots.append((slot,old,new,regs[2],digest(r,((0x9568C,36),))))
    for old,mode in itertools.product((0,0x12345678),(0,1,2,0xFFFFFFFF)):
        r=ram();struct.pack_into('<I',r,0x9B6B8,old);execute(r,0x800812F4,(mode,))
        modes.append((old,mode,struct.unpack_from('<I',r,0x9B6B8)[0]))
    lines=['/* Complete original startup helper cases. */','static const uint32_t CDAUDIO_ranges[][2]={{0x130000u,0x200u},{0x131000u,4u}};',
      'static const struct { uint32_t seed,left,right; uint64_t hash; } CDAUDIO_cases[]={']
    lines+=['{'+','.join(f'{x}u' for x in row[:-1])+f',UINT64_C(0x{row[-1]:016X})'+'},' for row in audio];lines+=['};','static const struct { uint32_t slot,old,next,result; uint64_t hash; } CDSLOT_cases[]={']
    lines+=['{'+','.join(f'{x}u' for x in row[:-1])+f',UINT64_C(0x{row[-1]:016X})'+'},' for row in slots];lines+=['};','static const struct { uint32_t old,mode,result; } CDMODE_cases[]={']
    lines+=['{'+','.join(f'{x}u' for x in row)+'},' for row in modes];lines+=['};']
    out='\n'.join(lines)+'\n';p=ROOT/'pc_port/tests/retail_cd_startup_helpers_cases.h'
    if '--check' in sys.argv:assert p.read_text()==out
    else:p.write_text(out)
    print(f'PASS {len(audio)} original audio setup, {len(slots)} slot wrapper, {len(modes)} mode setter graphs')
    for a,b in ((0x7BAC0,0x7BBB0),(0x73D58,0x73D88),(0x74478,0x744A4),(0x812F4,0x81310)):
        print(f'{a:X}..{b:X} {(b-a)//4} words {hashlib.sha256(ex[a-0xF800:b-0xF800]).hexdigest()}')
if __name__=='__main__':main()
