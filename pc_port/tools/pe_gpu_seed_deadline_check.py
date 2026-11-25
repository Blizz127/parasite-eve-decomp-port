from pathlib import Path
import sys,json,ctypes,random,struct
root=Path(__file__).resolve().parents[2];sys.path.insert(0,str(root/'pc_port/tools'));import pe_cd_leaf_vectors as c
s=Path(sys.argv[1]);base=bytes(c.v.fixture((root/'build/disc1.candidate.exe').read_bytes(),0,0,0));rng=random.Random(0x800773D0);n=int(sys.argv[2]) if len(sys.argv)>2 else 6000;mutate=len(sys.argv)>3;fail=[];fault=[];drops=done=0
S=ctypes.CFUNCTYPE(None,ctypes.c_uint32,ctypes.c_uint32);C=ctypes.CFUNCTYPE(ctypes.c_uint32,*([ctypes.c_uint32]*5));lib=ctypes.CDLL(str(s/'retail-draft.so'));lib.draft.argtypes=[S,C];lib.draft.restype=ctypes.c_uint32
for i in range(n):
 ram,_,_,_=c.prep(base,1);ram[0x95888:0x95890]=rng.randbytes(8);native=bytearray(ram);value=rng.choice([0,1,0xffffff0f,0xffffff10,0xffffffff,rng.getrandbits(32)]);post=rng.getrandbits(32);oc=[];nc=[];stores=[]
 def sw(mem,a,v):struct.pack_into('<I',mem,a&0x1fffff,v)
 def invoke(mem,args,events,nl,ns):
  events.append([0x80073a44,args,nl,ns])
  if mutate:sw(mem,0x80095888,post);sw(mem,0x8009588c,post^0xffffffff)
  return value
 def cb(mem,regs):return invoke(mem,[regs[4]],oc,len(c.pl.LOADS),len(c.pl.STORES))
 c.UNFAITHFUL[0]=False
 try:out=c.run_with_callees(ram,0x800773D0,[],{0x80073A44:value},{0x80073A44:cb})
 except(AssertionError,struct.error)as e:fault.append([i,str(e)]);continue
 if c.UNFAITHFUL[0]:drops+=1;continue
 def st(a,v):sw(native,a,v);stores.append((a,4,v))
 def call(fn,a,b,d,e):assert fn==0x80073a44;return invoke(native,[a],nc,0,len(stores))
 got=lib.draft(S(st),C(call));done+=1
 if got!=out[0] or out[1] or stores!=out[2] or nc!=oc or ram[0x95888:0x95890]!=native[0x95888:0x95890]:fail.append(dict(case=i,returns=[got,out[0]],loads=out[1],stores=[stores,out[2]],calls=[nc,oc]))
summary=dict(requested=n,compared=done,failures=len(fail),faults=len(fault),unfaithful=drops);tag='retail-mutating-check'if mutate else'retail-check';(s/f'{tag}-{n}.json').write_text(json.dumps(dict(summary=summary,failures=fail[:12],faults=fault[:12]),indent=2)+'\n');print(summary);sys.exit(bool(fail or fault or drops))
