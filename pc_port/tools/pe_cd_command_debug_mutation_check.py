from pathlib import Path
import sys,json,ctypes,random,hashlib,struct
r=Path(__file__).resolve().parents[2];sys.path.insert(0,str(r/'pc_port/tools'));import pe_cd_leaf_vectors as c
s=Path(sys.argv[1]);exe=(r/'build/disc1.candidate.exe').read_bytes();assert hashlib.sha1(exe).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b';base=bytes(c.v.fixture(exe,0,0,0));rng=random.Random(0x8007b558);n=int(sys.argv[2]) if len(sys.argv)>2 else 6000;bad=[];fault=[];drops=done=0
L=ctypes.CFUNCTYPE(ctypes.c_uint32,ctypes.c_uint32,ctypes.c_uint32);S=ctypes.CFUNCTYPE(None,ctypes.c_uint32,ctypes.c_uint32,ctypes.c_uint32);C=ctypes.CFUNCTYPE(ctypes.c_uint32,ctypes.c_uint32,ctypes.c_uint32,ctypes.c_uint32);lib=ctypes.CDLL(str(s/'prefix-debug-draft.so'));lib.draft.argtypes=[ctypes.c_uint32,ctypes.c_uint32,ctypes.c_uint32,L,S,C];lib.draft.restype=ctypes.c_uint32
for i in range(n):
 ram,_,_,_=c.prep(base,1);ram[0x140000:0x140100]=rng.randbytes(256);cmd=rng.choice([2,14,rng.randrange(32)])|(rng.getrandbits(24)<<8);id=cmd&255;inp=rng.choice([0,0x80140000]);mode=rng.randrange(2);initial=rng.randrange(5);inp=0x80140000 if not inp and not initial and id in (2,14) else inp;newcount=rng.randrange(5);inp=0x80140000 if not inp and not initial and newcount else inp;attr=rng.randrange(2);val=rng.getrandbits(8);struct.pack_into('<I',ram,0x9afc0,rng.choice([0,1,2,3,0xffffffff]));struct.pack_into('<I',ram,0x9afdc+4*id,rng.getrandbits(32))
 for a,p in ((0x8009b27c,0x80140080),(0x8009b284,0x80140090),(0x8009b280,0x801400a0)):struct.pack_into('<I',ram,a&0x1fffff,p)
 struct.pack_into('<I',ram,0x9b1fc+4*id,initial);struct.pack_into('<I',ram,0x9b0fc+4*id,attr);debug_after=rng.choice([0,1,2,0xffffffff]); count_after=rng.randrange(5); name_after=rng.getrandbits(32); inp=0x80140000 if not inp and struct.unpack_from("<i",ram,0x9afc0)[0]>=2 and not count_after and (id in (2,14) or newcount) else inp; native=bytearray(ram);calls1=[];calls2=[];loads=[];stores=[]
 def mutate(mem,calls):
  calls.append((0x8007b010,0,0));struct.pack_into('<I',mem,0x9b1fc+4*id,newcount);struct.pack_into('<I',mem,0x9b0fc+4*id,attr^1);mem[0x140000]=val;return 0x1234
 def log_mutate(mem,calls,a,b):
  calls.append((0x80071a74,a,b))
  if a==0x80011bb4:
   struct.pack_into('<I',mem,0x9afc0,debug_after);struct.pack_into('<I',mem,0x9b1fc+4*id,count_after);struct.pack_into('<I',mem,0x9afdc+4*id,name_after)
 c.pl.LOADS.clear();c.pl.STORES.clear();c.UNFAITHFUL[0]=False;stop=c.Stop((0x8007b010,0x80071a74,0x8007b714,c.v.RET));state={4:cmd,5:inp,6:0,7:mode,31:c.v.RET};pc=0x8007b558
 try:
  while True:
   regs=c.v.execute(ram,pc,(),stop_at=stop,initial_regs=state,instruction_budget=10000)
   if stop.hit not in (0x8007b010,0x80071a74):break
   state=dict(enumerate(regs))
   if stop.hit==0x8007b010:
    assert (regs[4]&0xffffffff)==0 and (regs[5]&0xffffffff)==0
    state[2]=mutate(ram,calls1)
   else:
    log_mutate(ram,calls1,regs[4]&0xffffffff,regs[5]&0xffffffff);state[2]=0
   pc=regs[31];stop=c.Stop((0x8007b010,0x80071a74,0x8007b714,c.v.RET))
 except (AssertionError,IndexError,struct.error) as e:fault.append([i,str(e)]);continue
 if c.UNFAITHFUL[0]:
  print("UNFAITHFUL",i,"cmd",id,"input",hex(inp),"initial",initial,"newcount",newcount);drops+=1;continue
 expected=0x100 if stop.hit==0x8007b714 else regs[2]&0xffffffff;original_loads=list(c.pl.LOADS);original_stores=list(c.pl.STORES);loads=[];stores=[]
 def ld(a,k):v=int.from_bytes(native[a&0x1fffff:(a&0x1fffff)+k],'little');loads.append((a,k,v));return v
 def st(a,v,k):native[a&0x1fffff:(a&0x1fffff)+k]=(v&((1<<(8*k))-1)).to_bytes(k,'little');stores.append((a,k,v&((1<<(8*k))-1)))
 def call(a,b,d):
  if a==0x8007b010:
   assert (b,d)==(0,0);return mutate(native,calls2)
  assert a==0x80071a74;log_mutate(native,calls2,b,d);return 0
 got=lib.draft(cmd,inp,mode,L(ld),S(st),C(call));done+=1;keep=lambda z:[x for x in z if (x[0]&0x1fffff)<0x1fe000]
 if got!=expected or calls1!=calls2 or loads!=keep(original_loads) or stores!=keep(original_stores) or native[0x9af00:0x9b300]!=ram[0x9af00:0x9b300] or native[0x140000:0x140100]!=ram[0x140000:0x140100]:bad.append(dict(case=i,returns=[got,expected],calls=[calls2,calls1],loads=[loads,keep(original_loads)],stores=[stores,keep(original_stores)]))
summary=dict(requested=n,compared=done,failures=len(bad),faults=len(fault),unfaithful=drops);(s/f'prefix-mutation-check-{n}.json').write_text(json.dumps(dict(summary=summary,scope='debug 0, 1, 2, 3 and -1; stops before synchronous wait; logging mutates debug, parameter count and command name',failures=bad[:2],faults=fault[:2]),indent=2)+'\n');print(summary);sys.exit(bool(bad or fault or drops))
