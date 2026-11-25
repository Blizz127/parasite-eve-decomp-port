from pathlib import Path
import sys,json,ctypes,random,struct
root=Path(__file__).resolve().parents[2];sys.path.insert(0,str(root/'pc_port/tools'));import pe_cd_leaf_vectors as c
s=Path(sys.argv[1]);base=bytes(c.v.fixture((root/'build/disc1.candidate.exe').read_bytes(),0,0,0));rng=random.Random(0x80074F44);n=int(sys.argv[2]) if len(sys.argv)>2 else 6000;mutate=len(sys.argv)>3;fail=[];fault=[];drops=done=0
L=ctypes.CFUNCTYPE(ctypes.c_uint32,ctypes.c_uint32,ctypes.c_uint32);C=ctypes.CFUNCTYPE(ctypes.c_uint32,*([ctypes.c_uint32]*5));lib=ctypes.CDLL(str(s/'retail-draft.so'));lib.draft.argtypes=[ctypes.c_uint32]*4+[L,C];lib.draft.restype=ctypes.c_uint32
for i in range(n):
 ram,_,_,_=c.prep(base,1);rect=rng.getrandbits(32);rgb=[rng.getrandbits(32) for _ in range(3)]
 def sw(mem,a,v):struct.pack_into('<I',mem,a&0x1fffff,v)
 tables=[0x80095704,0x80150000];sw(ram,0x80095744,rng.choice(tables))
 for j,t in enumerate(tables):sw(ram,t+8,0x80170000+4*j);sw(ram,t+12,rng.getrandbits(32))
 native=bytearray(ram);oc=[];nc=[];loads=[];rets={fn:rng.getrandbits(32)for fn in [0x80074E28,0x80170000,0x80170004]};replacement=rng.choice(tables);op=rng.getrandbits(32)
 def invoke(mem,fn,args,events,nl):
  events.append([fn,args,nl])
  if mutate and fn==0x80074E28:sw(mem,0x80095744,replacement);sw(mem,replacement+12,op)
  return rets[fn]
 def model(fn):
  def cb(mem,regs):return invoke(mem,fn,regs[4:6]if fn==0x80074E28 else regs[4:8],oc,len(c.pl.LOADS))
  return cb
 c.UNFAITHFUL[0]=False
 try:out=c.run_with_callees(ram,0x80074F44,[rect,*rgb],rets,{fn:model(fn)for fn in rets})
 except(AssertionError,struct.error)as e:fault.append([i,str(e)]);continue
 if c.UNFAITHFUL[0]:drops+=1;continue
 def ld(a,k):v=int.from_bytes(native[a&0x1fffff:(a&0x1fffff)+k],'little');loads.append((a,k,v));return v
 def call(fn,a,b,d,e):return invoke(native,fn,[a,b]if fn==0x80074E28 else [a,b,d,e],nc,len(loads))
 got=lib.draft(rect,*rgb,L(ld),C(call));done+=1
 if got!=out[0] or loads!=out[1] or out[2] or nc!=oc:fail.append(dict(case=i,returns=[got,out[0]],loads=[loads,out[1]],stores=out[2],calls=[nc,oc]))
summary=dict(requested=n,compared=done,failures=len(fail),faults=len(fault),unfaithful=drops);tag='retail-mutating-check'if mutate else'retail-check';(s/f'{tag}-{n}.json').write_text(json.dumps(dict(summary=summary,failures=fail[:12],faults=fault[:12]),indent=2)+'\n');print(summary);sys.exit(bool(fail or fault or drops))
