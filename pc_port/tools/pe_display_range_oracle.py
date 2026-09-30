#!/usr/bin/env python3
"""Original NTSC PutDispEnv vertical-range arithmetic, including clipping."""
import hashlib,itertools,struct,sys
from pe_battle_hud_oracle import ROOT,execute

def main():
 e=(ROOT/'build/disc1.candidate.exe').read_bytes()
 assert hashlib.sha1(e).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
 rows=[]
 for y,h in itertools.product((-32768,-17,-8,0,8,16,223,239,240,241,32767),(-32768,-1,0,1,2,224,240,32767)):
  r=bytearray(0x200000);r[0x10000:0x10000+len(e)-0x800]=e[0x800:]
  struct.pack_into('<h',r,0x14000A,y);struct.pack_into('<h',r,0x14000E,h)
  regs=execute(r,0x80075858,initial_regs={2:0,17:0x80140000},stop_at=(0x80075880,))
  regs=execute(r,0x80075A14,initial_regs=dict(enumerate(regs)),stop_at=(0x80075A60,))
  rows.append((y,h,regs[16]-16,regs[18]-regs[16]))
 out=['/* Original PutDispEnv NTSC vertical ranges (start relative to line16). */',
 'static const struct { int y,h; unsigned top,lines; } DAY1_vertical_range_cases[]={']
 out += [' {'+','.join(map(str,x))+'},' for x in rows];out+=['};']
 if '--write-header' in sys.argv:(ROOT/'pc_port/tests/retail_display_range_cases.h').write_text('\n'.join(out)+'\n')
 print('PASS:',len(rows),'original vertical range cases')
if __name__=='__main__':main()
