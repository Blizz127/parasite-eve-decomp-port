#!/usr/bin/env python3
"""Original 6CDA4 control flow with explicit disc/SPU provider return contracts.

The complete original routine executes, including every internal retry. Only
the listed hardware-provider callees are replaced by controlled return values;
their arguments/order are compared separately from final RAM and return value.
This proves loader control flow, not disc or SPU provider implementation.
"""
import hashlib,struct,sys
from pe_battle_hud_oracle import ROOT,execute

PROVIDERS={0x80087198:0,0x80087414:0,0x8006E6D4:4,0x8006E7E8:0,
           0x800871AC:2,0x80087090:2,0x800875FC:2,0x80087428:3,0x800870E0:0}
RETURNS={0x8006CE8C:0x80087198,0x8006CEA4:0x80087414,0x8006CF04:0x8006E6D4,
         0x8006CF30:0x8006E7E8,0x8006CF90:0x800871AC,0x8006CFA0:0x80087090,
         0x8006CFB4:0x800875FC,0x8006CFCC:0x80087428,0x8006CFF0:0x800870E0}
RANGES=((0x9D170,16),(0xB0CD8,0x120))
CASES=[(mode,state,blocking,fault,ret,empty) for mode in range(4)
       for state in (0,7,8,9,10) for blocking in (0,1)
       for fault,ret in ((-1,0),(0,-1),(0,1),(1,-1),(1,1),(2,-1))
       for empty in (0,1)]

def fixture(exe,c):
 r=bytearray(0x200000);r[0x10000:0x10000+len(exe)-0x800]=exe[0x800:]
 for a,n in RANGES:r[a:a+n]=bytes(n)
 mode,state,blocking,fault,ret,empty=c
 struct.pack_into('<IHH',r,0x9317C,100,0,5)
 struct.pack_into('<H',r,0x93184,5 if empty else 9)
 struct.pack_into('<4I',r,0x9D170,1005,4,0 if empty else 4,2)
 struct.pack_into('<I',r,0xB0DD8,900)
 r[0xB0DC8]=state
 return r

def fingerprint(r):
 h=14695981039346656037
 for a,n in RANGES:
  for b in r[a:a+n]:h=((h^b)*1099511628211)&0xFFFFFFFFFFFFFFFF
 return h

def original(r,c):
 mode,state,blocking,fault,ret,empty=c
 # An initial completed chunk with zero remaining would underflow just as
 # retail does; those invalid fixtures are excluded by main.
 regs=execute(r,0x8006CDA4,(mode,1,3,0x80150000,2,blocking),stop_at=PROVIDERS)
 calls=[]
 while regs[31]:
  fn=RETURNS[regs[31]];n=PROVIDERS[fn]
  calls.append((fn,*regs[4:4+n],*[0]*(4-n)))
  regs[2]=(ret if len(calls)-1==fault else 0)&0xFFFFFFFF
  assert len(calls)<100,'provider script did not terminate'
  regs=execute(r,regs[31],stop_at=PROVIDERS,initial_regs=dict(enumerate(regs)))
 return regs[2],calls

def main():
 exe=(ROOT/'build/disc1.candidate.exe').read_bytes()
 assert hashlib.sha1(exe).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
 assert hashlib.sha256(exe[0x5D5A4:0x5D878]).hexdigest()=='c94c08adeed015909c745201d6c1f142f9966204b52470fc0fc0778fce3c8f79'
 cases=[];trace=[]
 for c in CASES:
  if c[5] and c[1] in (8,9,10):continue
  r=fixture(exe,c);result,calls=original(r,c)
  first=len(trace);trace.extend(calls)
  cases.append((*c,result,first,len(trace),fingerprint(r)))
 out=['/* Generated original6CDA4 control-flow cases; explicit provider returns. */',
      'static const uint32_t DAY1_loader_calls[][5]={']
 out.extend('    {'+','.join(f'0x{v:08X}u' for v in row)+'},' for row in trace)
 out+=['};','static const struct { int mode,state,blocking,fault,ret,empty; uint32_t result,first,end; uint64_t hash; } DAY1_loader_cases[]={']
 out.extend('    {'+','.join(str(v) for v in row[:-1])+f',UINT64_C(0x{row[-1]:016X})'+'},' for row in cases)
 out.append('};')
 if '--write-header' in sys.argv:(ROOT/'pc_port/tests/retail_disc_loader_loop_cases.h').write_text('\n'.join(out)+'\n')
 print('PASS:',len(cases),'complete original loader control-flow cases;',len(trace),'provider calls')
if __name__=='__main__':main()
