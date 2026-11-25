from pathlib import Path
import sys,json,ctypes,random,hashlib
r=Path(__file__).resolve().parents[2];sys.path.insert(0,str(r/'pc_port/tools'));import pe_cd_leaf_vectors as c
s=Path(sys.argv[1]);img=Path(sys.argv[3]).read_bytes();assert hashlib.sha1(img).hexdigest()=='146c0ce7308bf9fdc2ba5a84230e198db0663f3b'
base=bytes(c.v.fixture((r/'build/disc1.candidate.exe').read_bytes(),0,0,0));movie=img[0x039f*0x800:0x03c5*0x800];off=0x8010c0d8-0x8010bcf8;assert hashlib.sha256(movie[off:off+36]).hexdigest()=='e74b3ac9231b36b93ae760b3c2cd8c018905679c0da656f07b8cbd164933d59b'
rng=random.Random(0x8010c0d8);n=int(sys.argv[2]);fail=[];fault=[];drops=done=0;C=ctypes.CFUNCTYPE(ctypes.c_uint32,ctypes.c_uint32,ctypes.c_uint32,ctypes.c_uint32);lib=ctypes.CDLL(str(s/'retail-draft.so'));lib.draft.argtypes=[ctypes.c_uint32,C];lib.draft.restype=ctypes.c_uint32
for i in range(n):
 ram,_,_,_=c.prep(base,1);ram[0x10bcf8:0x10bcf8+len(movie)]=movie;callback=rng.getrandbits(32);ret=rng.getrandbits(32);native=bytearray(ram);mutation=rng.getrandbits(8);calls1=[];calls2=[]
 def orig(mem,regs):calls1.append((0x80073cf4,regs[4]&0xffffffff,regs[5]&0xffffffff));mem[0x140000]=mutation;return ret
 c.UNFAITHFUL[0]=False
 try:res=c.run_with_callees(ram,0x8010c0d8,[callback],{}, {0x80073cf4:orig})
 except (AssertionError,IndexError) as e:fault.append([i,str(e)]);continue
 if c.UNFAITHFUL[0]:drops+=1;continue
 def call(a,b,d):calls2.append((a,b,d));native[0x140000]=mutation;return ret
 got=lib.draft(callback,C(call));done+=1
 if got!=res[0] or calls1!=calls2 or native[0x140000]!=ram[0x140000]:fail.append({'case':i,'returns':[got,res[0]],'calls':[calls2,calls1]})
summary=dict(requested=n,compared=done,failures=len(fail),faults=len(fault),unfaithful=drops);(s/f'retail-check-{n}.json').write_text(json.dumps({'summary':summary,'failures':fail[:3],'faults':fault[:3]},indent=2)+'\n');print(summary);sys.exit(bool(fail or fault or drops))
