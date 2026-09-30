#!/usr/bin/env python3
"""Original updater control flow with explicit SDK/frame/wait providers."""
import itertools,struct
from pe_movie_init_oracle import load,execute
from pe_cd_ack_oracle import Stops

def main():
    ex,o=load();cases=0
    calls_at=(0x80121004,0x8010BFA0,0x8010C01C,0x80121270,0x8010C89C,0x8007C394,
              0x8007C2A0,0x8007F72C,0x8007F778,0x80080D5C,0x80081314,0x8010C0D8,
              0x8007A2A4,0x80080DC4,0x800870F0,0x80122258)
    for active,end,bank,change,retry,timeout in itertools.product((0,1,2,255),(0,1,2),(0,1),(0,2),(0,1),(0,1)):
        r=bytearray(0x200000);r[0x10000:0x10000+len(ex)-2048]=ex[2048:];r[0x120D00:0x122D00]=o
        def sw(a,v):struct.pack_into('<I',r,a,v)
        def sh(a,v):struct.pack_into('<H',r,a,v)
        r[0xB0DBA]=active;r[0xB0DBB]=1;r[0x1223F5]=end;r[0x1223F6]=3;r[0x1223F8]=change
        sw(0x1223FC,0x11223344);sw(0x9CDDC,1);sh(0xB0DBC,65535)
        sw(0x122430,0x80130000);sw(0x1228CC,0x80160000);sw(0x1228D0,0x80164000)
        r[0x1228D4]=bank;r[0x1228E0]=bank;sw(0x1228D8,0x80170000);sw(0x1228DC,0x80174000)
        sh(0x1228F8,24);sh(0x1228FA,240);r[0x1228F2]=0;r[0x1228FC]=0 if timeout else 1
        sh(0x1228EA,17);sh(0x1228EC,240)
        stops=Stops(calls_at);pc=0x80122040;initial=None;calls=[];polls=0
        while True:
            stops.last=0;regs=execute(r,pc,initial_regs=initial,stop_at=stops)
            fn=stops.last
            if not fn:break
            if fn==0x80122258:
                # Explicit counter provider tests the original expiry arm;
                # does not claim to execute eight million timing iterations.
                sw((regs[29]+16)&0x1FFFFF,1);pc=fn+4;regs[2]=1
                initial=dict(enumerate(regs));continue
            calls.append((fn,*regs[4:7]));regs[2]=0
            if fn==0x80121270:
                polls+=1;regs[2]=0 if retry and polls<=2000 else 0x80150000
            elif fn==0x8007F72C:regs[2]=1
            elif fn==0x80081314:regs[2]=1
            pc=regs[31];initial=dict(enumerate(regs))
        if active<2:assert regs[2]==0 and not calls
        else:
            assert regs[2]==(0 if end==1 else 1)
            assert r[0xB0DBA]==(active-1 if end==1 else active)
            assert struct.unpack_from('<H',r,0xB0DBC)[0]==0 and r[0x1228D4]==bank^1
            assert not r[0x1228FC] and r[0x1228F2]==timeout
            assert struct.unpack_from('<I',r,0x122414)[0]==0x11223344
            assert (0x8010BFA0,0x80160000+bank*0x4000,3) in [c[:3] for c in calls]
            assert (0x8010C01C,0x80170000+bank*0x4000,2880) in [c[:3] for c in calls]
            assert (0x8010C89C,0x80150000,0x80160000+(bank^1)*0x4000,0x80130000) in calls
            assert sum(c[0]==0x80121270 for c in calls)==(2001 if retry else 1)
            assert sum(c[0]==0x8007C2A0 for c in calls)==retry
            assert sum(c[0]==0x80080DC4 for c in calls)==(end==1)
            if end==1:assert [c[0] for c in calls[-3:]]==[0x8010C0D8,0x8007A2A4,0x80080DC4] and calls[-1][1:]==(9,0,0)
        cases+=1
    print(f'PASS {cases} original updater graphs (explicit call and timeout-counter providers)')
if __name__=='__main__':main()
