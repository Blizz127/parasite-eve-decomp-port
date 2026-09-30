#!/usr/bin/env python3
"""Execute original 91C94 with explicit read-issue/poll provider contracts.

No real I/O is performed. Provider mutations test when globals are reread.
"""
import hashlib,itertools,struct,sys
from pe_battle_hud_oracle import ROOT,execute
from pe_m0000i_leaves_oracle import load_overlay
from pe_transition_loader_oracle import fnv
STOPS=(0x8006E6A8,0x8006E7E8)
RANGES=((0x93170,8),(0xB0DD8,4),(0x19C1F0,4))
def run(exe,overlay,base,flag,profile,mutate,edge):
 r=bytearray(0x200000);r[0x10000:0x10000+len(exe)-0x800]=exe[0x800:]
 r[base&0x1FFFFF:(base&0x1FFFFF)+len(overlay)]=overlay
 def sw(a,v):struct.pack_into('<I',r,a,v&0xFFFFFFFF)
 sw(0xB0DD8,0xFFFFFFF0 if edge else 1013);sw(0x19C1F0,flag)
 struct.pack_into('<4H',r,0x93170,*( (65535,0,65535,1) if edge else (1975,2016,2105,2205)))
 issues=[0,0];polls=[0,0];phase=0;calls=0;trace=14695981039346656037
 regs=execute(r,0x80191C94,stop_at=STOPS)
 while regs[31]:
  ret=regs[31];w=struct.unpack_from('<I',r,(ret-8)&0x1FFFFF)[0];fn=0x80000000|((w&0x3FFFFFF)<<2)
  args=regs[4:7] if fn==STOPS[0] else [0]*3
  trace=fnv(struct.pack('<4I',fn,*args),trace);calls+=1;assert calls<100
  if fn==STOPS[0]:
   phase={0x801D0260:0,0x8019CE10:1}[args[1]]
   result=-1 if profile&1 and issues[phase]<2 else -2
   issues[phase]+=1
  else:
   seq=(2,-2,-1,1,0) if profile&2 else (2,-2,0)
   result=seq[polls[phase]];polls[phase]+=1
  if mutate:
   sw(0xB0DD8,struct.unpack_from('<I',r,0xB0DD8)[0]+17)
   for i in range(4):
    a=0x93170+i*2;struct.pack_into('<H',r,a,(struct.unpack_from('<H',r,a)[0]+i+1)&65535)
   r[0x19C1F0]^=1
  regs[2]=result&0xFFFFFFFF
  regs=execute(r,ret,initial_regs=dict(enumerate(regs)),stop_at=STOPS)
 return calls,trace,fnv(b''.join(r[a:a+n] for a,n in RANGES))
def main():
 exe,overlay,base=load_overlay()
 assert hashlib.sha256(overlay[0x80191C94-base:0x80191DE8-base]).hexdigest()=='3a5583187fcbf16da1ba8c270731bb69a8e87c07601d7633ce3fc15cc0728f2b'
 rows=[]
 for inputs in itertools.product((0,1,2,0x100),range(4),range(2),range(2)):
  rows.append((*inputs,*run(exe,overlay,base,*inputs)))
 out=['/* Original91C94 explicit CD-provider contract cases. */','static const struct { unsigned flag,profile,mutate,edge,calls; uint64_t trace,state; } DAY1_package_cases[]={']
 for row in rows:out.append(' {'+','.join(map(str,row[:5]))+','+','.join(f'UINT64_C(0x{x:016X})' for x in row[5:])+'},')
 out.append('};')
 if '--write-header' in sys.argv:(ROOT/'pc_port/tests/retail_transition_packages_cases.h').write_text('\n'.join(out)+'\n')
 print('PASS:',len(rows),'original package-loader cases;',sum(r[4] for r in rows),'CD-provider calls')
if __name__=='__main__':main()
