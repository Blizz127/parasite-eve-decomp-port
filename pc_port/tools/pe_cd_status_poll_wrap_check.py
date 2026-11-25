from pathlib import Path
import sys,json,ctypes,random,hashlib,struct
r=Path(__file__).resolve().parents[2];sys.path.insert(0,str(r/'pc_port/tools'));import pe_cd_leaf_vectors as c
s=Path(sys.argv[1]);exe=(r/'build/disc1.candidate.exe').read_bytes();assert hashlib.sha1(exe).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b';base=bytes(c.v.fixture(exe,0,0,0));rng=random.Random(0x8007b010);n=int(sys.argv[2]) if len(sys.argv)>2 else 6000;bad=[];fault=[];drops=done=0
L=ctypes.CFUNCTYPE(ctypes.c_uint32,ctypes.c_uint32,ctypes.c_uint32);S=ctypes.CFUNCTYPE(None,ctypes.c_uint32,ctypes.c_uint32,ctypes.c_uint32);C=ctypes.CFUNCTYPE(ctypes.c_uint32,*([ctypes.c_uint32]*6));lib=ctypes.CDLL(str(s/'full-draft.so'));lib.draft.argtypes=[ctypes.c_uint32,ctypes.c_uint32,L,S,C];lib.draft.restype=ctypes.c_uint32
for i in range(n):
 ram,_,_,_=c.prep(base,1);ram[0x9af00:0x9b300]=rng.randbytes(0x400);ram[0xa3450:0xa3490]=rng.randbytes(0x40);ram[0x140000:0x140100]=rng.randbytes(256);scenario=0;cause=0;mode=rng.randrange(2);dst=rng.choice([0,0x80140000,0x800a3460,0x800a3461,0x800a345f]);final=rng.choice([0,1,2,5]) if mode else rng.choice([2,5]);timer=rng.getrandbits(32);second=rng.getrandbits(32);bits=[rng.choice([1,2,4,6,7]) for _ in range(rng.randrange(4))]+[0];change=rng.randrange(2);ptr0=0x80140080;ptr1=0x80140090;cb1=0x80170000;cb2=0x80170040;status0=rng.randrange(16);status1=rng.randrange(16);last=rng.randrange(16);msg=rng.getrandbits(32)
 struct.pack_into('<I',ram,0x9b27c,ptr0);struct.pack_into('<I',ram,0x9afb8,rng.choice([0,cb1,cb2]));struct.pack_into('<I',ram,0x9afb4,rng.choice([0,cb1,cb2]));ram[0x9b294]=rng.randrange(8);ram[0x9b295]=rng.randrange(8);native=bytearray(ram);ack1=[0];ack2=[0];tm1=[0];tm2=[0];calls1=[];calls2=[];loads=[];stores=[]
 def action(mem,target,args,ack,tm,calls):
  arity={0x80073a44:1,0x80073de8:0,0x8007aab4:0,cb1:2,cb2:2,0x80073c5c:1,0x80071a74:5,0x8007b9ec:0}[target];calls.append((target,*args[:arity]))
  if target==0x80073a44:
   tm[0]+=1
   if scenario==2 and tm[0]==2 and cause:struct.pack_into('<I',mem,0xa347c,0x3c0001)
   return second if tm[0]==2 else timer
  if target==0x80073de8:
   if scenario==0:mem[0x9b294]=final
   return 1 if scenario==1 else 0
  if target==0x8007aab4:
   v=bits[ack[0]];ack[0]+=1
   if not v:mem[0x9b294]=final
   return v
  if target in (cb1,cb2) and change:struct.pack_into('<I',mem,0x9b27c,ptr1);struct.pack_into('<I',mem,0x9afb4,cb2)
  if target==0x80073c5c:mem[0x9b294]=status0;mem[0x9b295]=status1;mem[0x9afd5]=last;struct.pack_into('<I',mem,0xa3480,msg)
  return 0
 def orig(target):
  def cb(mem,regs):
   args=[regs[j]&0xffffffff for j in range(4,8)]+[struct.unpack_from('<I',mem,(regs[29]+0x10)&0x1fffff)[0]]
   return action(mem,target,args,ack1,tm1,calls1)
  return cb
 c.UNFAITHFUL[0]=False
 try:res=c.run_with_callees(ram,0x8007b010,[mode,dst],{}, {a:orig(a) for a in (0x80073a44,0x80073de8,0x8007aab4,cb1,cb2,0x80073c5c,0x80071a74,0x8007b9ec)})
 except (AssertionError,IndexError,struct.error) as e:fault.append([i,str(e)]);continue
 if c.UNFAITHFUL[0]:drops+=1;continue
 def ld(a,k):v=int.from_bytes(native[a&0x1fffff:(a&0x1fffff)+k],'little');loads.append((a,k,v));return v
 def st(a,v,k):native[a&0x1fffff:(a&0x1fffff)+k]=(v&((1<<(8*k))-1)).to_bytes(k,'little');stores.append((a,k,v&((1<<(8*k))-1)))
 def call(a,b,d,e,f,g):return action(native,a,[b,d,e,f,g],ack2,tm2,calls2)
 got=lib.draft(mode,dst,L(ld),S(st),C(call));done+=1;keep=lambda z:[x for x in z if (x[0]&0x1fffff)<0x1fe000]
 if got!=res[0] or calls1!=calls2 or loads!=keep(res[1]) or stores!=keep(res[2]) or native[0xa3450:0xa3490]!=ram[0xa3450:0xa3490] or native[0x140000:0x140100]!=ram[0x140000:0x140100] or native[0x9af00:0x9b300]!=ram[0x9af00:0x9b300]:bad.append(dict(case=i,returns=[got,res[0]],calls=[calls2,calls1],loads=[loads,keep(res[1])],stores=[stores,keep(res[2])]))
summary=dict(requested=n,compared=done,failures=len(bad),faults=len(fault),unfaithful=drops);(s/f'wrap-check-{n}.json').write_text(json.dumps(dict(summary=summary,original_sha256=hashlib.sha256(base[0x7b010:0x7b290]).hexdigest(),scope='finite polls and IRQs; scripted timer, IRQ, callbacks, diagnostics and flush',failures=bad[:2],faults=fault[:2]),indent=2)+'\n');print(summary);sys.exit(bool(bad or fault or drops))
