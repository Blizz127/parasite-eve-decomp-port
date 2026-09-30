#!/usr/bin/env python3
"""Original queue completion/removal with real known callbacks and restart gates."""
import itertools,struct,sys,hashlib
from pe_movie_init_oracle import load,ROOT,execute
from pe_movie_frame_oracle import digest
from pe_cd_ack_oracle import Stops
RANGES=((0xA3510,0x184),(0x9B554,0x50),(0xB0CD0,4),(0xBCD7C,4),(0x9B374,4))
def main():
    ex,_=load();rows=[];removals=[]
    def init(head,count,pattern,current,retries,cb,globalcb,lane,null):
        r=bytearray(0x200000);r[0x10000:0x10000+len(ex)-2048]=ex[2048:]
        def sw(a,v):struct.pack_into('<I',r,a,v&0xFFFFFFFF)
        for a,n in RANGES:r[a:a+n]=bytes((37+i*17)&255 for i in range(n))
        sw(0xA3600,head);sw(0xA3604,current);sw(0xA3608,count);sw(0xA3690,7)
        for i in range(8):
            a=0xA3540+((head+i)%8)*24
            seq=0 if pattern==0 else 11 if pattern==1 else 11 if i<2 else 22+i
            sw(a,seq);r[a+4]=1;sw(a+12,0x80140000);sw(a+16,(0,0x800813E8,0x80170000)[cb]);sw(a+20,retries)
        sw(0xB8AB0,(0,0x800813E8,0x80170010)[globalcb])
        sw(0x9B574,lane);sw(0x9B598,1);sw(0x9B554,1)
        sw(0xB89F4,0);sw(0xA801C,1);sw(0x9B34C,0x80130000);sw(0x130000,0x01000000)
        sw(0xC0DB8,0x80160000);sw(0xBCD7C,0xFFFFFFFF)
        for i in range(8):r[0x140000+i]=(165+i*17)&255
        return r
    for head,count,pattern,status,retries,cb,gcb,lane,null in itertools.product((0,7),(0,4),(0,1,2),(2,0x105,1),(-2,-1,0,2),(0,1,2),(0,1,2),(1,2),(0,1)):
        current=(head+1)%8
        r=init(head,count,pattern,current,retries,cb,gcb,lane,null)
        stops=Stops((0x80170000,0x80170010))
        regs=execute(r,0x8007E964,(status,0 if null else 0x80140000),stop_at=stops,instruction_budget=30000)
        b=stops.last or 0
        rows.append((head,count,pattern,current,retries&0xFFFFFFFF,cb,gcb,lane,null,status,b,regs[4] if b else 0,regs[5] if b else 0,digest(r,RANGES)))
    for head,count,pattern in itertools.product((0,7),(-1,0,1,4,8),(0,1,2)):
        r=init(head,count,pattern,5,0,0,0,2,0)
        execute(r,0x8007E5C4,instruction_budget=30000)
        removals.append((head,count&0xFFFFFFFF,pattern,digest(r,RANGES)))
    lines=['/* Original queue completion/removal graphs, known stream callback and restart gate execute. */',
      'static const uint32_t CDQUEUE_ranges[][2]={'+','.join('{'+f'0x{a:X}u,{n}u'+'}' for a,n in RANGES)+'};',
      'static const struct { uint32_t head,count,pattern,current,retries,cb,gcb,lane,null,status,boundary,arg0,arg1; uint64_t hash; } CDQUEUE_cases[]={']
    lines+=['{'+','.join(f'{x}u' for x in row[:-1])+f',UINT64_C(0x{row[-1]:016X})'+'},' for row in rows];lines+=['};','static const struct { uint32_t head,count,pattern; uint64_t hash; } CDQUEUE_removals[]={']
    lines+=['{'+','.join(f'{x}u' for x in row[:-1])+f',UINT64_C(0x{row[-1]:016X})'+'},' for row in removals];lines+=['};']
    out='\n'.join(lines)+'\n';p=ROOT/'pc_port/tests/retail_cd_queue_completion_cases.h'
    if '--check' in sys.argv:assert p.read_text()==out
    else:p.write_text(out)
    print(f'PASS {len(rows)} original completion graphs/prefixes ({sum(r[10]==0 for r in rows)} complete), {len(removals)} removals')
    for a,b in ((0x7E5C4,0x7E6B0),(0x7E964,0x7EB88)):
        print(f'{a:X}..{b:X} {(b-a)//4} words {hashlib.sha256(ex[a-0xF800:b-0xF800]).hexdigest()}')
if __name__=='__main__':main()
