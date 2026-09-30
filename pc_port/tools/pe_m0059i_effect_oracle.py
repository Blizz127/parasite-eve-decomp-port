#!/usr/bin/env python3
"""Actual M0059I effect loop: full VM, allocation and effect update/draw."""
import json,struct,sys
import pe_day2_station_routes as station
from pe_day2_extended_routes import PINS
from pe_battle_hud_oracle import execute
RANGES=((0x9CDB4,4),(0x9CDD8,8),(0x9CDF8,4),(0x9CE00,4),(0x9D2F0,4),(0x9D300,4),(0x9EC70,0x70),(0xA7834,4),(0xB0E38,8),(0xB0E58,8),(0xBCFA4,8),(0xE21A4,4),(0xE2800,4),(0x150000,0x1000),(0x152000,0x2000),(0x155000,0x200),(0x156000,0x400))
def digest(r,s):
 h=14695981039346656037
 for a,n in RANGES:
  for b in r[a:a+n]:h=((h^b)*1099511628211)&0xffffffffffffffff
 for b in s[:0x38]:h=((h^b)*1099511628211)&0xffffffffffffffff
 return h

def main():
 station.PINS[59]=PINS[59];ex,rooms=station.load();raw,script,base,rec,chunk=rooms[59]
 meta=rec['meta'];lba=1013+rec['start']+(meta&255)+(meta>>8&4095)
 common=None;patches=[];cases=[];frames=[];seen=set();summaries=[]
 for bank in (0,1):
  for camera in (4612,5488):
   r=bytearray(0x200000);r[0x10000:0x10000+len(ex)-2048]=ex[2048:];r[0x18EFE8:0x18EFE8+len(chunk)]=chunk;s=bytearray(0x400)
   def sw(a,v):struct.pack_into('<I',r,a&0x1fffff,v&0xffffffff)
   def sh(a,v):struct.pack_into('<H',r,a&0x1fffff,v&65535)
   def lw(a):return struct.unpack_from('<I',r,a&0x1fffff)[0]
   for a,n in RANGES:r[a:a+n]=bytes(n)
   sw(0xB0E38,0x80152000);sw(0xB0E3C,0x80153000);sw(0xB0E58,0x80150000);sw(0xB0E5C,0x80150800)
   sw(0xBCFA4,0x80155000);sw(0xBCFA8,0x80155040);sw(0x155040,256);sw(0xE2800,0x80155100)
   for i in (0,8,16):sh(0x155000+i,4096)
   sw(0x155014,1488);sw(0x155018,350);sw(0x15501c,camera)
   actor=0x80156000;task=0x80156300
   sw(actor+0x9C,0x8019FD70);sw(actor+0xA8,task);sw(task,0x8019FDC4);sw(task+16,1);sw(0x9D2F0,actor);sw(0x9D300,task)
   seed={a+i:struct.unpack_from('<I',r,a+i)[0] for a,n in RANGES for i in range(0,n,4)}
   if common is None:common={a:v for a,v in seed.items() if v}
   pfirst=len(patches);patches.extend((a,v) for a,v in seed.items() if v!=common.get(a,0));first=len(frames);counts=[];spawns=[]
   for frame in range(200):
    sw(0x9CDDC,(bank+frame)&1);sw(0x9CDD8,0)
    for i in range(0,0x2000,4):sw(0x152000+i,0xABFFFFFF)
    before=struct.unpack_from('<H',r,0xE21A4)[0]
    sw(0x9D300,task)
    execute(r,0x80017018,scratchpad=s,visited_pcs=seen)
    after=struct.unpack_from('<H',r,0xE21A4)[0]
    if after>before:spawns.append(frame)
    execute(r,0x800E01BC,scratchpad=s,visited_pcs=seen,initial_cop_control={24:160<<16,25:112<<16,26:256})
    count=struct.unpack_from('<H',r,0xE21A4)[0];counts.append(count)
    frames.append((digest(r,s),lw(task),count,lw(0x9CDD8)))
   assert len(spawns)>=7 and max(counts)>=2 and any(a>b for a,b in zip(counts,counts[1:])),(spawns,counts)
   cases.append((bank,pfirst,len(patches),first,len(frames)))
   summaries.append(dict(bank=bank,camera_z=camera,spawn_frames=spawns,max_active=max(counts)))
 assert {0x80016910,0x800E00CC,0x800E01BC,0x800E03A0,0x800E051C}<=seen
 lines=['/* Actual M0059I VM loop and full effect graph; explicit camera/actor fixture. */',f'#define M59E_LBA {lba}u',f'#define M59E_SECTORS {len(chunk)//2048}u']
 for name,rows in (('ranges',RANGES),('common',sorted(common.items())),('patches',patches)):
  lines.append(f'static const uint32_t M59E_{name}[][2]={{');lines.extend(f'{{0x{a:X}u,0x{b:X}u}},' for a,b in rows);lines.append('};')
 lines.append('static const struct { unsigned bank,pfirst,pend,first,end; } M59E_cases[]={');lines.extend('{%s},'%','.join(map(str,c)) for c in cases);lines.append('};')
 lines.append('static const struct { uint64_t hash; uint32_t pc,count,bytes; } M59E_frames[]={');lines.extend('{UINT64_C(0x%016X),0x%Xu,%du,%du},'%f for f in frames);lines.append('};')
 header='\n'.join(lines)+'\n';p=station.ROOT/'pc_port/tests/retail_m0059i_effect_cases.h'
 if '--check' in sys.argv:assert p.read_text()==header
 else:p.write_text(header)
 (station.ROOT/'local/live/m0059i-effect-116.json').write_text(json.dumps(dict(script_sha256=script['sha256'],cases=summaries,scope='actual repeating script component; synthetic camera/actor/pools; no full scene or GPU output'),indent=2)+'\n')
 print(f'PASS {len(cases)} original M0059I effect loops, {len(frames)} frame checkpoints');print(summaries)
if __name__=='__main__':main()
