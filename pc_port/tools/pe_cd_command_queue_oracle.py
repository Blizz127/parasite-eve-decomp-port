#!/usr/bin/env python3
"""Original command enqueue/poll graphs; dispatch and status tick providers explicit."""
import itertools,struct,sys
from pe_movie_init_oracle import load,ROOT,execute
from pe_movie_frame_oracle import digest
from pe_cd_ack_oracle import Stops
ISSUE=((0x9B53C,4),(0xA3540,0xCC),(0x130000,16))
POLL=((0xA3500,16),(0xA3610,128),(0x130000,16))
def main():
    ex,_=load()
    def fresh():
        r=bytearray(0x200000);r[0x10000:0x10000+len(ex)-2048]=ex[2048:];return r
    def sw(r,a,v):struct.pack_into('<I',r,a,v&0xFFFFFFFF)
    def run(r,entry,args,stop):
        stops=Stops((stop,));pc=entry;initial=None;calls=0
        while True:
            stops.last=0;regs=execute(r,pc,args if initial is None else (),initial_regs=initial,stop_at=stops)
            if not stops.last:return regs[2],calls
            if stop==0x8007FC64:assert regs[4]==0
            calls+=1;regs[2]=0;pc=regs[31];initial=dict(enumerate(regs))
    issue=[];poll=[]
    for prefix,param,count,sequence,head,lane in itertools.product((0,1),(0,0x80130000),(0,7,8),(0,0xFFFFFFFF),(0,7),(1,2)):
        r=fresh()
        for a,n in ISSUE:r[a:a+n]=bytes((37+i*17)&255 for i in range(n))
        sw(r,0x9B4BC+9*4,prefix);sw(r,0x9B53C,sequence);sw(r,0x9B574,lane)
        sw(r,0xA3600,head);sw(r,0xA3604,head);sw(r,0xA3608,count)
        result,calls=run(r,0x8007EE84,(0x109,param,0x12345678,0x89ABCDEF),0x8007E8F4)
        issue.append((prefix,param,count,sequence,head,lane,result,calls,digest(r,ISSUE)))
    for head,sequence,match,status,response in itertools.product((0,3,7),(0,42,43,1,0xFFFFFFFF,0x80000000),(0,7),(0,2,5),(0,0x80130000,0x800A3504)):
        r=fresh()
        for a,n in POLL:r[a:a+n]=bytes((37+i*17)&255 for i in range(n))
        sw(r,0xA3690,head)
        for i in range(8):sw(r,0xA3610+i*16,10+i);r[0xA3614+i*16]=status
        sw(r,0xA3610+match*16,42)
        result,calls=run(r,0x8007F418,(sequence,response),0x8007FC64)
        assert calls==bool(sequence)
        poll.append((head,sequence,match,status,response,result,digest(r,POLL)))
    out='/* Original command graphs; issue dispatch and poll status providers explicit. */\n'
    for name,ranges,fields,rows in (('issue',ISSUE,'prefix,param,count,sequence,head,lane,result,calls',issue),('poll',POLL,'head,sequence,match,status,response,result',poll)):
        out+=f'static const uint32_t CDQUEUE_{name}_ranges[][2]={{'+','.join('{'+f'0x{a:X}u,{n}u'+'}' for a,n in ranges)+'};\n'
        out+=f'static const struct {{ uint32_t {fields}; uint64_t hash; }} CDQUEUE_{name}[]={{\n'+''.join('{'+','.join(f'0x{v:X}u' for v in row[:-1])+f',UINT64_C(0x{row[-1]:016X})'+'},\n' for row in rows)+'};\n'
    p=ROOT/'pc_port/tests/retail_cd_command_queue_cases.h'
    if '--check' in sys.argv:assert p.read_text()==out
    else:p.write_text(out)
    print(f'PASS {len(issue)} command enqueue and {len(poll)} completion poll graphs')
if __name__=='__main__':main()
