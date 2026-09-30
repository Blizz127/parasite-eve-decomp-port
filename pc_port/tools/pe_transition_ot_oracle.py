#!/usr/bin/env python3
"""Original complete frame OT compaction loop, including live pointer aliases."""
import hashlib,struct,sys
from pe_m0000i_leaves_oracle import load_overlay
from pe_battle_hud_oracle import execute,ROOT
from pe_transition_loader_oracle import fnv

def main():
 ex,o,b=load_overlay();digest=hashlib.sha256(o[0x801925A0-b:0x8019262C-b]).hexdigest();print('SHA256',digest);assert digest=='e3bbb8893991be52590560e3222bffa28dcdec3c44eaf5ff1be22fa387a1464a'
 out=['/* Original transition OT loop comparison fixtures. */','static const struct { uint32_t seed,mode,initial,result; uint64_t hash; } DAY1_transition_ot_cases[]={']
 for mode in range(8):
  for seed in range(64):
   r=bytearray(0x200000);r[0x10000:0x10000+len(ex)-0x800]=ex[0x800:];r[b&0x1fffff:(b&0x1fffff)+len(o)]=o
   def sw(a,v):struct.pack_into('<I',r,a&0x1fffff,v&0xffffffff)
   for i in range(4096):
    empty=(False,True,i%2==0,i%2==1,i>seed*64,i<seed*64,(i*2654435761+seed)&15!=0,(i*2654435761+seed)&15!=0)[mode]
    value=0x160000+i*4-4 if empty else 0x145000+(i*28)%4096
    if i==0:value=0xFFFFFF
    sw(0x160000+i*4,value)
   # Alias mode places the descriptor OT pointer in bucket zero, preserving
   # its high address byte and testing the bottom-of-table comparison.
   descriptor=0x15FFFC if mode==7 else 0x150000
   sw(descriptor+4,0x80160000);sw(0x19C9C0,0x80000000+descriptor)
   initial=seed*67
   regs=execute(r,0x801925A0,stop_at=(0x8019262C,),initial_regs={16:initial,19:0xFFFFFF,20:0x80000000},instruction_budget=150000)
   h=fnv(r[0x150000:0x164000]+r[0x19C9C0:0x19C9C4]);out.append(f' {{{seed},{mode},{initial},{regs[16]},UINT64_C(0x{h:016X})'+'},')
 out.append('};');header='\n'.join(out)+'\n'
 if '--write-header' in sys.argv:(ROOT/'pc_port/tests/retail_transition_ot_cases.h').write_text(header)
 else:assert (ROOT/'pc_port/tests/retail_transition_ot_cases.h').read_text()==header
 print('PASS 512 original OT compaction cases')
if __name__=='__main__':main()
