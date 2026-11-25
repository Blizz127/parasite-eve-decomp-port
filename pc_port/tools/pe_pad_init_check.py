from pathlib import Path
import sys,json,ctypes,random,struct
root=Path(__file__).resolve().parents[2];sys.path.insert(0,str(root/'pc_port/tools'));import pe_cd_leaf_vectors as c
s=Path(sys.argv[1]);base=bytes(c.v.fixture((root/'build/disc1.candidate.exe').read_bytes(),0,0,0));rng=random.Random(0x80082534);n=int(sys.argv[2]) if len(sys.argv)>2 else 6000;fail=[];fault=[];drops=done=0
C=ctypes.CFUNCTYPE(ctypes.c_uint32,*([ctypes.c_uint32]*5));lib=ctypes.CDLL(str(s/'retail-draft.so'));lib.draft.argtypes=[ctypes.c_uint32]*4+[C];lib.draft.restype=ctypes.c_uint32
for i in range(n):
 ram,_,_,_=c.prep(base,1);args=[rng.getrandbits(32)for _ in range(4)];result=rng.getrandbits(32);oc=[];nc=[]
 def model(mem,regs):oc.append([0x80082CF0,*regs[4:8]]);return result
 c.UNFAITHFUL[0]=False
 try:out=c.run_with_callees(ram,0x80082534,args,{0x80082CF0:result},{0x80082CF0:model})
 except (AssertionError,struct.error)as e:fault.append([i,str(e)]);continue
 if c.UNFAITHFUL[0]:drops+=1;continue
 def call(*args):nc.append(list(args));return result
 got=lib.draft(*args,C(call));done+=1
 if got!=out[0] or oc!=nc or out[1] or out[2]:fail.append(dict(case=i,original=out,draft=got,calls=[oc,nc]))
summary=dict(requested=n,compared=done,failures=len(fail),faults=len(fault),unfaithful=drops);(s/f'retail-check-{n}.json').write_text(json.dumps(dict(summary=summary,failures=fail[:12],faults=fault[:12]),indent=2)+'\n');print(summary);sys.exit(bool(fail or fault or drops))
