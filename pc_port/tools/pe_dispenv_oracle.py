#!/usr/bin/env python3
"""Original PutDispEnv, video-standard getter and GP1 writer instructions.

Only BIOS memcpy/printf are contracts. GP1 bus address is relocated to fixture
RAM; an instruction observer records all actual stores to that bus word.
"""
import hashlib,struct,sys
from pe_m0000i_leaves_oracle import load_overlay
import pe_battle_hud_oracle as cpu
from pe_transition_loader_oracle import fnv
WIDTHS=(-32768,-1,0,280,281,352,353,400,401,560,561,32767)
VALUES=(-32768,-1000,-1,0,1,2,255,256,257,288,289,32767)
def main():
 ex,_,_=load_overlay();digest=hashlib.sha256(ex[0x755F0-0xF800:0x75AE8-0xF800]).hexdigest();assert digest=='c8dc390773e463aadf9158ed73f3cb4467a6c8b2af8f853c3950512ef0577ca1'
 source=cpu.ROOT.joinpath('pc_port/tools/pe_battle_hud_oracle.py').read_text()
 marker='        jump = None\n';assert source.count(marker)==1
 source=source.replace(marker,marker+'''        if op==43 and ((r[rs]+si)&0xFFFFFFFF)==0x80151000: commands.append(r[rt])
''')
 marker='        if pc == 0xA0:\n';assert source.count(marker)==1
 source=source.replace(marker,'''        if pc==0xA0 and r[9]==0x2A:
            for i in range(r[6]):ram[(r[4]&0x1FFFFF)+i]=ram[(r[5]&0x1FFFFF)+i]
            r[2]=r[4];pc=r[31];continue
        if pc==0xA0 and r[9]==0x3F and r[4]==0x80011970:
            r[2]=len('PutDispEnv(%08x)...\\n'%r[5]);pc=r[31];continue
'''+marker)
 ns=dict(__file__=cpu.__file__,__name__='dispenv_observer',commands=[]);exec(compile(source,cpu.__file__,'exec'),ns)
 table=ex[0x95820-0xF800:0x95850-0xF800]
 out=['/* Original PutDispEnv state and ordered GP1 words. */','static const uint8_t DAY1_dispenv_table[]={'+','.join(str(x) for x in table)+'};','static const struct {uint64_t hash;uint32_t count,commands[4];} DAY1_dispenv_cases[]={']
 for n in range(2048):
  ram=bytearray(0x200000);ram[0x10000:0x10000+len(ex)-0x800]=ex[0x800:]
  def sw(a,v):struct.pack_into('<I',ram,a,v&0xFFFFFFFF)
  def sh(a,v):struct.pack_into('<H',ram,a,v&65535)
  ram[0x150000:0x150014]=bytes(20);ram[0x957B8:0x957CC]=bytes([0xA5])*20;ram[0xA3348:0xA3448]=bytes(256)
  sw(0x95854,0x80151000);sw(0x956EC,n%2)
  ram[0x9574E]=2 if n%127==0 else 0;ram[0x9574F]=n//2%2
  for i in range(8):sh(0x150000+i*2,WIDTHS[n%12] if i==2 else VALUES[(n//(i+1)+i)%12])
  ram[0x150010:0x150014]=bytes([n//4%2,n//8%2,n//16%9,n//32%256])
  if n&64:ram[0x957B8:0x957C0]=ram[0x150000:0x150008];ram[0x957C8:0x957CC]=ram[0x150010:0x150014]
  if n&128:ram[0x957C0:0x957C8]=ram[0x150008:0x150010]
  ns['commands']=[];r=ns['execute'](ram,0x800755F0,(0x80150000,))
  assert r[2]==0x80150000
  commands=ns['commands'];assert 1<=len(commands)<=4
  data=ram[0x150000:0x150014]+ram[0x957B8:0x957CC]+ram[0xA3348:0xA3448]
  out.append('{UINT64_C(0x%016X),%d,{%s}},'%(fnv(data),len(commands),','.join('0x%08Xu'%v for v in commands+[0]*(4-len(commands)))))
 out.append('};');header='\n'.join(out)+'\n'
 path=cpu.ROOT/'pc_port/tests/retail_dispenv_cases.h'
 if '--write-header' in sys.argv:path.write_text(header)
 else:assert path.read_text()==header
 print('SHA256',digest);print('PASS 2048 complete original PutDispEnv cases')
if __name__=='__main__':main()
