from pathlib import Path
import sys,json,ctypes,random,struct
root=Path(__file__).resolve().parents[2];sys.path.insert(0,str(root/'pc_port/tools'));import pe_cd_leaf_vectors as c
s=Path(sys.argv[1]);base=bytes(c.v.fixture((root/'build/disc1.candidate.exe').read_bytes(),0,0,0));rng=random.Random(0x8007A2A4);n=int(sys.argv[2]) if len(sys.argv)>2 else 6000;mutate=len(sys.argv)>3;fail=[];fault=[];drops=done=0
L=ctypes.CFUNCTYPE(ctypes.c_uint32,ctypes.c_uint32,ctypes.c_uint32);S=ctypes.CFUNCTYPE(None,ctypes.c_uint32,ctypes.c_uint32,ctypes.c_uint32);C=ctypes.CFUNCTYPE(ctypes.c_uint32,*([ctypes.c_uint32]*5));lib=ctypes.CDLL(str(s/'retail-draft.so'));lib.draft.argtypes=[L,S,C];lib.draft.restype=ctypes.c_uint32
functions=[0x80072714,0x80072724,0x800824F0,0x800824C8,0x8007A8EC,0x8007A4BC]
for i in range(n):
 ram,_,_,_=c.prep(base,1);native=None
 def sw(mem,a,v):struct.pack_into('<I',mem,a&0x1fffff,v)
 state=rng.choice([0,1,2,0xffffffff]);p1=rng.choice([0x80140001,0x80140002,0x8009af28]);p2=0x80140003
 ram[0x140000:0x140010]=rng.randbytes(16);sw(ram,0x8009afd8,state);sw(ram,0x8009af1c,p1);sw(ram,0x8009af28,p2);native=bytearray(ram)
 oc=[];nc=[];loads=[];stores=[];rets={fn:rng.getrandbits(32) for fn in functions};newstate=rng.choice([0,1,2]);replacement=rng.choice([0x80140000,0x80140001,0x8009af28])
 def invoke(mem,fn,args,events,nl,ns):
  events.append([fn,args,nl,ns])
  if mutate:
   if fn==0x80072714:sw(mem,0x8009afd8,newstate)
   elif fn in [0x800824F0,0x8007A8EC]:sw(mem,0x8009af1c,replacement)
   elif fn in [0x800824C8,0x8007A4BC]:sw(mem,0x8009af28,0x80140003)
   elif fn==0x80072724:mem[0x140004]=0x6d
  return rets[fn]
 def arity(fn):return 0 if fn in [0x80072714,0x80072724] else 1
 def model(fn):
  def cb(mem,regs):return invoke(mem,fn,regs[4:4+arity(fn)],oc,len(c.pl.LOADS),len(c.pl.STORES))
  return cb
 c.UNFAITHFUL[0]=False
 try:out=c.run_with_callees(ram,0x8007A2A4,[],rets,{fn:model(fn)for fn in functions})
 except(AssertionError,struct.error)as e:fault.append([i,str(e)]);continue
 if c.UNFAITHFUL[0]:drops+=1;continue
 def ld(a,k):v=int.from_bytes(native[a&0x1fffff:(a&0x1fffff)+k],'little');loads.append((a,k,v));return v
 def st(a,v,k):native[a&0x1fffff:(a&0x1fffff)+k]=v.to_bytes(k,'little');stores.append((a,k,v))
 def call(fn,a,b,d,e):return invoke(native,fn,[a,b,d,e][:arity(fn)],nc,len(loads),len(stores))
 got=lib.draft(L(ld),S(st),C(call));done+=1;regions=[(0x9af1c,4),(0x9af28,4),(0x9afd8,4),(0x140000,16)]
 if got!=out[0] or loads!=out[1] or stores!=out[2] or nc!=oc or any(ram[a:a+size]!=native[a:a+size]for a,size in regions):fail.append(dict(case=i,returns=[got,out[0]],loads=[loads,out[1]],stores=[stores,out[2]],calls=[nc,oc]))
summary=dict(requested=n,compared=done,failures=len(fail),faults=len(fault),unfaithful=drops);tag='retail-mutating-check'if mutate else'retail-check';(s/f'{tag}-{n}.json').write_text(json.dumps(dict(summary=summary,failures=fail[:12],faults=fault[:12]),indent=2)+'\n');print(summary);sys.exit(bool(fail or fault or drops))
