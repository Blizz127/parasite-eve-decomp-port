#!/usr/bin/env python3
"""Actual M0059I battle request/wait and full original mode6 controller."""
import json,struct,sys
import pe_day2_station_routes as station
from pe_day2_extended_routes import PINS
from pe_mode6_ready_oracle import make,RANGES as M6_RANGES
from pe_scripted_exit_oracle import words
from pe_battle_hud_oracle import execute
RANGES=M6_RANGES+((0x178000,0x1000),(0xA77F0,4))
def digest(r):
 h=14695981039346656037
 for a,n in RANGES:
  for b in r[a:a+n]:h=((h^b)*1099511628211)&0xffffffffffffffff
 return h

def main():
 station.PINS[59]=PINS[59];ex,rooms=station.load();_,script,_,rec,chunk=rooms[59]
 meta=rec['meta'];lba=1013+rec['start']+(meta&255)+(meta>>8&4095)
 common=None;patches=[];cases=[];frames=[];seen=set();summary=[]
 for bank in range(2):
  for start in (0x801A6934,0x801A6F78):
   for timer in (0,1,3):
    r,s=make(ex,bank,8);r[0x18efe8:0x18efe8+len(chunk)]=chunk
    def put(a,b):a&=0x1fffff;r[a:a+len(b)]=b;s[a:a+len(b)]=b
    def sw(a,v):put(a,struct.pack('<I',v&0xffffffff))
    def lw(a):return struct.unpack_from('<I',r,a&0x1fffff)[0]
    put(0x178000,bytes(0x1000));a=0x80178000;t=0x80178300
    sw(a+0x9c,0x801A6374);sw(a+0xa8,t);sw(t,start);sw(t+8,0x80);sw(t+16,1);sw(0x9d2f0,a);sw(0x9d28c,0);sw(0x9cdfc,t+0x40)
    for i in range(12):sw(t+0x40+i*0x40+0x24,t+0x80+i*0x40)
    sw(0xA77F0,0);put(0x144056,bytes((timer,)))
    w=words(s)
    if common is None:common=w
    pfirst=len(patches);patches.extend((i*4,v) for i,(v,c) in enumerate(zip(w,common)) if v!=c)
    first=len(frames);ready=None
    for frame in range(8):
     struct.pack_into('<I',r,0x9d300,lw(a+0xa8))
     execute(r,0x80017018,scratchpad=bytearray(0x400),visited_pcs=seen,instruction_budget=1000000)
     vm=digest(r)
     if lw(0x9d28c)==6:execute(r,0x8002BC90,scratchpad=bytearray(0x400),visited_pcs=seen,instruction_budget=1000000)
     if ready is None and lw(0x9d28c)==7:ready=frame
     frames.append((vm,digest(r),lw(t),lw(0x9d28c)))
    assert ready==timer,(start,timer,ready)
    assert lw(t)==(0x801A6A2C if start==0x801A6934 else 0x801A7070),hex(lw(t))
    assert lw(t+0x24)!=0,'follow-up task was not linked'
    cases.append((pfirst,len(patches),first,len(frames)));summary.append(dict(bank=bank,start=hex(start),timer=timer,ready_frame=ready))
 assert {0x80017FF0,0x80019154,0x8002BC90,0x8002CF24,0x800131E8}<=seen
 lines=['/* Actual M0059I request -> mode6 -> mode7 -> follow-up task; synthetic battle fixture. */',f'#define M59B_LBA {lba}u',f'#define M59B_SECTORS {len(chunk)//2048}u']
 for name,rows in (('ranges',RANGES),('common',[(i*4,v) for i,v in enumerate(common) if v]),('patches',patches)):
  lines.append(f'static const uint32_t M59B_{name}[][2]={{');lines.extend(f'{{0x{a:X}u,0x{b:X}u}},' for a,b in rows);lines.append('};')
 lines.append('static const struct { unsigned pfirst,pend,first,end; } M59B_cases[]={');lines.extend('{%s},'%','.join(map(str,c)) for c in cases);lines.append('};')
 lines.append('static const struct { uint64_t vm,controller; uint32_t pc,mode; } M59B_frames[]={');lines.extend('{UINT64_C(0x%016X),UINT64_C(0x%016X),0x%Xu,%du},'%f for f in frames);lines.append('};')
 header='\n'.join(lines)+'\n';p=station.ROOT/'pc_port/tests/retail_m0059i_battle_ready_cases.h'
 if '--check' in sys.argv:assert p.read_text()==header
 else:p.write_text(header)
 (station.ROOT/'local/live/m0059i-battle-ready-117.json').write_text(json.dumps(dict(script_sha256=script['sha256'],cases=summary,scope='actual request/wait/follow-up task and original mode6; explicit prepared battle actor and resource fixture; no full dispatcher or combat acceptance'),indent=2)+'\n')
 print(f'PASS {len(cases)} original M0059I battle readiness graphs, {len(frames)*2} checkpoints')
if __name__=='__main__':main()
