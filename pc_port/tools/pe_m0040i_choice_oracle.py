#!/usr/bin/env python3
"""Actual M0040I dialogue1 -> rendered choice -> VM reader and branch."""
import json,struct,sys
import pe_day2_station_routes as station
from pe_day2_extended_routes import PINS
from pe_battle_hud_oracle import execute
RANGES=((0x9CE90,0x48),(0x9EC70,0x70),(0xBCEA8,224),(0x150000,0x4800),(0x155000,0x40),(0x156000,0x400),(0x9CE00,4),(0x9D1A0,4),(0x9D1F4,4),(0x9D2F0,8),(0x9D300,4),(0xB0E38,24),(0x9CDDC,4),(0x9CDF0,4),(0xB8628,0x300),(0xB0CD8,0x180),(0xBCD80,0x240),(0xBCF88,0x80))
def digest(r):
 h=14695981039346656037
 for a,n in RANGES:
  for b in r[a:a+n]:h=((h^b)*1099511628211)&0xffffffffffffffff
 return h

def main():
 station.PINS[40]=PINS[40];ex,rooms=station.load();_,script,_,rec,chunk=rooms[40]
 lwc=lambda a:struct.unpack_from('<I',chunk,a)[0]
 packed=lwc(lwc(4)+32);d={chunk[(packed&0x3fffff)+i*8+7]:lwc((packed&0x3fffff)+i*8+4)&0xffffff for i in range(packed>>22)};text=0x8018efe8+d[1];assert text==0x801D0800
 common=None;patches=[];cases=[];frames=[];seen=set();summary=[]
 for choice in (0,1):
  for bank in (0,1):
   for delay in (0,4):
    r=bytearray(0x200000);r[0x10000:0x10000+len(ex)-2048]=ex[2048:];r[0x18efe8:0x18efe8+len(chunk)]=chunk
    def sw(a,v):struct.pack_into('<I',r,a&0x1fffff,v&0xffffffff)
    def lw(a):return struct.unpack_from('<I',r,a&0x1fffff)[0]
    r[0x150000:0x156400]=bytes(0x6400)
    execute(r,0x800371B0,(text,));a=0x80156000;t=0x80156300
    sw(a+0x9c,0x801c8f0c);sw(a+0xa8,t);sw(t,0x801c8fec);sw(t+16,1);sw(0x9d2f0,a);sw(0x9d1a0,0)
    for i in range(2):sw(0xb0e38+i*4,0x80155000+i*32);sw(0xb0e44+i*4,0x80150000+i*0x2400)
    seed={a+i:struct.unpack_from('<I',r,a+i)[0] for a,n in RANGES for i in range(0,n,4)}
    if common is None:common={a:v for a,v in seed.items() if v}
    pfirst=len(patches);patches.extend((a,v) for a,v in seed.items() if v!=common.get(a,0));first=len(frames);waiting=0
    for frame in range(50):
     pad=0
     if r[0xbcea8]==2:
      waiting+=1
      if waiting>delay:pad=0x20 if lw(0xbceb4)&0x200000 and r[0x9cea0]<choice else 0x100
     sw(0x9d1f4,pad);sw(0x9d300,t);sw(0x9cddc,(frame+bank)&1)
     for i in range(2):
      for j in range(4):sw(0x155000+i*32+j*4,0xffffff if not j else 0x155000+i*32+(j-1)*4)
     execute(r,0x80017018,visited_pcs=seen,instruction_budget=1000000);execute(r,0x80037870,visited_pcs=seen,instruction_budget=1000000)
     frames.append((digest(r),lw(t),pad))
     if lw(t)!=0x801c8ff8:break
    else:raise AssertionError('choice did not complete')
    assert lw(t)==(0x801c9084 if choice==0 else 0x801c9558)
    assert lw(a+0xAC+4*4)==choice and r[0x9cea4]==choice and r[0xbcea8]==0
    cases.append((choice,bank,delay,pfirst,len(patches),first,len(frames)));summary.append(dict(choice=choice,bank=bank,delay=delay,frames=frame+1,task=hex(lw(t))))
 assert {0x80017018,0x80017410,0x800177C8,0x80017DE4,0x80037870,0x800868AC}<=seen
 m=rec['meta'];lba=1013+rec['start']+(m&255)+(m>>8&4095)
 lines=['/* Actual M0040I text/VM choice component; explicit actor/draw fixture. */',f'#define M40C_LBA {lba}u',f'#define M40C_SECTORS {len(chunk)//2048}u']
 for name,rows in (('ranges',RANGES),('common',sorted(common.items())),('patches',patches)):
  lines.append(f'static const uint32_t M40C_{name}[][2]={{');lines.extend(f'{{0x{a:X}u,0x{b:X}u}},' for a,b in rows);lines.append('};')
 lines.append('static const struct { unsigned choice,bank,delay,pfirst,pend,first,end; } M40C_cases[]={');lines.extend('{%s},'%','.join(map(str,c)) for c in cases);lines.append('};')
 lines.append('static const struct { uint64_t hash; uint32_t pc,pad; } M40C_frames[]={');lines.extend('{UINT64_C(0x%016X),0x%Xu,0x%Xu},'%f for f in frames);lines.append('};')
 header='\n'.join(lines)+'\n';p=station.ROOT/'pc_port/tests/retail_m0040i_choice_cases.h'
 if '--check' in sys.argv:assert p.read_text()==header
 else:p.write_text(header)
 (station.ROOT/'local/live/m0040i-choice-124.json').write_text(json.dumps(dict(script_sha256=script['sha256'],text=hex(text),cases=summary,scope='actual dialogue1 and VM choice reader/branch through next wait; no scene construction or branch continuation'),indent=2)+'\n')
 print(f'PASS {len(cases)} original M0040I choice graphs, {len(frames)} frame checkpoints')
if __name__=='__main__':main()
