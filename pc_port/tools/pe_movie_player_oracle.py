#!/usr/bin/env python3
"""Original movie player 121C04 control flow with explicit SDK/search/frame providers."""
import itertools,struct
from pe_movie_init_oracle import load,execute
from pe_cd_ack_oracle import Stops

def strcat(r,dst,src):
    end=dst
    while r[end]:end+=1
    while r[src]:r[end]=r[src];end+=1;src+=1
    r[end]=0

def main():
    ex,o=load();cases=0
    calls_at=(0x80121004,0x800719F4,0x80081414,0x8007F72C,0x8007F778,0x80080D5C,
              0x80081314,0x8010BE3C,0x8010C0D8,0x8007A214,0x8007C304,0x800870F0,
              0x8010BD4C,0x80121270,0x8010C89C,0x8007C394)
    for vid in (0x2F,0x30,0x8000012F,0xFFFF):
        r=bytearray(0x200000);r[0x10000:0x10000+len(ex)-2048]=ex[2048:];r[0x120D00:0x122D00]=o
        r[0xB0DBF]=0xA5
        regs=execute(r,0x80121C04,((vid&0xFFFFFFFF),))
        assert regs[2]==0 and r[0xB0DBF]==0xA5,f'early {vid:#x}'
        cases+=1
    for vid,wide,failfirst in itertools.product((0,0x14,0x15,0x2E),(0,1),(0,1)):
        r=bytearray(0x200000);r[0x10000:0x10000+len(ex)-2048]=ex[2048:];r[0x120D00:0x122D00]=o
        def sw(a,v):struct.pack_into('<I',r,a,v)
        def sh(a,v):struct.pack_into('<H',r,a,v)
        sw(0x120FF4,0x80130060);sw(0x120FFC,0x80130060)
        r[0x130060:0x130068]=b'\\MV\\'+bytes(2)
        record=0x122438+vid*20
        r[0x130070:0x13007C]=b'\\A.STR;1'+bytes(1)
        sw(record,0x80130070);r[record+4]=wide
        sh(record+6,0x1234);sh(record+0xA,0x5678);sh(record+0xC,0x300)
        sw(0x122420,0x80160000);sw(0x122424,0x80164000)
        sw(0x122428,0x80170000);sw(0x12242C,0x80174000)
        sw(0x122430,0x80130000);sw(0x122434,0x80190000)
        r[0xB0DBA]=3;sh(0xB0DBC,0);sw(0x9CDDC,0x100);r[0xB0DBE]=0x98
        stops=Stops(calls_at);pc=0x80121C04;initial={4:vid&0xFFFFFFFF};calls=[];searched=0
        while True:
            stops.last=0;regs=execute(r,pc,initial_regs=initial,stop_at=stops)
            fn=stops.last
            if not fn:break
            if fn==0x800719F4:
                strcat(r,regs[4]&0x1FFFFF,regs[5]&0x1FFFFF)
            elif fn==0x80081414:
                searched+=1
                sw(regs[4]&0x1FFFFF,0x4321)
                regs[2]=0 if (failfirst and searched==1) else 0x1234
            elif fn==0x8007F72C:regs[2]=1
            elif fn==0x8007F778:regs[2]=0
            elif fn==0x80080D5C or fn==0x80081314:regs[2]=1
            elif fn==0x80121270:regs[2]=0x80150000
            else:regs[2]=0
            calls.append((fn,*regs[4:8]))
            pc=regs[31];initial=dict(enumerate(regs))
        assert regs[2]==1 and r[0xB0DBF]==(vid&255)
        assert struct.unpack_from('<I',r,0x1227E4)[0]==record+0x80000000
        assert r[0xB0DBB]==wide
        assert (0x80121004,0,wide) in [c[:3] for c in calls]
        assert (0x80121004,1,wide) in [c[:3] for c in calls]
        assert r[0x1223FC:0x122400]==b'\x21\x43\x00\x00'
        assert r[0x122414:0x122418]==b'\x21\x43\x00\x00'
        assert searched==(2 if failfirst else 1)
        assert struct.unpack_from('<I',r,0x1228CC)[0]==0x80160000
        assert struct.unpack_from('<I',r,0x1228D0)[0]==0x80164000
        assert struct.unpack_from('<I',r,0x1228D8)[0]==0x80170000
        assert struct.unpack_from('<I',r,0x1228DC)[0]==0x80174000
        assert r[0x1228D4]==1 and not r[0x1228E0]
        assert struct.unpack_from('<H',r,0x1228E2)[0]==0x5678
        assert struct.unpack_from('<H',r,0x1228EA)[0]==0x5678
        assert struct.unpack_from('<H',r,0x1228E4)[0]==0x3F0
        assert struct.unpack_from('<H',r,0x1228EC)[0]==0x300
        assert struct.unpack_from('<H',r,0x1228F4)[0]==0x5678
        assert struct.unpack_from('<H',r,0x1228F6)[0]==0x3F0
        assert struct.unpack_from('<H',r,0x1228F8)[0]==(0x18 if wide else 0x10)
        assert r[0x1228F2]==0 and not r[0x1228FC]
        assert r[0x1223F5]==0 and r[0xB0DBA]==4
        assert struct.unpack_from('<H',r,0xB0DBC)[0]==1
        assert (0x8010C0D8,0x801214D4) in [c[:2] for c in calls]
        assert (0x8007A214,0x80190000,0x40) in [c[:3] for c in calls]
        assert (0x8007C304,1,0x1234,0xFFFFFFFF,0) in [c[:5] for c in calls]
        assert (0x800870F0,0x98) in [c[:2] for c in calls]
        assert (0x8010BD4C,0x80130000) in [c[:2] for c in calls]
        assert (0x8010C89C,0x80150000,0x80164000,0x80130000) in [c[:4] for c in calls]
        assert (0x8007C394,0x80150000) in [c[:2] for c in calls]
        assert sum(c[0]==0x80080D5C for c in calls)==1
        assert sum(c[0]==0x80081314 for c in calls)==1
        cases+=1
    print(f'PASS {cases} original player graphs (explicit SDK/search/frame providers)')
if __name__=='__main__':main()
