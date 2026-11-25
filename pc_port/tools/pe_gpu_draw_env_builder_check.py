from pathlib import Path
import sys,json,ctypes,random,struct
root=Path(__file__).resolve().parents[2];sys.path.insert(0,str(root/'pc_port/tools'));import pe_cd_leaf_vectors as c
s=Path(sys.argv[1]);base=bytes(c.v.fixture((root/'build/disc1.candidate.exe').read_bytes(),0,0,0));rng=random.Random(0x80075ee0);n=int(sys.argv[2]) if len(sys.argv)>2 else 6000;bad=[];fault=[];drops=done=0
L=ctypes.CFUNCTYPE(ctypes.c_uint32,ctypes.c_uint32,ctypes.c_uint32);S=ctypes.CFUNCTYPE(None,ctypes.c_uint32,ctypes.c_uint32,ctypes.c_uint32);C=ctypes.CFUNCTYPE(ctypes.c_uint32,ctypes.c_uint32,ctypes.c_uint32,ctypes.c_uint32,ctypes.c_uint32)
lib=ctypes.CDLL(str(s/'retail-draft.so'));lib.draft.argtypes=[ctypes.c_uint32,ctypes.c_uint32,L,S,C];lib.draft.restype=ctypes.c_uint32
entries=(0x80076170,0x80076208,0x800762a0,0x80076150,0x800762bc);arity={entries[0]:2,entries[1]:2,entries[2]:2,entries[3]:3,entries[4]:1}
for i in range(n):
 ram,_,_,_=c.prep(base,1);ram[0x140000:0x140100]=rng.randbytes(256);ram[0x95750:0x95754]=rng.randbytes(4)
 out=0x80140040;env=out+rng.choice([-32,-16,0,16,32,64]);ram[(env&0x1fffff)+0x18]=rng.choice([0,1]);rets={a:rng.getrandbits(32) for a in entries};native=bytearray(ram);calls1=[];calls2=[];loads=[];stores=[]
 def callback(a):
  def cb(mem,regs):calls1.append((a,tuple(regs[4:4+arity[a]])));return rets[a]
  return cb
 c.UNFAITHFUL[0]=False
 try:result=c.run_with_callees(ram,0x80075ee0,[out,env],{}, {a:callback(a) for a in entries})
 except (AssertionError,struct.error) as e:fault.append([i,str(e)]);continue
 if c.UNFAITHFUL[0]:drops+=1;continue
 def ld(a,k):
  signed=k==0x102;k=2 if signed else k;v=int.from_bytes(native[a&0x1fffff:(a&0x1fffff)+k],'little');v=(v|0xffff0000) if signed and v&0x8000 else v;loads.append((a,k,v));return v
 def st(a,v,k):native[a&0x1fffff:(a&0x1fffff)+k]=(v&((1<<(8*k))-1)).to_bytes(k,'little');stores.append((a,k,v&((1<<(8*k))-1)))
 def call(a,x,y,z):calls2.append((a,(x,y,z)[:arity[a]]));return rets[a]
 got=lib.draft(out,env,L(ld),S(st),C(call));done+=1;keep=lambda z:[x for x in z if (x[0]&0x1fffff)<0x1fe000]
 if got!=result[0] or loads!=keep(result[1]) or stores!=keep(result[2]) or calls1!=calls2 or native[0x140000:0x140100]!=ram[0x140000:0x140100]:bad.append(dict(case=i,returns=[got,result[0]],loads=[loads,keep(result[1])],stores=[stores,keep(result[2])],calls=[calls2,calls1]))
summary=dict(requested=n,compared=done,failures=len(bad),faults=len(fault),unfaithful=drops);(s/f'retail-check-{n}.json').write_text(json.dumps(dict(summary=summary,failures=bad[:3],faults=fault[:3]),indent=2)+'\n');print(summary);sys.exit(bool(bad or fault or drops))
