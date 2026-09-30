#!/usr/bin/env python3
"""Original command-completion graph, stopping only at unknown callback identities."""
import itertools,struct,sys,hashlib
from pe_movie_init_oracle import load,ROOT,execute
from pe_movie_frame_oracle import digest
from pe_cd_ack_oracle import Stops
RANGES=((0x9B554,0x50),)
def main():
    ex,_=load();rows=[]
    table=struct.unpack_from('<26I',ex,0x11D0C-0xF800)
    seeds=[]
    # Public/default commands, mode transitions, errors, truncation and enable gates.
    for kind,cmd,status,flags,mode,cb in itertools.product((31,33),range(32),(2,5,0x102),(2,16),(0,128),(0,1,2)):
        seeds.append((kind,cmd,status,flags,mode,11,0,0,0,cb))
    # Every internal state/phase, timeout edge, status flag and callback gate.
    for state,phase,timer,status,flags,cb in itertools.product((11,12,13,14,15,16,17),(21,22,23,24),(300,301),(2,5),(0,2,16),(0,1,2)):
        seeds.append((32,1,status,flags,0,state,phase,timer,0,cb))
    seeds += [(32,1,2,0,0,s,0,0,1,1) for s in (16,17)]
    for values in seeds:
        kind,cmd,status,flags,mode,state,phase,timer,delay,cb=values
        r=bytearray(0x200000);r[0x10000:0x10000+len(ex)-2048]=ex[2048:]
        def sw(a,v):struct.pack_into('<I',r,a,v&0xFFFFFFFF)
        r[0x9B554:0x9B5A4]=bytes((37+i*17)&255 for i in range(0x50))
        sw(0x9B554,0 if cb==2 else 1);r[0x9B558]=cmd;r[0x9B559]=mode
        r[0x9B581]=0;sw(0x9B570,kind);sw(0x9B578,state);sw(0x9B57C,phase)
        sw(0x9B58C,timer);sw(0x9B5A0,delay);sw(0x9B624+cmd*4,1)
        for a in (0xA36A4,0xA36A8,0xA36AC):sw(a,0 if cb==0 else 0x80170000+(a-0xA36A4)*4)
        for i in range(8):r[0x140000+i]=(165+i*17)&255
        r[0x140000]=flags
        stops=Stops((0x80170000,0x80170010,0x80170020))
        regs=execute(r,0x80080164,(status,0x80140000),stop_at=stops,instruction_budget=20000)
        boundary=stops.last or 0
        rows.append((*values,boundary,regs[4] if boundary else 0,regs[5] if boundary else 0,digest(r,RANGES)))
    lines=['/* Original 80164 completion graphs; unknown callback prefixes explicit. */',
      'static const uint32_t CDCMD_table[]={'+','.join(f'0x{x:X}u' for x in table)+'};',
      'static const uint32_t CDCMD_ranges[][2]={{0x9B554u,0x50u}};',
      'static const struct { uint32_t kind,cmd,status,flags,mode,state,phase,timer,delay,cb,boundary,arg0,arg1; uint64_t hash; } CDCMD_cases[]={']
    lines+=['{'+','.join(f'{x}u' for x in row[:-1])+f',UINT64_C(0x{row[-1]:016X})'+'},' for row in rows];lines+=['};']
    out='\n'.join(lines)+'\n';p=ROOT/'pc_port/tests/retail_cd_command_cases.h'
    if '--check' in sys.argv:assert p.read_text()==out
    else:p.write_text(out)
    print(f'PASS {len(rows)} original command graphs/prefixes ({sum(r[10]==0 for r in rows)} complete)')
    for a,b in ((0x80164,0x80220),(0x80220,0x80404),(0x80404,0x8068C),(0x8068C,0x80778)):
        print(f'{a:X}..{b:X} {(b-a)//4} words {hashlib.sha256(ex[a-0xF800:b-0xF800]).hexdigest()}')
if __name__=='__main__':main()
