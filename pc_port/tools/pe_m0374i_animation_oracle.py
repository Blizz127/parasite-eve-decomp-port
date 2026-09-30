#!/usr/bin/env python3
"""Real M0374I send -> queue drain -> receiver VM -> real model animation through wait release."""
import hashlib, json, struct, sys
import pe_day2_station_routes as station
from pe_day2_station_return import PINS
from pe_battle_hud_oracle import execute
ROOT=station.ROOT
CHUNK=0x8018EFE8
RANGES=((0x150000,0x600),(0x160000,0x180),(0x9CDB4,1),(0x9CDFC,8),
        (0x9D300,4),(0x9D308,2),(0x9DF70,8),(0xA3180,12),(0xA7918,4),(0xBCEA8,224),(0x9CEA0,8),(0x9D1CC,2),(0x9D1D8,4),(0x9D1FC,4),(0x170000,0x18000),(0xB1638,32),(0x9CDDC,4),(0x94488,160),(0xB0CE8,4))
def digest(r,ranges):
 h=14695981039346656037
 for a,n in ranges:
  for b in r[a:a+n]:h=((h^b)*1099511628211)&0xFFFFFFFFFFFFFFFF
 return h

def main():
 station.PINS.update(PINS);ex,rooms=station.load();raw,script,base,rec,chunk=rooms[374]
 ranges=RANGES+((CHUNK&0x1FFFFF,len(chunk)),)
 meta=rec['meta'];lba=1013+rec['start']+(meta&255)+(meta>>8&4095)
 rows=[];seen=set()
 for packets in (0,1):
  for speed in (0x8000,0x10000,0x18000,0x20000):
   story=0xE4;serial=0
   r=bytearray(0x200000);r[0x10000:0x10000+len(ex)-0x800]=ex[0x800:]
   r[CHUNK&0x1FFFFF:(CHUNK&0x1FFFFF)+len(chunk)]=chunk
   def sw(a,v):struct.pack_into('<I',r,a&0x1FFFFF,v&0xFFFFFFFF)
   def sh(a,v):struct.pack_into('<H',r,a&0x1FFFFF,v&65535)
   def lw(a):return struct.unpack_from('<I',r,a&0x1FFFFF)[0]
   def call(a,args=(),**kw):execute(r,a,args,visited_pcs=seen,instruction_budget=1000000,scratchpad=bytearray(0x400),**kw)
   sender=0x80150000;actor=0x80150200;task=0x80160000;mail=task+0x40;child=task+0x80
   sw(sender+4,actor);sh(sender+0x24,0x1234);sw(sender+0x9C,0x801C4E04)
   r[0x15020C]=2;sw(actor+0x9C,0x801C5840);sw(actor+0xA8,0)
   sw(0x8009D20C,sender);sw(0x8009D2F0,actor);sw(0x8009D1A0,0)
   r[0xB0CE9]=0
   for pc in (0x801C5840,0x801C5860):
    for i in range(6):sw(0x80154020+i*4,pc+8+i*4 if i<5 else 0x800B6AC4)
    call(0x80015DAC,(0x80154020,))
   assert r[0xB0CE9]==2
   assert r[0x944A8:0x944B8]==struct.pack('<4B2H4B2H',2,0,1,0,0x3F8,0x3F8,2,0,1,18,0x3F9,0x3F9)
   sw(0x80154000,0x801C58C4);sw(0x80154004,0x801C58C8)
   call(0x80017588,(0x80154000,));assert lw(actor+0x19C)==0x801C6354
   sw(task,0x801C54B8);sw(task+16,1);sw(sender+0xA8,task)
   sw(mail+0x24,child);sw(child+0x24,task+0xC0);sw(0x8009CDFC,mail)
   sh(0x8009D308,serial);r[0x9CDB4]=0;sw(0x800A7918,story)
   header=CHUNK+lw(CHUNK+4)
   execute(r,0x8006B7B0,initial_regs={18:0,20:CHUNK,21:header,22:0x800B0CD8},stop_at=(0x8006B898,))
   obj=lw(0x800B0E78);assert obj==0x8019690C
   call(0x8003D050,(actor+0x1B4,obj,0x80170000,0,0,0,0,0,0,packets))
   mesh=CHUNK+(lw(CHUNK+(lw(header+24)&0x3FFFFF)+4)&0xFFFFFF)
   sw(0x800B1620,mesh);call(0x8001A918)
   sw(actor+0x1C,speed)
   hashes=[digest(r,ranges)]
   sw(0x8009D2F0,sender);sw(0x8009D300,task);call(0x80017018);call(0x80065400)
   hashes.append(digest(r,ranges));assert lw(actor+0xA8)==mail and lw(mail+20)==1
   sw(0x8009D2F0,actor);sw(0x8009D300,mail);call(0x80017018)
   for i in range(9):sh(0x800BEA40+i*2,4096 if i%4==0 else 0)
   sw(0x8009CDA0,0x00808080)
   def pose():call(0x8003D834,(actor+0x1B4,lw(actor+0x1B0),lw(actor+0x14)>>16,0x800BEA40),initial_cop_control={16:4096,18:4096,20:4096})
   pose();hashes.append(digest(r,ranges))
   assert lw(actor+0xBC)==1 and lw(child)==0x801C5D94 and lw(child+16)==1, [hex(lw(a)) for a in (actor+0xBC,child,child+16,actor+0xA8,0x8009CDFC,task,mail)]
   assert lw(actor+0xA8)==mail and lw(mail+0x24)==child
   assert lw(child+0x24)==(0)
   assert lw(0x800A7918)==0xE6 and r[0xBCEA8]==1
   assert struct.unpack_from("<H",r,0xBCEB8)[0]==0x62
   assert lw(actor+0x1B0)==0x801B88C0 and r[(actor+15)&0x1FFFFF]==122
   assert struct.unpack_from('<H',r,(actor+0x12)&0x1FFFFF)[0]==122
   ticks=0
   while lw(child)==0x801C5D94:
    call(0x8001A4AC,(actor,))
    sw(0x8009D300,child);call(0x80017018)
    ticks+=1;assert ticks<=244
   assert ticks==(122*65536+speed-1)//speed and lw(child)==0x801C5D9C
   assert lw(actor+0x14)==122<<16
   pose();hashes.append(digest(r,ranges))
   sw(0x8009D300,child);call(0x80017018)
   assert lw(child)==0x801C5DC0 and r[0xBCEA8]==1
   assert [lw(actor+0x28+i*4) for i in range(3)]==[0x13A0000,0x530000,0x32D0000], [hex(lw(actor+0x28+i*4)) for i in range(3)]
   hashes.append(digest(r,ranges))
   rows.append((packets,speed,ticks,hashes))
 assert {0x80017588,0x80017764,0x80065400,0x800177AC,0x800131E8,0x80012700,0x8003D050,0x8003D834,0x8001A4AC,0x8001AA78,0x80037548}<=seen
 lines=['/* Original M0374I real model/mesh and animation wait. */',
        f'#define M374A_CHUNK_LBA {lba}u',f'#define M374A_CHUNK_SECTORS {len(chunk)//2048}u',
        'static const uint32_t M374A_ranges[][2]={']
 lines += [f'{{0x{a:X}u,{n}u}},' for a,n in ranges]
 lines += ['};','static const struct { unsigned packets,speed,ticks; uint64_t hash[5]; } M374A_cases[]={']
 lines += ['{%d,%d,%d,{%s}},'%(p,s,t,','.join('UINT64_C(0x%016X)'%h for h in hs)) for p,s,t,hs in rows]
 lines += ['};'];header='\n'.join(lines)+'\n';target=ROOT/'pc_port/tests/retail_m0374i_animation_cases.h'
 if '--check' in sys.argv:assert target.read_text()==header
 else:target.write_text(header)
 (ROOT/'local/live/m0374i-animation-108.json').write_text(json.dumps(dict(script_sha256=script['sha256'],chunk_sha256=hashlib.sha256(chunk).hexdigest(),scope='original model init, mesh relocation, real sender/receiver VM, clipE frame0/122 full poses and tick/wait sequence into placement and dialogue62 poll; explicit actor/task/allocation/speed/lighting inputs, no GPU, dialogue completion or preceding scene proof',cases=rows),indent=2)+'\n')
 print(f'PASS {len(rows)} original real-model animation graphs')
if __name__=='__main__':main()
