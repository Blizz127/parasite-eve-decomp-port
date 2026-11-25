from pathlib import Path
import sys,json,struct,ctypes,random
root=Path(__file__).resolve().parents[2];sys.path.insert(0,str(root/'pc_port/tools'));import pe_cd_leaf_vectors as c
s=Path(sys.argv[1]);base=bytes(c.v.fixture((root/'build/disc1.candidate.exe').read_bytes(),0,0,0))
L=ctypes.CFUNCTYPE(ctypes.c_uint32,ctypes.c_uint32,ctypes.c_uint32);S=ctypes.CFUNCTYPE(None,ctypes.c_uint32,ctypes.c_uint32,ctypes.c_uint32);C=ctypes.CFUNCTYPE(ctypes.c_uint32,ctypes.c_uint32,ctypes.c_uint32,ctypes.c_uint32);lib=ctypes.CDLL(str(s/'retail-draft.so'));lib.draft.argtypes=[ctypes.c_uint32]*4+[L,S,C];lib.draft.restype=ctypes.c_uint32
rng=random.Random(0x8007F0C8);n=int(sys.argv[2]) if len(sys.argv)>2 else 6000;fail=[];fault=[];drops=0;done=0
fns=[0x8007E6B0,0x8007E8F4,0x8007FBF0,0x80080950,0x80080C48]
for i in range(n):
 ram,_,_,_=c.prep(base,7919*(i+1));ram[0x1fe000:0x200000]=bytes([rng.randrange(256)])*0x2000;ram[0x150000:0x154000]=bytes([0x5A])*0x4000
 args=[rng.getrandbits(32),0x80150000,rng.choice([0,1,3,6,21,22,27,28,31,0x103,0x106,0x80000015,0xffffffff]),rng.getrandbits(32),rng.getrandbits(32)]
 serial=rng.choice([0,0x7fffffff,0xfffffffe,0xffffffff,rng.getrandbits(32)]);idx=rng.randrange(8)
 for a,v in [(0x150000,rng.getrandbits(32)),(0x1ff010,args[4]),(0x9B53C,serial),(0xA3608,rng.choice([0,4,5,7,8,0x7ffffffb,0x7ffffffc,0x7fffffff,0x80000000,0xffffffff])),(0xA3604,idx),(0xA3540+24*idx,rng.choice([0,1,(serial+1)&0xffffffff]))]:struct.pack_into('<I',ram,a,v)
 native=bytearray(ram);rets={0x80080C48:rng.choice([0,123,0x7fffffff,0x80000000,0xffffffff]),0x8007FBF0:rng.choice([0,1,2,0xffffffff]),0x8007E8F4:rng.getrandbits(32)};unique=rng.randrange(2);alloc_o=[0];alloc_n=[0];oc=[];nc=[];loads=[];stores=[]
 mutations={fn:[[(0xA3608,rng.getrandbits(32)),(0x9B53C,rng.getrandbits(32)),(0xA3604,rng.randrange(8)),(0x1fef99,rng.getrandbits(32)),(0x1fefa9,rng.getrandbits(32)),(0x150000,rng.getrandbits(32))] if rng.randrange(4)==0 else [] for _ in range(6)] for fn in fns};seen_o={fn:0 for fn in fns};seen_n={fn:0 for fn in fns}
 def mutate(mem,fn):
  seen=seen_n if mem is native else seen_o;k=min(seen[fn],5);seen[fn]+=1
  for a,v in mutations[fn][k]:struct.pack_into('<I',mem,a,v)
 def invoke(mem,fn,a,b,alloc,events,nl,ns):
  payload=bytes(mem[b&0x1fffff:(b&0x1fffff)+4]).hex()if fn==0x80080950 else (bytes(mem[0x1fef99:0x1fef9d]).hex()if fn==0x80080C48 else None)
  events.append([fn,a,b,payload,nl,ns]);mutate(mem,fn)
  if fn==0x8007E6B0:
   v=0x80153000+unique*24*alloc[0];alloc[0]+=1;return v
  if fn==0x80080950:mem[a&0x1fffff:(a&0x1fffff)+4]=mem[b&0x1fffff:(b&0x1fffff)+4];return 0
  return rets[fn]
 def model(fn):
  def cb(mem,regs):
   a=regs[4]&0xffffffff if fn in [0x80080950,0x80080C48,0x8007FBF0]else 0;b=regs[5]&0xffffffff if fn==0x80080950 else 0
   return invoke(mem,fn,a,b,alloc_o,oc,len(c.pl.LOADS),len(c.pl.STORES))
  return cb
 c.UNFAITHFUL[0]=False
 try:out=c.run_with_callees(ram,0x8007F0C8,args,rets,{fn:model(fn)for fn in fns})
 except (AssertionError,struct.error)as e:fault.append([i,str(e)]);continue
 if c.UNFAITHFUL[0]:drops+=1;continue
 def ld(a,k):v=int.from_bytes(native[a&0x1fffff:(a&0x1fffff)+k],'little');loads.append((a,k,v));return v
 def st(a,k,v):
  v&=(1<<(8*k))-1;native[a&0x1fffff:(a&0x1fffff)+k]=v.to_bytes(k,'little')
  if not 0x1fef80<=(a&0x1fffff)<0x1ff000:stores.append((a,k,v))
 def call(fn,a,b):return invoke(native,fn,a,b,alloc_n,nc,len(loads),len(stores))
 ret=lib.draft(*args[:4],L(ld),S(st),C(call));done+=1
 if [out[0],out[1],out[2],oc]!=[ret,loads,stores,nc]:fail.append(dict(case=i,args=args,expected=[out[0],out[1],out[2],oc],actual=[ret,loads,stores,nc]))
summary=dict(requested=n,compared=done,failures=len(fail),faults=len(fault),unfaithful=drops);(s/f'retail-mutating-check-{n}.json').write_text(json.dumps(dict(summary=summary,failures=fail,faults=fault),indent=2)+'\n');print(summary);sys.exit(bool(fail or fault or drops))
