from pathlib import Path
import sys,json,ctypes,random,struct
root=Path(__file__).resolve().parents[2];sys.path.insert(0,str(root/'pc_port/tools'));import pe_cd_leaf_vectors as c
s=Path(sys.argv[1]);base=bytes(c.v.fixture((root/'build/disc1.candidate.exe').read_bytes(),0,0,0));rng=random.Random(0x8007512C);n=int(sys.argv[2]) if len(sys.argv)>2 else 6000;mutate=len(sys.argv)>3;fail=[];fault=[];drops=done=0
L=ctypes.CFUNCTYPE(ctypes.c_uint32,ctypes.c_uint32,ctypes.c_uint32);S=ctypes.CFUNCTYPE(None,ctypes.c_uint32,ctypes.c_uint32);C=ctypes.CFUNCTYPE(ctypes.c_uint32,*([ctypes.c_uint32]*5));lib=ctypes.CDLL(str(s/'retail-draft.so'));lib.draft.argtypes=[ctypes.c_uint32,ctypes.c_uint32,ctypes.c_uint32,L,S,C];lib.draft.restype=ctypes.c_uint32
for i in range(n):
 ram,_,_,_=c.prep(base,1);rect=rng.choice([0x80140000,0x80160000]);x=rng.getrandbits(32);y=rng.getrandbits(32);ram[rect&0x1fffff:(rect&0x1fffff)+8]=rng.randbytes(8)
 def sw(mem,a,v):struct.pack_into('<I',mem,a&0x1fffff,v)
 def sh(mem,a,v):struct.pack_into('<H',mem,a&0x1fffff,v)
 sh(ram,rect+4,rng.choice([0,1,0xffff,rng.getrandbits(16)]));sh(ram,rect+6,rng.choice([0,1,0xffff,rng.getrandbits(16)]));tables=[0x80095704,0x80150000];sw(ram,0x80095744,rng.choice(tables))
 for j,t in enumerate(tables):sw(ram,t+24,rng.getrandbits(32));sw(ram,t+8,0x80170000+4*j)
 native=bytearray(ram);oc=[];nc=[];loads=[];stores=[];rets={fn:rng.getrandbits(32) for fn in [0x80074E28,0x80170000,0x80170004]};replacement=rng.choice(tables);neww=rng.choice([0,1,0xffff]);newh=rng.choice([0,1,0xffff]);post=rng.getrandbits(32)
 def invoke(mem,fn,args,events,nl,ns):
  events.append([fn,args,nl,ns])
  if mutate:
   if fn==0x80074E28:sw(mem,0x80095744,replacement);sh(mem,rect+4,neww);sh(mem,rect+6,newh)
   else:sw(mem,0x800957ec,post)
  return rets[fn]
 def arity(fn):return 2 if fn==0x80074E28 else 4
 def model(fn):
  def cb(mem,regs):return invoke(mem,fn,regs[4:4+arity(fn)],oc,len(c.pl.LOADS),len(c.pl.STORES))
  return cb
 c.UNFAITHFUL[0]=False
 try:out=c.run_with_callees(ram,0x8007512C,[rect,x,y],rets,{fn:model(fn)for fn in rets})
 except(AssertionError,struct.error)as e:fault.append([i,str(e)]);continue
 if c.UNFAITHFUL[0]:drops+=1;continue
 def ld(a,k):v=int.from_bytes(native[a&0x1fffff:(a&0x1fffff)+k],'little');trace=(v|0xffff0000) if k==2 and v&0x8000 else v;loads.append((a,k,trace));return v
 def st(a,v):sw(native,a,v);stores.append((a,4,v))
 def call(fn,a,b,d,e):return invoke(native,fn,[a,b,d,e][:arity(fn)],nc,len(loads),len(stores))
 got=lib.draft(rect,x,y,L(ld),S(st),C(call));done+=1;regions=[(rect&0x1fffff,8),(0x957e4,20),(0x95744,4)]
 if got!=out[0] or loads!=out[1] or stores!=out[2] or nc!=oc or any(ram[a:a+size]!=native[a:a+size]for a,size in regions):fail.append(dict(case=i,returns=[got,out[0]],loads=[loads,out[1]],stores=[stores,out[2]],calls=[nc,oc]))
summary=dict(requested=n,compared=done,failures=len(fail),faults=len(fault),unfaithful=drops);tag='retail-mutating-check'if mutate else'retail-check';(s/f'{tag}-{n}.json').write_text(json.dumps(dict(summary=summary,failures=fail[:12],faults=fault[:12]),indent=2)+'\n');print(summary);sys.exit(bool(fail or fault or drops))
