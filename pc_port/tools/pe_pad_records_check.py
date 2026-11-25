from pathlib import Path
import sys,json,ctypes,random,struct
root=Path(__file__).resolve().parents[2];sys.path.insert(0,str(root/'pc_port/tools'));import pe_cd_leaf_vectors as c
# The shared oracle normally omits low RAM; include it for explicit null-buffer cases.
old_logged=c.pl._logged
c.pl._logged=lambda address: True if 0<=address<0x1000 else old_logged(address)
s=Path(sys.argv[1]);base=bytes(c.v.fixture((root/'build/disc1.candidate.exe').read_bytes(),0,0,0));rng=random.Random(0x800844E4);n=int(sys.argv[2]) if len(sys.argv)>2 else 6000;fail=[];fault=[];drops=done=0
L=ctypes.CFUNCTYPE(ctypes.c_uint32,ctypes.c_uint32,ctypes.c_uint32);S=ctypes.CFUNCTYPE(None,ctypes.c_uint32,ctypes.c_uint32,ctypes.c_uint32);C=ctypes.CFUNCTYPE(ctypes.c_uint32,*([ctypes.c_uint32]*3));lib=ctypes.CDLL(str(s/'retail-draft.so'));lib.draft.argtypes=[ctypes.c_uint32]*2+[L,S,C];lib.draft.restype=ctypes.c_uint32
for i in range(n):
 ram,_,_,_=c.prep(base,1);ram[0x9B700:0x9B800]=bytes([rng.randrange(256)])*256;ram[0xA5A00:0xA5E00]=bytes([rng.randrange(256)])*1024
 args=[rng.choice([0,0x80140000,0x80150000,0x800A5BA0,0x800A5C90,0x800A5B70,0x800A5C60,0x8009B75C])for _ in range(2)];native=bytearray(ram);oc=[];nc=[];loads=[];stores=[]
 def invoke(mem,fn,a,b,events,nl,ns):
  events.append([fn,a,b,nl,ns])
  if fn==0x80071A24:mem[a&0x1fffff:(a&0x1fffff)+b]=bytes(b)
  return (i*13579+fn)&0xffffffff
 def model(fn):
  def cb(mem,regs):return invoke(mem,fn,regs[4]if fn==0x80071A24 else 0,regs[5]if fn==0x80071A24 else 0,oc,len(c.pl.LOADS),len(c.pl.STORES))
  return cb
 c.UNFAITHFUL[0]=False
 try:out=c.run_with_callees(ram,0x800844E4,args,{fn:0 for fn in [0x80084B44,0x80071A24,0x80082ADC]},{fn:model(fn)for fn in [0x80084B44,0x80071A24,0x80082ADC]})
 except (AssertionError,struct.error)as e:fault.append([i,str(e)]);continue
 if c.UNFAITHFUL[0]:drops+=1;continue
 def ld(a,k):v=int.from_bytes(native[a&0x1fffff:(a&0x1fffff)+k],'little');loads.append((0x1F800000+a if a<0x1000 else a,k,v));return v
 def st(a,k,v):native[a&0x1fffff:(a&0x1fffff)+k]=v.to_bytes(k,'little');stores.append((0x1F800000+a if a<0x1000 else a,k,v))
 def call(fn,a,b):return invoke(native,fn,a,b,nc,len(loads),len(stores))
 got=lib.draft(*args,L(ld),S(st),C(call));done+=1
 if native[:0x1000]!=ram[:0x1000] or got!=out[0] or(loads,stores,nc)!=(out[1],out[2],oc):fail.append(dict(case=i,args=args,returns=[got,out[0]],loads=[loads,out[1]],stores=[stores,out[2]],calls=[nc,oc]))
summary=dict(requested=n,compared=done,failures=len(fail),faults=len(fault),unfaithful=drops);(s/f'retail-check-{n}.json').write_text(json.dumps(dict(summary=summary,failures=fail[:12],faults=fault[:12]),indent=2)+'\n');print(summary);sys.exit(bool(fail or fault or drops))
