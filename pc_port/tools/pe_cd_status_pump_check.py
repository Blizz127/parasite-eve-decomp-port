from pathlib import Path
import sys,json,struct,ctypes,random
root=Path(__file__).resolve().parents[2];sys.path.insert(0,str(root/'pc_port/tools'));import pe_cd_leaf_vectors as c
s=Path(sys.argv[1]);base=bytes(c.v.fixture((root/'build/disc1.candidate.exe').read_bytes(),0,0,0));rng=random.Random(0x8007FE24);n=int(sys.argv[2]) if len(sys.argv)>2 else 6000;fail=[];fault=[];drops=done=0
L=ctypes.CFUNCTYPE(ctypes.c_uint32,ctypes.c_uint32,ctypes.c_uint32);S=ctypes.CFUNCTYPE(None,ctypes.c_uint32,ctypes.c_uint32,ctypes.c_uint32);C=ctypes.CFUNCTYPE(ctypes.c_uint32,ctypes.c_uint32,ctypes.c_uint32,ctypes.c_uint32);lib=ctypes.CDLL(str(s/'retail-draft.so'));lib.draft.argtypes=[L,S,C];lib.draft.restype=None
for i in range(n):
 ram,_,_,_=c.prep(base,1);ram[0x9B000:0x9C000]=bytes(0x1000);ram[0xA3500:0xA3700]=bytes(0x200);ram[0x1FE000:0x200000]=bytes([rng.randrange(256)])*0x2000
 fields={0x9B574:rng.choice([0,1,2,2,2,3,4]),0x9B578:rng.choice(list(range(21))+[0xFFFFFFFF,0x80000000]),0x9B57C:rng.choice(list(range(19,27))+[0xFFFFFFFF,0x80000000]),0x9B554:rng.randrange(2),0xA36A0:rng.choice([0,0x80170000]),0x9B6A4:rng.randrange(2)}
 for a in [0x9B598,0x9B594,0x9B5A0]:fields[a]=rng.choice([0,0,1,2,0x7FFFFFFF,0x80000000,0xFFFFFFFF])
 for a in [0x9B58C,0x9B59C]:fields[a]=rng.choice([0,1,2,3,4,5,7,8,15,16,31,32,59,60,75,120,0x7FFFFFFF,0xFFFFFFFF,rng.getrandbits(32)])
 for a,v in fields.items():struct.pack_into('<I',ram,a,v)
 for a in [0x9B588,0x9B58A,0x9B58B]:ram[a]=rng.choice([0,1,2,255])
 native=bytearray(ram);oc=[];nc=[];loads=[];stores=[]
 def invoke(mem,fn,a,b,events,nl,ns):
  events.append([fn,a,b,bytes(mem[b&0x1fffff:(b&0x1fffff)+4]).hex()if fn==0x8007FCFC and b else None,nl,ns]);return 0
 def model(fn):
  def cb(mem,regs):return invoke(mem,fn,regs[4]&0xffffffff if fn==0x8007FCFC else 0,regs[5]&0xffffffff if fn==0x8007FCFC else 0,oc,len(c.pl.LOADS),len(c.pl.STORES))
  return cb
 c.UNFAITHFUL[0]=False
 try:out=c.run_with_callees(ram,0x8007FE24,[],{0x8007FCFC:0,0x800800F4:0,0x80170000:0},{fn:model(fn)for fn in [0x8007FCFC,0x800800F4,0x80170000]})
 except (AssertionError,struct.error)as e:fault.append([i,str(e)]);continue
 if c.UNFAITHFUL[0]:drops+=1;continue
 def ld(a,k):v=int.from_bytes(native[a&0x1fffff:(a&0x1fffff)+k],'little');loads.append((a,k,v));return v
 def st(a,k,v):
  native[a&0x1fffff:(a&0x1fffff)+k]=v.to_bytes(k,'little')
  if not 0x1fef80<=(a&0x1fffff)<0x1ff000:stores.append((a,k,v))
 def call(fn,a,b):return invoke(native,fn,a,b,nc,len(loads),len(stores))
 lib.draft(L(ld),S(st),C(call));done+=1
 if (loads,stores,nc)!=(out[1],out[2],oc):fail.append(dict(case=i,fields=fields,bytes={hex(a):ram[a]for a in [0x9B588,0x9B58A,0x9B58B]},loads=[loads,out[1]],stores=[stores,out[2]],calls=[nc,oc]))
summary=dict(requested=n,compared=done,failures=len(fail),faults=len(fault),unfaithful=drops);(s/f'retail-check-{n}.json').write_text(json.dumps(dict(summary=summary,failures=fail[:12],faults=fault[:12]),indent=2)+'\n');print(summary);sys.exit(bool(fail or fault or drops))
