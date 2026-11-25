from pathlib import Path
import ctypes,json,sys,random
root=Path(__file__).resolve().parents[2];sys.path.insert(0,str(root/'pc_port/tools'));import pe_cd_leaf_vectors as c
here=Path(sys.argv[1]);base=bytes(c.v.fixture((root/'build/disc1.candidate.exe').read_bytes(),0,0,0));entry=0x8007EC14
CALL=ctypes.CFUNCTYPE(ctypes.c_uint32,ctypes.c_uint32,ctypes.c_uint32);STORE=ctypes.CFUNCTYPE(None,ctypes.c_uint32,ctypes.c_uint32,ctypes.c_uint32);lib=ctypes.CDLL(str(here/'retail-draft.so'));lib.draft.argtypes=[CALL,STORE];lib.draft.restype=ctypes.c_uint32
callees=[0x8007E594,0x8007F994,0x8007FBC0,0x8007FBCC,0x8007FBD8,0x8007FBE4,0x80080940,0x80081E5C,0x800822BC];rng=random.Random(entry);n=int(sys.argv[2]) if len(sys.argv)>2 else 6000;fail=[];fault=[];drops=0;done=0
for i in range(n):
 ram,_,_,_=c.prep(base,7919*(i+1));rets={fn:rng.choice([0,0,1,0xffffffff,rng.getrandbits(32)])for fn in callees};orig=[];native=[];stores=[]
 def canon(fn,a):return [fn,a]if fn in (0x8007E594,0x80081E5C,0x8007FBC0,0x8007FBCC,0x8007FBD8,0x8007FBE4)else[fn]
 def model(fn):
  def cb(mem,regs):orig.append([canon(fn,regs[4]&0xffffffff),len(c.pl.STORES)]);return rets[fn]
  return cb
 c.UNFAITHFUL[0]=False
 try:res=c.run_with_callees(ram,entry,[],rets,{fn:model(fn)for fn in callees})
 except AssertionError as e:fault.append([i,str(e)]);continue
 if c.UNFAITHFUL[0]:drops+=1;continue
 def nc(fn,a):native.append([canon(fn,a),len(stores)]);return rets[fn]
 ret=lib.draft(CALL(nc),STORE(lambda a,s,v:stores.append((a,s,v))));done+=1
 if [res[0],res[2],orig]!=[ret,stores,native]:fail.append(dict(case=i,expected=[res[0],res[2],orig],actual=[ret,stores,native]))
summary=dict(requested=n,compared=done,failures=len(fail),faults=len(fault),unfaithful=drops);(here/f'retail-check-{n}.json').write_text(json.dumps(dict(summary=summary,failures=fail,faults=fault),indent=2)+'\n');print(summary);sys.exit(bool(fail or fault or drops))
