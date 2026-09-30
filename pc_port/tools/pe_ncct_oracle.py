#!/usr/bin/env python3
"""Original NCCT instruction with documented hardware arithmetic; no hardware capture."""
import hashlib,struct,sys
from pe_battle_hud_oracle import ROOT,execute

def main():
 ex=(ROOT/'build/disc1.candidate.exe').read_bytes();assert hashlib.sha1(ex).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
 out=['/* NCCT oracle: psx-spx arithmetic, original instruction at3BBDC. */','static const struct { uint32_t input[20],output[9]; } NCCT_cases[]={']
 for n in range(512):
  seed=0x91+n*65537
  def nxt():
   nonlocal seed
   seed^=(seed<<13)&0xFFFFFFFF;seed^=seed>>17;seed^=(seed<<5)&0xFFFFFFFF;return seed
  values=[nxt() for _ in range(20)]
  if n<256:
   for i in range(6,16):values[i]=((values[i]&0x1FFF0000)|(values[i]&0x1FFF))
   for i in range(16,19):values[i]=values[i]&0xFFF
  else:
   for i in range(16,19):values[i]=(0x7FFFFFFF,0x80000000,0xFFFFFFFF,0)[(n+i)%4]
  data={i:values[i] for i in range(6)};data[6]=values[19]
  control={8+i:values[6+i] for i in range(5)};control.update({16+i:values[11+i] for i in range(5)});control.update({13+i:values[16+i] for i in range(3)})
  r=bytearray(0x200000);r[0x10000:0x10000+len(ex)-0x800]=ex[0x800:];state={}
  execute(r,0x8003BBDC,stop_at=(0x8003BBE0,),initial_cop_data=data,initial_cop_control=control,final_gte=state,strict_gte_flags=True)
  d=state['data'];result=d[20:23]+d[25:28]+d[9:12]
  out.append('{ {'+','.join(f'0x{x:X}u' for x in values)+'}, {'+','.join(f'0x{x:X}u' for x in result)+'} },')
 out.append('};');header='\n'.join(out)+'\n';p=ROOT/'pc_port/tests/retail_ncct_cases.h'
 if '--write-header' in sys.argv:p.write_text(header)
 else:assert p.read_text()==header
 print('PASS512 original NCCT instruction cases against documented arithmetic')
if __name__=='__main__':main()
