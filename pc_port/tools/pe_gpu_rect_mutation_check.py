from pathlib import Path
import sys,json,ctypes,random,struct
root=Path(__file__).resolve().parents[2];sys.path.insert(0,str(root/'pc_port/tools'));import pe_cd_leaf_vectors as c
s=Path(sys.argv[1]);base=bytes(c.v.fixture((root/'build/disc1.candidate.exe').read_bytes(),0,0,0));rng=random.Random(0x80074E28);n=int(sys.argv[2]) if len(sys.argv)>2 else 6000;fail=[];fault=[];drops=done=0
L=ctypes.CFUNCTYPE(ctypes.c_uint32,ctypes.c_uint32,ctypes.c_uint32);C=ctypes.CFUNCTYPE(ctypes.c_uint32,*([ctypes.c_uint32]*6));lib=ctypes.CDLL(str(s/'retail-draft.so'));lib.draft.argtypes=[ctypes.c_uint32,ctypes.c_uint32,L,C];lib.draft.restype=ctypes.c_uint32
for i in range(n):
 ram,_,_,_=c.prep(base,1);debug=rng.choice([0,1,1,1,2,3,128,255]);ram[0x9574e]=debug;label=0x80140000;rect=rng.choice([0x80150000,0x80160000]);values=[rng.choice([-32768,-1,0,1,2,255,256,257,639,640,641,32767])for _ in range(4)];limits=[rng.choice([-32768,-1,0,1,256,640,1024,32767])for _ in range(2)];struct.pack_into('<hhhh',ram,rect&0x1fffff,*values);struct.pack_into('<hh',ram,0x95750,*limits);struct.pack_into('<I',ram,0x95748,0x80170000)
 native=bytearray(ram);oc=[];nc=[];loads=[];rets={fn:rng.getrandbits(32)for fn in [0x80170000,0x80170004]}
 def invoke(mem,fn,fmt,words,events,nl):
  events.append([fn,fmt,words,nl])
  if fmt!=0x800118A4:
   struct.pack_into('<I',mem,0x95748,0x80170004)
   struct.pack_into('<HHHH',mem,rect&0x1fffff,*[((i*1979+j*32769)^0xa55a)&0xffff for j in range(4)])
   mem[0x9574e]=0;struct.pack_into('<HH',mem,0x95750,0,0)
  return rets[fn]
 def model(fn):
  def cb(mem,regs):
   fmt=regs[4];words=[regs[5]]if fmt!=0x800118A4 else regs[5:8]+[struct.unpack_from('<I',mem,(regs[29]+16)&0x1fffff)[0]]
   return invoke(mem,fn,fmt,words,oc,len(c.pl.LOADS))
  return cb
 c.UNFAITHFUL[0]=False
 try:out=c.run_with_callees(ram,0x80074E28,[label,rect],rets,{fn:model(fn)for fn in rets})
 except(AssertionError,struct.error)as e:fault.append([i,str(e)]);continue
 if c.UNFAITHFUL[0]:drops+=1;continue
 def ld(a,k):v=(int.from_bytes(native[a&0x1fffff:(a&0x1fffff)+k],'little',signed=k==2)&0xffffffff);loads.append((a,k,v));return v
 def call(fn,fmt,a,b,d,e):return invoke(native,fn,fmt,[a]if fmt!=0x800118A4 else[a,b,d,e],nc,len(loads))
 got=lib.draft(label,rect,L(ld),C(call));done+=1
 if got!=out[0] or loads!=out[1] or out[2] or nc!=oc:fail.append(dict(case=i,debug=debug,rect=values,limits=limits,returns=[got,out[0]],loads=[loads,out[1]],stores=out[2],calls=[nc,oc]))
summary=dict(requested=n,compared=done,failures=len(fail),faults=len(fault),unfaithful=drops);(s/f'retail-mutating-check-{n}.json').write_text(json.dumps(dict(summary=summary,failures=fail[:12],faults=fault[:12]),indent=2)+'\n');print(summary);sys.exit(bool(fail or fault or drops))
