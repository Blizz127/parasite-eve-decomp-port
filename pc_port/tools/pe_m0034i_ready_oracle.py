#!/usr/bin/env python3
"""Original M0034I Aya placement produces the mailbox readiness bit."""
import hashlib,json,struct,sys
from pe_m0034i_setup_oracle import load,BASE,ROOT
from pe_day2_route_audit import parse_field_table,read_form1,find_disc
from pe_battle_hud_oracle import execute
CHUNK=0x8018EFE8
RANGES=((0x150000,0xC00),(0x154000,0x200),(0x160000,0xC0),(CHUNK&0x1FFFFF,204800),(0x9CDB4,1),(0x9CDFC,8),(0x9CE08,16),(0x9D1CC,2),(0x9D1D8,4),(0x9D1FC,4),(0x9D244,8),(0x9D264,2),(0x9D28C,4),(0x9D2E8,4),(0x9D2F8,4),(0x9D308,2),(0x9DF70,12),(0xB6A80,20),(0xB0CD8,4),(0xBCF88,4),(0xA3180,12))
def digest(r):
 h=14695981039346656037
 for a,n in RANGES:
  for byte in r[a:a+n]:h=((h^byte)*1099511628211)&0xFFFFFFFFFFFFFFFF
 return h

def main():
 ex,raw=load();rec=next(x for x in parse_field_table(ex) if x['map_id']==34);m=rec['meta'];prefix=((m&255)+(m>>8&4095))*2048
 chunk=read_form1(find_disc(ROOT),1013+rec['start']+prefix//2048,m>>20)
 assert hashlib.sha256(chunk).hexdigest()=='0eb2efb10e4779672a00f6da46c2d54f915f1b3e433048513fd08de296eedd5a'
 rows=[];seen=set();out=[]
 for variant in range(16):
  for cap in (0,1,8,255):
   r=bytearray(0x200000);r[0x10000:0x10000+len(ex)-0x800]=ex[0x800:];r[CHUNK&0x1FFFFF:(CHUNK&0x1FFFFF)+len(chunk)]=chunk
   def sw(a,v):struct.pack_into('<I',r,a&0x1FFFFF,v&0xFFFFFFFF)
   def sh(a,v):struct.pack_into('<H',r,a&0x1FFFFF,v&65535)
   def lw(a):return struct.unpack_from('<I',r,a&0x1FFFFF)[0]
   def call(a,args=()):return execute(r,a,args,scratchpad=bytearray(0x400),visited_pcs=seen,instruction_budget=200000)
   controller=0x80150000;aya=0x80150800;task=0x80160000;atask=task+0x40
   sw(controller+4,aya);r[0x15000C]=5;sw(controller+0x9C,BASE+0x453C);sh(controller+0x24,0xABCD)
   sw(aya,0x80154000);sw(aya+0x9C,BASE+0x20);sw(0x8009D254,aya);sw(0x8009D20C,controller)
   flags=(2 if variant&1 else 0)|(0x80 if variant&2 else 0)|(0x100 if variant&4 else 0)|(0x200 if variant&8 else 0)
   sw(aya+0x98,flags);sw(aya+0x14,0x12345678);sw(aya+0x18,0x87654321);sw(aya+0x1C,0xFFFD0000);sw(aya+0x1A4,0x12345678);sw(aya+0x1A8,0x87654321)
   sh(0x80154010,9000);sw(0x800B0EA8,0x80154100);r[0x154102]=cap
   sw(0x8009D1A0,0);sw(0x8009D2F0,aya)
   for i in range(2):sw(0x80154080+i*4,BASE+0x84+i*4)
   call(0x80017588,(0x80154080,));assert lw(aya+0x19C)==BASE+0x214
   header=CHUNK+lw(CHUNK+4);mesh=CHUNK+(lw(CHUNK+(lw(header+24)&0x3FFFFF)+4)&0xFFFFFF)
   assert mesh==CHUNK+0x2B288
   sw(0x800B1620,mesh);call(0x8001A918)
   sw(0x800BCF88,0xA501);sw(0x800B6A80,0);sw(0x8009D2E8,0xA1);sw(0x800B0CD8,0xA02000);sw(0x8009D28C,8)
   r[0x160000:0x1600C0]=bytes(0xC0)
   sw(task,BASE+0x4DEC);sh(task+8,4);sw(task+16,1);sw(task+20,112);sw(controller+0xA8,task)
   sw(atask+0x24,atask+0x40);sw(0x8009CDFC,atask);sh(0x8009D308,0xFFFF);r[0x9CDB4]=0
   hashes=[digest(r)]
   sw(0x8009D2F0,controller);sw(0x8009D300,task);call(0x80017018);hashes.append(digest(r));assert lw(task)==BASE+0x4F24 and lw(0x800B6A80)==0
   for i in range(3):sw(0x80154080+i*4,BASE+0x46EC+i*4)
   call(0x80017764,(0x80154080,));call(0x80065400);hashes.append(digest(r))
   assert lw(aya+0xA8)==atask and lw(atask+20)==101
   sw(0x8009D2F0,aya);sw(0x8009D300,atask);call(0x80017018);hashes.append(digest(r))
   assert lw(0x800B6A80)==16 and lw(atask)==BASE+0x4A0
   assert lw(aya+0x1C)==65536 and r[0x15080E]==4 and r[0x15080F]==(cap-1)&255
   assert lw(aya+0x14)==0 and lw(aya+0x18)==0
   assert lw(0x800BCF88)==0xA581
   sw(0x8009D2F0,controller);sw(0x8009D300,task);call(0x80017018);hashes.append(digest(r))
   assert lw(task)==BASE+0x56D8 and lw(0x800B6A80)==20 and lw(0x8009D28C)==0
   rows.append((variant,cap,hashes));out.append(dict(variant=variant,cap=cap,position=[lw(aya+0x28+i*4) for i in range(3)],floor=f'{lw(aya+0x1A4):08X}'))
 assert {0x8001A918,0x80017764,0x80065400,0x80012C20,0x8001AA78,0x8001C614,0x80017AE8,0x8001A680,0x80017A50}<=seen, sorted(hex(x) for x in seen if 0x80017900<=x<0x80017B00)
 lines=['/* Original M0034I readiness producer; real room data required. */','static const uint32_t M34R_ranges[][2]={']
 lines.extend(f'{{0x{a:X}u,{n}}},' for a,n in RANGES);lines+=['};','static const struct { unsigned variant,cap; uint64_t hash[5]; } M34R_cases[]={']
 lines.extend('{%d,%d,{%s}},'%(v,c,','.join('UINT64_C(0x%016X)'%h for h in hs)) for v,c,hs in rows);lines.append('};')
 header='\n'.join(lines)+'\n';target=ROOT/'pc_port/tests/retail_m0034i_ready_cases.h'
 if '--check' in sys.argv:assert target.read_text()==header
 else:target.write_text(header)
 (ROOT/'local/live/m0034i-ready.json').write_text(json.dumps(dict(scope='actual room mesh and script, original placement and animation selection produce readiness and release mailbox; supplied actor/task publication and common clip frame-count header, no real animation playback or scene-entry proof',cases=out),indent=2)+'\n')
 print(f'PASS {len(rows)} original M0034I readiness graphs')
if __name__=='__main__':main()
