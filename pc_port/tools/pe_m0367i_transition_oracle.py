#!/usr/bin/env python3
"""Original CD transition component: RGB fades, dialogue94 and exit publication."""
import json,struct,sys
import pe_day2_station_routes as station
from pe_battle_hud_oracle import execute
PIN='dc0d5b09ccc503f24ed981f17c6a1cb2329fdc4723e6b1b0a5a0ef4fe7a5676e'
RANGES=((0x150000,0x2400),(0x155000,0x40),(0x156000,0x400),
        (0x9CE90,0x48),(0x9EC70,0x70),(0xBCEA8,224),(0xBCF88,128),
        (0x9CE00,4),(0x9D1A0,4),(0x9D280,4),(0xA7918,4))
def digest(r):
 h=14695981039346656037
 for a,n in RANGES:
  for b in r[a:a+n]:h=((h^b)*1099511628211)&0xFFFFFFFFFFFFFFFF
 return h

def main():
 station.PINS[367]=PIN;ex,rooms=station.load();raw,script,base,rec,chunk=rooms[367]
 lwc=lambda a:struct.unpack_from('<I',chunk,a)[0]
 packed=lwc(lwc(4)+32);directory={chunk[(packed&0x3FFFFF)+i*8+7]:lwc((packed&0x3FFFFF)+i*8+4)&0xFFFFFF for i in range(packed>>22)}
 text=0x8018EFE8+directory[1]
 rows=[]
 for bank in (0,1):
  for delay in (0,3):
   r=bytearray(0x200000);r[0x10000:0x10000+len(ex)-2048]=ex[2048:];r[0x18EFE8:0x18EFE8+len(chunk)]=chunk
   def sw(a,v):struct.pack_into('<I',r,a&0x1FFFFF,v&0xFFFFFFFF)
   def lw(a):return struct.unpack_from('<I',r,a&0x1FFFFF)[0]
   def call(a,args=()):return execute(r,a,args,instruction_budget=1000000)
   call(0x800371B0,(text,))
   actor=0x80156000;task=0x80156300
   sw(actor+0x9C,0x801AA43C);sw(actor+0xA8,task);sw(task,0x801AA8AC);sw(task+16,1)
   sw(0x9D2F0,actor);sw(0x9D1A0,0);sw(0xA7918,0xCD)
   r[0xB0DB4:0xB0DB8]=bytes([255])*4 # no channel34; initial EA200 music load excluded
   for i in range(2):sw(0xB0E38+i*4,0x80155000+i*32);sw(0xB0E44+i*4,0x80150000)
   waiting=0;states=[]
   for frame in range(300):
    sw(0x9CDDC,(frame+bank)&1)
    for i in range(2):
     for j in range(4):sw(0x155000+i*32+j*4,0xFFFFFF if j==0 else 0x155000+i*32+(j-1)*4)
    if r[0xBCEA8]==2:waiting+=1
    else:waiting=0
    sw(0x9D1F4,0x100 if waiting>delay else 0)
    sw(0x9D300,task);call(0x80017018)
    call(0x80037870);call(0x80068E24)
    states.append((lw(task),r[0xBCEA8],digest(r)))
    if lw(0x9D1A0)&0x2000:break
   else:raise AssertionError(('transition did not finish',hex(lw(task)),r[0xBCEA8]))
   assert lw(0xA7918)==0xCE and lw(0x9D280)==0xA80434C8 and lw(task)==0x801AA954
   rows.append((bank,delay,states))
 m=rec['meta'];lba=1013+rec['start']+(m&255)+(m>>8&4095)
 lines=['/* Original M0367I fade/dialogue/transfer component; music load excluded. */',f'#define M367T_LBA {lba}u',f'#define M367T_SECTORS {len(chunk)//2048}u',f'#define M367T_TEXT 0x{text:X}u','static const uint32_t M367T_ranges[][2]={']
 lines += [f'{{0x{a:X}u,{n}u}},' for a,n in RANGES];lines+=['};','static const struct { uint32_t pc; unsigned state; uint64_t hash; } M367T_frames[]={']
 for _,_,frames in rows:lines += [f'{{0x{pc:X}u,{s},UINT64_C(0x{h:016X})}},' for pc,s,h in frames]
 lines+=['};','static const struct { unsigned bank,delay,first,end; } M367T_cases[]={'];offset=0
 for b,d,frames in rows:lines.append(f'{{{b},{d},{offset},{offset+len(frames)}}},');offset+=len(frames)
 lines+=['};'];header='\n'.join(lines)+'\n';target=station.ROOT/'pc_port/tests/retail_m0367i_transition_cases.h'
 if '--check' in sys.argv:assert target.read_text()==header
 else:target.write_text(header)
 (station.ROOT/'local/live/m0367i-transition-112.json').write_text(json.dumps(dict(script_sha256=PIN,text_base=f'{text:08X}',scope='explicit actor/task and inputs; starts after EA200 music load; actual RGB fades, text94 confirmation, VM delay and destination publication; no next-room load, music or full-scene acceptance',cases=rows),indent=2)+'\n')
 print(f'PASS {len(rows)} original transition components, {offset} frame checkpoints')
if __name__=='__main__':main()
