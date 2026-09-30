#!/usr/bin/env python3
"""Movie display setup and frame acquisition; original GPU/audio calls are providers."""
import struct,sys
from pe_movie_init_oracle import load,ROOT,execute
DRANGES=((0xB9F00,0x6300),(0x1223F4,8))
RANGES=((0x140000,0x100),(0x150000,64),(0x151000,32),(0x1223F4,0x30),
        (0x1227E4,8),(0x9D1C8,4),(0xBE9EC,4))

def digest(r,ranges):
    h=14695981039346656037
    for a,n in ranges:
        for b in r[a:a+n]:h=((h^b)*1099511628211)&0xFFFFFFFFFFFFFFFF
    return h

def main():
    ex,o=load();displays=[];frames=[]
    def ram():
        r=bytearray(0x200000);r[0x10000:0x10000+len(ex)-2048]=ex[2048:];r[0x120D00:0x122D00]=o;return r
    def sw(r,a,v):struct.pack_into('<I',r,a,v&0xFFFFFFFF)
    def sh(r,a,v):struct.pack_into('<H',r,a,v&65535)
    for bank in (0,1,256,257,0xFFFFFFFF,127,128):
        for wide in (0,1,256,257,0xFFFFFFFF):
            for std in (0,1):
                for seed in (37,165):
                    r=ram()
                    for a,n in DRANGES:r[a:a+n]=bytes((seed+i*17)&255 for i in range(n))
                    sw(r,0x956EC,std);execute(r,0x80121004,(bank,wide))
                    displays.append((bank,wide,std,seed,digest(r,DRANGES)))
    specs=[]
    for wide in (0,1):
        for width,height in ((320,240),(160,120),(32768,65535)):
            for frame,limit,last in ((0,100,65535),(80,100,79),(84,100,84),(90,100,90),(98,100,97),(100,100,99),(101,100,102),(0,0,65535),(0,-1,65535),(0xFFFFFFFF,-32768,0)):
                specs.append((wide,width,height,frame,limit,last,0,0))
                specs.append((wide,width,height,frame,limit,last,1,0))
    specs += [(0,320,240,0,100,65535,1,1),(1,320,240,0,100,65535,1,1)]
    for wide,width,height,frame,limit,last,same,timeout in specs:
        r=ram()
        for a,n in RANGES:r[a:a+n]=bytes(n)
        for i in range(64):r[0x140000+i]=(37+i*17)&255
        sw(r,0x9B574,2);sw(r,0xC0DC8,0x80150000);sw(r,0xC20C4,2);sw(r,0xBE9EC,0)
        sh(r,0x150000,0 if timeout else 2);sw(r,0x150008,frame);sh(r,0x150010,width);sh(r,0x150012,height)
        sw(r,0x1227E4,0x80151000);sh(r,0x151008,limit);sh(r,0x1227E8,last)
        sh(r,0x122418,width if same else 1);sh(r,0x12241A,height if same else 1)
        r[0xB0DBB]=wide;r[0xB0DBE]=152;sw(r,0x9D2C0,2)
        regs=execute(r,0x80121270,(0x80140000,),stop_at=(0x80074F44,0x8007A88C),instruction_budget=200000)
        clears=0
        while regs[31]:
            ret=regs[31];ins=struct.unpack_from('<I',r,(ret-8)&0x1FFFFF)[0];fn=0x80000000|((ins&0x3FFFFFF)<<2)
            if fn==0x80074F44:
                assert struct.unpack_from('<4h',r,regs[4]&0x1FFFFF)==(0,0,480 if wide else 320,480)
                assert regs[5:8]==[0,0,0];clears+=1
            else:assert fn==0x8007A88C and regs[4]==0x8009D1C8
            regs[2]=0;regs=execute(r,ret,initial_regs=dict(enumerate(regs)),stop_at=(0x80074F44,0x8007A88C),instruction_budget=200000)
        assert regs[2]==(0 if timeout else 0x80150040)
        frames.append((wide,width,height,frame,limit&65535,last,same,timeout,regs[2],clears,digest(r,RANGES)))
    lines=['/* Original movie display graphs and frame graphs with GPU/audio providers. */']
    for label,ranges in (('display',DRANGES),('frame',RANGES)):
        lines.append('static const uint32_t MOVFRAME_'+label+'_ranges[][2]={'+','.join('{'+f'0x{a:X}u,{n}u'+'}' for a,n in ranges)+'};')
    lines+=['static const struct { uint32_t bank,wide,std,seed; uint64_t hash; } MOVFRAME_display[]={']
    lines+=['{'+','.join(f'{v}u' for v in row[:-1])+f',UINT64_C(0x{row[-1]:016X})'+'},' for row in displays];lines+=['};']
    lines+=['static const struct { uint32_t wide,width,height,frame,limit,last,same,timeout,result,clears; uint64_t hash; } MOVFRAME_cases[]={']
    lines+=['{'+','.join(f'{v}u' for v in row[:-1])+f',UINT64_C(0x{row[-1]:016X})'+'},' for row in frames];lines+=['};']
    output='\n'.join(lines)+'\n';p=ROOT/'pc_port/tests/retail_movie_frame_cases.h'
    if '--check' in sys.argv:assert p.read_text()==output
    else:p.write_text(output)
    print(f'PASS {len(displays)} complete original display graphs and {len(frames)} frame graphs (GPU/audio providers)')
if __name__=='__main__':main()
