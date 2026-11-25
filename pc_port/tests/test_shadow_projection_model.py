from pathlib import Path
import struct,sys
sys.path.insert(0,'pc_port/tools')
from pe_battle_hud_oracle import execute
lines=Path('build/codex-shadow-projection-20261106/states.txt').read_text().splitlines()
expected_xy=(0x0004000c,0x0004fffd,0x0004000d,0x0004fffd)
expected_depth=(135,135,127,127)
cases=0
for line in lines:
 if not line.startswith('CASE '): continue
 t=line.split(); k=int(t[1])-1; ci=t.index('C'); di=t.index('D')
 control=[int(x,16) for x in t[ci+1:di]]
 data=[int(x,16) for x in t[di+1:]]
 assert len(control)==len(data)==32 and 0<=k<4
 data[0]=int(t[3],16);data[1]=int(t[4],16)
 ram=bytearray(0x200000)
 for i,word in enumerate((0x4a180001,0x03e00008,0)):
  struct.pack_into('<I',ram,0x143000+4*i,word)
 result={}; visited=set()
 execute(ram,0x80143000,initial_cop_control=dict(enumerate(control)),initial_cop_data=dict(enumerate(data)),final_gte=result,visited_pcs=visited,instruction_budget=10)
 assert visited=={0x80143000,0x80143004,0x80143008}
 xy=result['data'][14]; depth=(result['data'][19]&65535)>>2; flag=result['control'][31]
 if (xy,depth,flag)!=(expected_xy[k],expected_depth[k],0):
  print('DIFFERENT',cases,hex(xy),depth,hex(flag));sys.exit(1)
 cases+=1
assert cases==8
print('independent test-only GTE model agrees with eight native projection calls on xy, depth and flags')
