from pathlib import Path
import sys,json,struct,ctypes,random
root=Path(__file__).resolve().parents[2];sys.path.insert(0,str(root/'pc_port/tools'));import pe_cd_leaf_vectors as c
base=bytes(c.v.fixture((root/'build/disc1.candidate.exe').read_bytes(),0,0,0));s=Path(sys.argv[1])
L=ctypes.CFUNCTYPE(ctypes.c_uint32,ctypes.c_uint32,ctypes.c_uint32);S=ctypes.CFUNCTYPE(None,ctypes.c_uint32,ctypes.c_uint32,ctypes.c_uint32);C=ctypes.CFUNCTYPE(ctypes.c_uint32,ctypes.c_uint32,ctypes.c_uint32,ctypes.c_uint32);lib=ctypes.CDLL(str(s/'retail-draft.so'));lib.draft.argtypes=[ctypes.c_uint32]*4+[L,S,C];lib.draft.restype=ctypes.c_uint32
rng=random.Random(0x8007EE84);n=int(sys.argv[2]) if len(sys.argv)>2 else 6000;fail=[];fault=[];drop=0;done=0
for i in range(n):
 ram,_,_,_=c.prep(base,7919*(i+1));args=[rng.getrandbits(32),rng.choice([0,0x80150000]),0x80151000,0x80152000];ram[0x150000:0x154000]=bytes([0x5A])*0x4000
 serial=rng.choice([0,1,0x7fffffff,0xfffffffe,0xffffffff,rng.getrandbits(32)]);count=rng.choice([0,1,6,7,8,9,0xffffffff,0x80000000]);idx=rng.randrange(8);active=rng.choice([0,(serial+1)&0xffffffff,(serial+2)&0xffffffff]);rec=0x80153000
 for a,val in [(0x9B4BC+4*(args[0]&255),rng.choice([0,0,1,2,0xffffffff])),(0xA3608,count),(0x9B53C,serial),(0xA3604,idx),(0xA3540+24*idx,active)]:struct.pack_into('<I',ram,a,val)
 native=bytearray(ram)
 mutations={fn:[[(0xA3608,rng.choice([0,1,7,8,0xffffffff])),(0x9B53C,rng.getrandbits(32)),(0xA3604,rng.randrange(8))] if rng.randrange(3)==0 else [] for _ in range(3)] for fn in [0x8007E6B0,0x8007E8F4,0x8007FBF0,0x80080950]};seen_o={fn:0 for fn in mutations};seen_n={fn:0 for fn in mutations}
 def mutate(mem,fn,seen):
  k=min(seen[fn],2);seen[fn]+=1
  for a,v in mutations[fn][k]:struct.pack_into('<I',mem,a,v)
 rets={0x8007E6B0:rec,0x8007E8F4:rng.getrandbits(32),0x8007FBF0:rng.choice([0,1,2,0xffffffff]),0x80080950:rng.getrandbits(32)};oc=[]
 def model(fn):
  def cb(mem,regs):oc.append([fn,0 if fn in (0x8007E6B0,0x8007E8F4) else regs[4]&0xffffffff,regs[5]&0xffffffff if fn==0x80080950 else 0,len(c.pl.STORES)]);mutate(mem,fn,seen_o);return rets[fn]
  return cb
 c.UNFAITHFUL[0]=False
 try:out=c.run_with_callees(ram,0x8007EE84,args,rets,{fn:model(fn)for fn in rets})
 except (AssertionError,struct.error)as e:fault.append([i,str(e)]);continue
 if c.UNFAITHFUL[0]:drop+=1;continue
 loads=[];stores=[];calls=[]
 def ld(a,n):v=int.from_bytes(native[a&0x1fffff:(a&0x1fffff)+n],'little');loads.append((a,n,v));return v
 def st(a,n,v):v&=(1<<(8*n))-1;native[a&0x1fffff:(a&0x1fffff)+n]=v.to_bytes(n,'little');stores.append((a,n,v))
 def call(fn,a,b):calls.append([fn,a,b,len(stores)]);mutate(native,fn,seen_n);return rets[fn]
 ret=lib.draft(*args,L(ld),S(st),C(call));done+=1
 if [out[0],out[1],out[2],oc]!=[ret,loads,stores,calls]:fail.append(dict(case=i,args=args,expected=[out[0],out[1],out[2],oc],actual=[ret,loads,stores,calls]))
summary=dict(requested=n,compared=done,failures=len(fail),faults=len(fault),unfaithful=drop);(s/f'retail-null-mutating-check-{n}.json').write_text(json.dumps(dict(summary=summary,failures=fail,faults=fault),indent=2)+'\n');print(summary);sys.exit(bool(fail or fault or drop))
