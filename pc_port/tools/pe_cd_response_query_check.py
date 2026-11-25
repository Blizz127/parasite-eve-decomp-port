from pathlib import Path
import sys,json,struct,ctypes,random
root=Path(__file__).resolve().parents[2];sys.path.insert(0,str(root/'pc_port/tools'));import pe_cd_leaf_vectors as c
s=Path(sys.argv[1]);base=bytes(c.v.fixture((root/'build/disc1.candidate.exe').read_bytes(),0,0,0))
L=ctypes.CFUNCTYPE(ctypes.c_uint32,ctypes.c_uint32,ctypes.c_uint32);S=ctypes.CFUNCTYPE(None,ctypes.c_uint32,ctypes.c_uint32,ctypes.c_uint32);C=ctypes.CFUNCTYPE(ctypes.c_uint32,ctypes.c_uint32,ctypes.c_uint32,ctypes.c_uint32);lib=ctypes.CDLL(str(s/'retail-draft.so'));lib.draft.argtypes=[ctypes.c_uint32]*2+[L,S,C];lib.draft.restype=ctypes.c_uint32
rng=random.Random(0x8007F418);n=int(sys.argv[2]) if len(sys.argv)>2 else 6000;fail=[];fault=[];drops=0;done=0
for i in range(n):
 ram,_,_,_=c.prep(base,31*(i+1));head=rng.randrange(8);serials=[rng.choice([0,1,42,0x7fffffff,0x80000000,0xffffffff,rng.getrandbits(32)])for _ in range(8)]
 for j,v in enumerate(serials):struct.pack_into('<I',ram,0xA3610+16*j,v)
 struct.pack_into('<I',ram,0xA3690,head);args=[rng.choice(serials+[0,1,42,rng.getrandbits(32)]),rng.choice([0,0x80150000,0x800A3504,0x800A3615])];native=bytearray(ram);oc=[];nc=[];loads=[];stores=[]
 def invoke(mem,fn,a,b,events,nl,ns):
  events.append([fn,a,b,bytes(mem[b&0x1fffff:(b&0x1fffff)+8]).hex()if fn==0x80080998 else None,nl,ns])
  if fn==0x80080998 and a:mem[a&0x1fffff:(a&0x1fffff)+8]=mem[b&0x1fffff:(b&0x1fffff)+8]
  return 0
 def model(fn):
  def cb(mem,regs):return invoke(mem,fn,regs[4]&0xffffffff,regs[5]&0xffffffff if fn==0x80080998 else 0,oc,len(c.pl.LOADS),len(c.pl.STORES))
  return cb
 c.UNFAITHFUL[0]=False
 try:out=c.run_with_callees(ram,0x8007F418,args,{0x8007FC64:0,0x80080998:0},{fn:model(fn)for fn in [0x8007FC64,0x80080998]})
 except (AssertionError,struct.error)as e:fault.append([i,str(e)]);continue
 if c.UNFAITHFUL[0]:drops+=1;continue
 def ld(a,k):v=int.from_bytes(native[a&0x1fffff:(a&0x1fffff)+k],'little');loads.append((a,k,v));return v
 def st(a,k,v):stores.append((a,k,v));native[a&0x1fffff:(a&0x1fffff)+k]=v.to_bytes(k,'little')
 def call(fn,a,b):return invoke(native,fn,a,b,nc,len(loads),len(stores))
 ret=lib.draft(*args,L(ld),S(st),C(call));done+=1
 if (ret,loads,stores,nc)!=(out[0],out[1],out[2],oc):fail.append(dict(case=i,args=args,head=head,serials=serials,ret=[ret,out[0]],loads=[loads,out[1]],stores=[stores,out[2]],calls=[nc,oc]))
summary=dict(requested=n,compared=done,failures=len(fail),faults=len(fault),unfaithful=drops);(s/f'retail-check-{n}.json').write_text(json.dumps(dict(summary=summary,failures=fail[:8],faults=fault[:8]),indent=2)+'\n');print(summary);sys.exit(bool(fail or fault or drops))
