#!/usr/bin/env python3
"""Complete original queue cancellation/completion graphs and unknown callback prefixes."""
import itertools,struct,sys
from pe_movie_init_oracle import load,ROOT,execute
from pe_movie_frame_oracle import digest
RANGES=((0xA3540,0x154),(0xB0CD0,4),(0xBCD7C,4),(0x9B374,4))

def main():
    ex,o=load();rows=[];writers=[]
    def ram():
        r=bytearray(0x200000);r[0x10000:0x10000+len(ex)-2048]=ex[2048:]
        for a,n in RANGES:r[a:a+n]=bytes((37+i*17)&255 for i in range(n))
        for i in range(8):r[0x140000+i]=(165+i*17)&255
        return r
    def sw(r,a,v):struct.pack_into('<I',r,a,v&0xFFFFFFFF)
    for count,head,ring,pattern,callbacks,null in itertools.product((-1,0,1,4,8),(0,7),(0,7),(0,1,2),(0,1,2),(0,1)):
        r=ram();sw(r,0xA3600,head);sw(r,0xA3604,5);sw(r,0xA3608,count);sw(r,0xA3690,ring)
        seqs=[];targets=[]
        for i in range(8):
            seq=(11 if pattern==0 else (0,0,11,11,0,22,0,0)[i] if pattern==1 else (1,2,1,2,1,2,1,2)[i])
            target=(0 if callbacks==0 else 0x800813E8 if callbacks==1 or i==0 else 0x80170000)
            seqs.append(seq);targets.append(target)
            addr=0xA3540+((head+i)%8)*24;sw(r,addr,seq);sw(r,addr+16,target)
        sw(r,0xB89F4,0);sw(r,0xA801C,1);sw(r,0x9B34C,0x80130000);sw(r,0x130000,0x01000000)
        sw(r,0xC0DB8,0x80160000);sw(r,0xBCD7C,0xFFFFFFFF)
        response=0 if null else 0x80140000
        regs=execute(r,0x8007E704,(0x105,response),stop_at=(0x80170000,),instruction_budget=200000)
        boundary=bool(regs[31]);arg0=arg1=0
        if boundary:
            assert regs[31]==0x8007E8B8;arg0,arg1=regs[4:6];assert(arg0,arg1)==(5,response)
        rows.append((count&0xFFFFFFFF,head,ring,pattern,callbacks,null,int(boundary),arg0,arg1,digest(r,RANGES)))
    for ring,status,null in itertools.product((0,1,7),(0,0x105,0xFFFFFFFF),(0,1)):
        r=ram();sw(r,0xA3690,ring)
        execute(r,0x8007EB88,(0x12345678,status,0 if null else 0x80140000))
        writers.append((ring,status,null,digest(r,RANGES)))
    lines=['/* Complete original queue/callback graphs, except explicit unknown callback prefixes. */',
      'static const uint32_t CDREC_ranges[][2]={'+','.join('{'+f'0x{a:X}u,{n}u'+'}' for a,n in RANGES)+'};',
      'static const struct { uint32_t count,head,ring,pattern,callbacks,null,boundary,arg0,arg1; uint64_t hash; } CDREC_cases[]={']
    lines+=['{'+','.join(f'{x}u' for x in row[:-1])+f',UINT64_C(0x{row[-1]:016X})'+'},' for row in rows];lines+=['};','static const struct { uint32_t ring,status,null; uint64_t hash; } CDREC_writers[]={']
    lines+=['{'+','.join(f'{x}u' for x in row[:-1])+f',UINT64_C(0x{row[-1]:016X})'+'},' for row in writers];lines+=['};']
    out='\n'.join(lines)+'\n';p=ROOT/'pc_port/tests/retail_cd_queue_recovery_cases.h'
    if '--check' in sys.argv:assert p.read_text()==out
    else:p.write_text(out)
    print(f'PASS {len(rows)} original cancellation graphs/prefixes ({sum(r[6]==0 for r in rows)} complete) and {len(writers)} completion writers')
if __name__=='__main__':main()
