from pathlib import Path
import sys,json,ctypes,random,struct
root=Path(__file__).resolve().parents[2];sys.path.insert(0,str(root/'pc_port/tools'));import pe_cd_leaf_vectors as c
s=Path(sys.argv[1]);base=bytes(c.v.fixture((root/'build/disc1.candidate.exe').read_bytes(),0,0,0));rng=random.Random(0x80075424);n=int(sys.argv[2]) if len(sys.argv)>2 else 6000;mutate=len(sys.argv)>3;fail=[];fault=[];drops=done=0
L=ctypes.CFUNCTYPE(ctypes.c_uint32,ctypes.c_uint32,ctypes.c_uint32);S=ctypes.CFUNCTYPE(None,ctypes.c_uint32,ctypes.c_uint32);C=ctypes.CFUNCTYPE(ctypes.c_uint32,*([ctypes.c_uint32]*5));lib=ctypes.CDLL(str(s/'retail-draft.so'));lib.draft.argtypes=[ctypes.c_uint32,L,S,C];lib.draft.restype=ctypes.c_uint32
for i in range(n):
 ram,_,_,_=c.prep(base,1);env=rng.choice([0x80140000,0x80160000]);ram[env&0x1fffff:(env&0x1fffff)+92]=rng.randbytes(92);ram[0x9574e]=rng.choice([0,1,2,3,255])
 def sw(mem,a,v):struct.pack_into('<I',mem,a&0x1fffff,v)
 tables=[0x80095704,0x80150000];sw(ram,0x80095744,rng.choice(tables));sw(ram,0x80095748,0x80170008)
 for j,t in enumerate(tables):sw(ram,t+24,rng.getrandbits(32));sw(ram,t+8,0x80170000+4*j)
 native=bytearray(ram);oc=[];nc=[];loads=[];stores=[];rets={fn:rng.getrandbits(32)for fn in [0x80075EE0,0x80071A34,0x80170000,0x80170004,0x80170008]};replacement=rng.choice(tables);tag=rng.getrandbits(32);post=rng.getrandbits(32)
 def invoke(mem,fn,args,events,nl,ns):
  events.append([fn,args,nl,ns])
  if fn==0x80071A34:
   dst,src,count=args;mem[dst&0x1fffff:(dst&0x1fffff)+count]=mem[src&0x1fffff:(src&0x1fffff)+count]
  if mutate:
   if fn in [0x80075EE0,0x80170008]:sw(mem,0x80095744,replacement);sw(mem,env+28,tag)
   elif fn in [0x80170000,0x80170004]:sw(mem,env,post)
  return rets[fn]
 def arity(fn):return 3 if fn==0x80071A34 else 2 if fn in [0x80075EE0,0x80170008] else 4
 def model(fn):
  def cb(mem,regs):return invoke(mem,fn,regs[4:4+arity(fn)],oc,len(c.pl.LOADS),len(c.pl.STORES))
  return cb
 c.UNFAITHFUL[0]=False
 try:out=c.run_with_callees(ram,0x80075424,[env],rets,{fn:model(fn)for fn in rets})
 except(AssertionError,struct.error)as e:fault.append([i,str(e)]);continue
 if c.UNFAITHFUL[0]:drops+=1;continue
 def ld(a,k):v=int.from_bytes(native[a&0x1fffff:(a&0x1fffff)+k],'little');loads.append((a,k,v));return v
 def st(a,v):sw(native,a,v);stores.append((a,4,v))
 def call(fn,a,b,d,e):return invoke(native,fn,[a,b,d,e][:arity(fn)],nc,len(loads),len(stores))
 got=lib.draft(env,L(ld),S(st),C(call));done+=1;regions=[(env&0x1fffff,92),(0x9575c,92)];same=all(ram[a:a+size]==native[a:a+size]for a,size in regions)
 if got!=out[0] or loads!=out[1] or stores!=out[2] or nc!=oc or not same:fail.append(dict(case=i,returns=[got,out[0]],loads=[loads,out[1]],stores=[stores,out[2]],calls=[nc,oc],regions_equal=same))
summary=dict(requested=n,compared=done,failures=len(fail),faults=len(fault),unfaithful=drops);tag='retail-mutating-check'if mutate else'retail-check';(s/f'{tag}-{n}.json').write_text(json.dumps(dict(summary=summary,failures=fail[:12],faults=fault[:12]),indent=2)+'\n');print(summary);sys.exit(bool(fail or fault or drops))
