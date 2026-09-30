#!/usr/bin/env python3
"""Extract the constructor's straight-line constant record initialization.

This is offline data extraction, not a production instruction interpreter.
Two dirty patterns identify untouched bytes. Single-bit flag runs identify
field-bit assignments; all-bit and alternating patterns verify the mapping.
"""
import hashlib,struct,sys
from pe_m0000i_leaves_oracle import load_overlay
from pe_battle_hud_oracle import ROOT,execute
START=0x1EA370;SIZE=520

def main():
 _,o,b=load_overlay();digest=hashlib.sha256(o[0x80196498-b:0x80197BA0-b]).hexdigest()
 assert digest=='c946ea3080c7e550f1ce346ee26e40cb8006fcbcd4f288d31ef9202cbb10562e'
 print('constructor SHA256',digest)
 def run(seed,flags):
  r=bytearray(0x200000);r[b&0x1FFFFF:(b&0x1FFFFF)+len(o)]=o
  r[START:START+SIZE]=bytes([seed])*SIZE;struct.pack_into('<I',r,0xA77FC,flags)
  regs=execute(r,0x80196620,stop_at=(0x80196EAC,),initial_regs={2:0x80140000})
  return r[START:START+SIZE]
 zero=run(0,0);dirty=run(255,0);bits=[run(0,1<<i) for i in range(32)]
 constants=[];dynamic=[]
 for i,(v,w) in enumerate(zip(zero,dirty)):
  if v!=w:assert v==0 and w==255;continue
  changes=[j for j in range(32) if bits[j][i]!=v]
  if changes:
   assert len(changes)==1 and v==0 and bits[changes[0]][i]==1;dynamic.append((i,changes[0]))
  else:constants.append((i,v))
 for flags in (0xFFFFFFFF,0x55555555,0xAAAAAAAA,0x12345678):
  expect=bytearray([165])*SIZE
  for i,v in constants:expect[i]=v
  for i,bit in dynamic:expect[i]=(flags>>bit)&1
  assert run(165,flags)==expect
 out=['/* Generated from original96620..96EAC by pe_transition_constructor_data.py. */',
 'static const struct { uint16_t offset; uint8_t value; } transition_record_constants[]={']
 out.extend(f' {{{i},{v}}},' for i,v in constants);out.append('};')
 out.append('static const struct { uint16_t offset; uint8_t bit; } transition_record_flags[]={')
 out.extend(f' {{{i},{v}}},' for i,v in dynamic);out.append('};')
 if '--write-header' in sys.argv:(ROOT/'pc_port/game/boot/transition_constructor_data.h').write_text('\n'.join(out)+'\n')
 print('PASS:',len(constants),'constant bytes;',len(dynamic),'flag bytes;',SIZE-len(constants)-len(dynamic),'preserved bytes')
if __name__=='__main__':main()
