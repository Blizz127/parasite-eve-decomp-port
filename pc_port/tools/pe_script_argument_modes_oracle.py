#!/usr/bin/env python3
"""Check research decoder modes against original VM argument-pointer binding."""
import struct
from pe_m0034i_setup_oracle import load,BASE,ROOT
from pe_day2_route_audit import decode_script
from pe_battle_hud_oracle import execute

def main():
 ex,raw=load();count=0
 for m in decode_script(raw)['modules']:
  for c in m['commands']:
   if c['argc']<6 or c['opcode']>=0xEA or any(x>4 for x in c['modes']):continue
   r=bytearray(0x200000);r[0x10000:0x10000+len(ex)-0x800]=ex[0x800:];r[BASE&0x1FFFFF:(BASE&0x1FFFFF)+len(raw)]=raw
   def sw(a,v):struct.pack_into('<I',r,a&0x1FFFFF,v&0xFFFFFFFF)
   def lw(a):return struct.unpack_from('<I',r,a&0x1FFFFF)[0]
   pc=BASE+c['offset'];handler=lw(0x800910A0+c['opcode']*4)
   if not 0x80010000<=handler<0x80090000:continue
   sw(0x8009D2F0,0x80150000);sw(0x8015009C,BASE+m['start']);sw(0x8009D300,0x80152000);sw(0x80152000,pc);sw(0x80152010,1);sw(0x8009D1A0,0)
   regs=execute(r,0x80017018,stop_at=(handler,),scratchpad=bytearray(0x400))
   for i,(mode,value) in enumerate(zip(c['modes'],c['args'])):
    expected=pc+8+i*4 if mode==0 else ((0,0x801500AC,0x800A77F0,0x8009DF70,0x800B6A80)[mode]+value*4)&0xFFFFFFFF
    assert lw(regs[4]+i*4)==expected,(hex(pc),i,mode,hex(lw(regs[4]+i*4)),hex(expected))
   count+=1
 assert count>10
 print(f'PASS {count} original multiword command argument bindings')
if __name__=='__main__':main()
