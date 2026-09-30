#!/usr/bin/env python3
"""Original M0239I dialogue7C/7D first page, with explicit window setup."""
import json,struct,sys
import pe_day2_station_routes as station
from pe_day2_extended_routes import PINS
from pe_battle_hud_oracle import execute
RANGES=((0x9CE90,0x48),(0x9EC70,0x70),(0xBCEA8,224),(0x150000,0x2400),(0x155000,16))
def digest(r):
 h=14695981039346656037
 for a,n in RANGES:
  for b in r[a:a+n]:h=((h^b)*1099511628211)&0xFFFFFFFFFFFFFFFF
 return h

def main():
 station.PINS.update(PINS);ex,rooms=station.load();raw,script,base,rec,chunk=rooms[239]
 lwc=lambda a:struct.unpack_from('<I',chunk,a)[0]
 packed=lwc(lwc(4)+32);entries=[(chunk[(packed&0x3FFFFF)+i*8+7],lwc((packed&0x3FFFFF)+i*8+4)&0xFFFFFF) for i in range(packed>>22)]
 text=0x8018EFE8+dict(entries)[1];assert text==0x801E465C
 rows=[]
 for message in (0x7C,0x7D):
  for bank in (0,1):
   for background in (0,1):
    r=bytearray(0x200000);r[0x10000:0x10000+len(ex)-2048]=ex[2048:];r[0x18EFE8:0x18EFE8+len(chunk)]=chunk
    def sw(a,v):struct.pack_into('<I',r,a&0x1FFFFF,v&0xFFFFFFFF)
    execute(r,0x800371B0,(text,));sw(0x9CDDC,bank)
    sw(0xB0E38+bank*4,0x80155000);sw(0xB0E44+bank*4,0x80150000)
    for i in range(4):sw(0x155000+i*4,0xFFFFFF if i==0 else 0x155000+(i-1)*4)
    struct.pack_into('<H',r,0x156000,0xFFFF)
    execute(r,0x800375E0,(message,0,0x80156000))
    sw(0x156020,0x80156040);sw(0x156024,0x80156044);sw(0x156040,2300);sw(0x156044,background)
    execute(r,0x80016910,(0x80156020,))
    execute(r,0x80037870,instruction_budget=1000000)
    assert r[0xBCEA8]==2
    rows.append((message,bank,background,digest(r)))
 m=rec['meta'];lba=1013+rec['start']+(m&255)+(m>>8&4095)
 lines=['/* Original M0239I first-page dialogue, requires original disc. */',f'#define M239D_LBA {lba}u',f'#define M239D_SECTORS {len(chunk)//2048}u','static const uint32_t M239D_ranges[][2]={']
 lines += [f'{{0x{a:X}u,{n}u}},' for a,n in RANGES]
 lines += ['};','static const struct { unsigned message,bank,background; uint64_t hash; } M239D_cases[]={']
 lines += [f'{{{m},{b},{v},UINT64_C(0x{h:016X})}},' for m,b,v,h in rows];lines+=['};']
 header='\n'.join(lines)+'\n';target=station.ROOT/'pc_port/tests/retail_m0239i_dialogue_cases.h'
 if '--check' in sys.argv:assert target.read_text()==header
 else:target.write_text(header)
 (station.ROOT/'local/live/m0239i-dialogue-111.json').write_text(json.dumps(dict(script_sha256=script['sha256'],text_base=f'{text:08X}',scope='real first-page text to state2 and packets; explicit init/window/arena/ED binding; no GPU or confirmation/scene proof',cases=rows),indent=2)+'\n')
 print(f'PASS {len(rows)} original M0239I first-page dialogue graphs')
if __name__=='__main__':main()
