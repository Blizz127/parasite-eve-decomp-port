#!/usr/bin/env python3
"""Actual M0034I send, delivery, wait/resume and battle-exit VM graph."""
import json,struct,sys
from pe_m0034i_setup_oracle import load,BASE,ROOT
from pe_battle_hud_oracle import execute
RANGES=((0x150000,0xC00),(0x152000,0x100),(0x160000,0x80),(0x9CDB4,1),(0x9CDFC,8),(0x9D1CC,2),(0x9D244,8),(0x9D264,2),(0x9D28C,4),(0x9D2E8,4),(0x9D2F8,4),(0x9D308,2),(0x9DF70,12),(0xB6A80,20),(0xB0CD8,4),(0xA3180,12),(0x70E04,76))
LEVELS=(1,10,11,20,21,30,31,40,41,99,0xFFFF,0x8000)
def digest(r):
 h=14695981039346656037
 for a,n in RANGES:
  for b in r[a:a+n]:h=((h^b)*1099511628211)&0xFFFFFFFFFFFFFFFF
 return h

def main():
 ex,raw=load();rows=[];seen=set();summaries=[]
 for profile in range(16):
  for draw in range(11):
   for waits in (0,2):
    r=bytearray(0x200000);r[0x10000:0x10000+len(ex)-0x800]=ex[0x800:];r[BASE&0x1FFFFF:(BASE&0x1FFFFF)+len(raw)]=raw
    def sw(a,v):struct.pack_into('<I',r,a&0x1FFFFF,v&0xFFFFFFFF)
    def sh(a,v):struct.pack_into('<H',r,a&0x1FFFFF,v&65535)
    def lw(a):return struct.unpack_from('<I',r,a&0x1FFFFF)[0]
    def call(a,args=()):return execute(r,a,args,scratchpad=bytearray(0x400),visited_pcs=seen,instruction_budget=200000)
    sender=0x80150000;actor=0x80150400;aya=0x80150800;task=0x80152000;node=0x80160000
    sw(sender+4,actor);r[0x15000C]=4;sh(sender+0x24,0xFF00+profile);sw(sender+0x9C,BASE+0x40CC)
    r[0x15040C]=5;sw(actor+0x9C,BASE+0x453C);sw(actor+0xA8,0x80152040)
    sw(0x8009D20C,sender);sw(0x8009D2F0,actor);sw(0x8009D300,task);sw(0x8009D1A0,0)
    for i in range(2):sw(0x80152200+i*4,BASE+0x459C+8+i*4)
    call(0x80017588,(0x80152200,));assert lw(actor+0x19C)==BASE+0x4DEC
    sw(0x8009D254,aya);sw(aya,aya+0x100)
    sh(aya+0x104,LEVELS[profile%12]);sh(aya+0x110,9000 if profile>=12 else 123)
    sh(aya+0x122,1+profile%6)
    for i,value in enumerate((0 if waits else 16,profile*7+1,1+profile%3,0xCAFEBABE,profile*3)):
     sw(0x800B6A80+i*4,value)
    sw(0x8009D2E8,0xA1);sw(0x800B0CD8,0xA02000);sw(0x8009D28C,8)
    r[0x160000:0x160080]=bytes([0xA5])*128
    sw(node+0x24,node+0x40);sw(node+0x64,0);sw(0x8009CDFC,node);sh(0x8009D308,0xFFFF)
    r[0x9CDB4]=0;sw(0x8009D2F0,sender);sw(task,BASE+0x451C);sw(task+16,1)
    sw(0x80070E04,0);sw(0x80070E08,8)
    for i in range(17):sw(0x80070E0C+i*4,0)
    sw(0x80070E0C,(draw*65536+10)//11)
    call(0x80017018);hashes=[digest(r)];assert r[0x9CDB4]==1
    call(0x80065400);hashes.append(digest(r))
    assert lw(actor+0xA8)==node and lw(node+20)==112 and lw(node+12)==0xFF00+profile
    assert lw(node)==BASE+0x4DEC and lw(node+0x24)==0x80152040 and lw(0x80152068)==node
    sw(0x8009D2F0,actor);sw(0x8009D300,node)
    for _ in range(waits):
     sw(0x8009D300,node);call(0x80017018);hashes.append(digest(r));assert lw(node)==BASE+0x4F24 and lw(node+16)==1
    sw(0x800B6A80,16);sw(0x8009D300,node);call(0x80017018);hashes.append(digest(r))
    assert lw(0x8009D2F8)==BASE+0x5724 and lw(0x8009D248)==BASE+0x5744, (profile,draw,waits,hex(lw(node)),hex(lw(node+16)),hex(lw(0x8009CE00)),hex(lw(0x8009D2F8)),hex(lw(0x8009D248)),hex(lw(0x800B6A80)))
    assert lw(0x8009D28C)==0 and r[0x9D244]==1 and lw(0x800B6A80)==20
    assert lw(node)==BASE+0x56D8
    hashes+= [0]*(5-len(hashes));rows.append((profile,draw,waits,hashes))
    summaries.append(dict(profile=profile,draw=draw,waits=waits,result=lw(0x800B6A8C),aya_actor_half=struct.unpack_from('<H',r,0x150810)[0],aya_stat_half=struct.unpack_from('<H',r,0x150910)[0]))
 assert {0x80017764,0x80065400,0x80012700,0x800177AC,0x800196E8,0x8001A3FC,0x8002FE78,0x8002FF78,0x80033A2C}<=seen
 lines=['/* Original M0034I delivery and battle-exit hashes; requires original script. */','static const uint32_t M34D_ranges[][2]={']
 lines.extend(f'{{0x{a:X}u,{n}}},' for a,n in RANGES);lines+=['};','static const uint16_t M34D_levels[]={'+','.join(str(x) for x in LEVELS)+'};','static const struct { unsigned profile,draw,waits; uint64_t hash[5]; } M34D_cases[]={']
 lines.extend('{%d,%d,%d,{%s}},'%(p,d,w,','.join('UINT64_C(0x%016X)'%h for h in hs)) for p,d,w,hs in rows);lines.append('};')
 lines.append('static const struct { unsigned tag; uint32_t value; uint64_t hash; } M34D_setters[]={')
 for tag in range(256):
  for value in (0,65535,0x80000000,0x12345678):
   r=bytearray(0x200000);r[0x10000:0x10000+len(ex)-0x800]=ex[0x800:]
   r[0x150000:0x150C00]=bytes([0xA5])*0xC00
   sw(0x8009D254,0x80150000);sw(0x80150000,0x80150800);sh(0x800942EC,0xCAFE)
   call(0x8002FF78,(tag,value))
   h=14695981039346656037
   for a,n in ((0x150000,0xC00),(0x942EC,2)):
    for byte in r[a:a+n]:h=((h^byte)*1099511628211)&0xFFFFFFFFFFFFFFFF
   lines.append('{%d,0x%Xu,UINT64_C(0x%016X)},'%(tag,value,h))
 lines.append('};')
 header='\n'.join(lines)+'\n';target=ROOT/'pc_port/tests/retail_m0034i_delivery_cases.h'
 if '--check' in sys.argv:assert target.read_text()==header
 else:target.write_text(header)
 (ROOT/'local/live/m0034i-delivery.json').write_text(json.dumps(dict(scope='original real send/full recipient VM through battle-exit requests; supplied actor list, Aya records, task pool, RNG and shared-flag assertion; no scheduler/actual battle completion claim',cases=summaries),indent=2)+'\n')
 print(f'PASS {len(rows)} original M0034I delivery/exit graphs and1024 Aya setter cases')
if __name__=='__main__':main()
