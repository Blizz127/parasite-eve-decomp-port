#!/usr/bin/env python3
"""Real M0374I send -> queue drain -> receiver VM -> scene fork to animation wait."""
import hashlib, json, struct, sys
import pe_day2_station_routes as station
from pe_day2_station_return import PINS
from pe_battle_hud_oracle import execute
ROOT=station.ROOT
CHUNK=0x8018EFE8
STORIES=(0,0xE0,0xE4,0xE6,0xE8,0xF0,0x80000000,0xFFFFFFFF)
RANGES=((0x150000,0x400),(0x160000,0x180),(0x9CDB4,1),(0x9CDFC,8),
        (0x9D300,4),(0x9D308,2),(0x9DF70,8),(0xA3180,12),(0xA7918,4),(0xBCEA8,224),(0x9CEA0,8),(0x9D1CC,2),(0x9D1D8,4),(0x9D1FC,4))
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
 for story in STORIES:
  for serial in (0,1,0xFFFE,0xFFFF):
   for linked in (0,1):
    r=bytearray(0x200000);r[0x10000:0x10000+len(ex)-0x800]=ex[0x800:]
    r[CHUNK&0x1FFFFF:(CHUNK&0x1FFFFF)+len(chunk)]=chunk
    def sw(a,v):struct.pack_into('<I',r,a&0x1FFFFF,v&0xFFFFFFFF)
    def sh(a,v):struct.pack_into('<H',r,a&0x1FFFFF,v&65535)
    def lw(a):return struct.unpack_from('<I',r,a&0x1FFFFF)[0]
    def call(a,args=()):execute(r,a,args,visited_pcs=seen,instruction_budget=100000)
    sender=0x80150000;actor=0x80150200;task=0x80160000;mail=task+0x40;child=task+0x80;old=task+0x100
    sw(sender+4,actor);sh(sender+0x24,0x1234);sw(sender+0x9C,0x801C4E04)
    r[0x15020C]=2;sw(actor+0x9C,0x801C5840);sw(actor+0xA8,old if linked else 0)
    sw(0x8009D20C,sender);sw(0x8009D2F0,actor);sw(0x8009D1A0,0)
    sw(0x80154000,0x801C58C4);sw(0x80154004,0x801C58C8)
    call(0x80017588,(0x80154000,));assert lw(actor+0x19C)==0x801C6354
    sw(task,0x801C54B8);sw(task+16,1);sw(sender+0xA8,task)
    sw(mail+0x24,child);sw(child+0x24,task+0xC0);sw(0x8009CDFC,mail)
    sh(0x8009D308,serial);r[0x9CDB4]=0;sw(0x800A7918,story)
    hashes=[digest(r,ranges)]
    sw(0x8009D2F0,sender);sw(0x8009D300,task);call(0x80017018);call(0x80065400)
    hashes.append(digest(r,ranges));assert lw(actor+0xA8)==mail and lw(mail+20)==1
    sw(0x8009D2F0,actor);sw(0x8009D300,mail);call(0x80017018)
    hashes.append(digest(r,ranges))
    assert lw(actor+0xBC)==1 and lw(child)==0x801C5D94 and lw(child+16)==1, [hex(lw(a)) for a in (actor+0xBC,child,child+16,actor+0xA8,0x8009CDFC,task,mail)]
    assert lw(actor+0xA8)==mail and lw(mail+0x24)==child
    assert lw(child+0x24)==(old if linked else 0)
    assert lw(0x800A7918)==0xE6 and r[0xBCEA8]==1
    assert struct.unpack_from("<H",r,0xBCEB8)[0]==0x62
    rows.append((story,serial,linked,hashes))
 assert {0x80017588,0x80017764,0x80065400,0x800177AC,0x800131E8,0x80012700}<=seen
 lines=['/* Original M0374I sender and receiver VM; explicit actor/task fixture. */',
        f'#define M374_CHUNK_LBA {lba}u',f'#define M374_CHUNK_SECTORS {len(chunk)//2048}u',
        'static const uint32_t M374D_ranges[][2]={']
 lines += [f'{{0x{a:X}u,{n}u}},' for a,n in ranges]
 lines += ['};','static const struct { uint32_t story; unsigned serial,linked; uint64_t hash[3]; } M374D_cases[]={']
 lines += ['{0x%Xu,%d,%d,{%s}},'%(s,n,l,','.join('UINT64_C(0x%016X)'%h for h in hs)) for s,n,l,hs in rows]
 lines += ['};'];header='\n'.join(lines)+'\n';target=ROOT/'pc_port/tests/retail_m0374i_delivery_cases.h'
 if '--check' in sys.argv:assert target.read_text()==header
 else:target.write_text(header)
 (ROOT/'local/live/m0374i-delivery-107.json').write_text(json.dumps(dict(script_sha256=script['sha256'],chunk_sha256=hashlib.sha256(chunk).hexdigest(),scope='original handler install, actual sender tail, queue drain and receiver VM fork through E6/dialogue62 to animation wait; explicit recordless actor/task fixture; no mesh, preceding scene or animation completion proof',cases=rows),indent=2)+'\n')
 print(f'PASS {len(rows)} original M0374I sender/delivery/fork graphs')
if __name__=='__main__':main()
