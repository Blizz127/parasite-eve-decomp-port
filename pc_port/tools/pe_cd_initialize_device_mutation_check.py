from pathlib import Path
import sys,json,ctypes,random,struct,hashlib
root=Path(__file__).resolve().parents[2];sys.path.insert(0,str(root/'pc_port/tools'));import pe_cd_leaf_vectors as c
s=Path(sys.argv[1]);exe=(root/'build/disc1.candidate.exe').read_bytes();assert hashlib.sha1(exe).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b';base=bytes(c.v.fixture(exe,0,0,0));rng=random.Random(0x8007bbfc);n=int(sys.argv[2]) if len(sys.argv)>2 else 6000;bad=[];fault=[];drops=done=0
L=ctypes.CFUNCTYPE(ctypes.c_uint32,ctypes.c_uint32,ctypes.c_uint32);S=ctypes.CFUNCTYPE(None,ctypes.c_uint32,ctypes.c_uint32,ctypes.c_uint32);C=ctypes.CFUNCTYPE(ctypes.c_uint32,*([ctypes.c_uint32]*5));lib=ctypes.CDLL(str(s/'draft.so'));lib.draft.argtypes=[L,S,C];lib.draft.restype=ctypes.c_uint32;oldload=c.pl._ns['LOAD'];arity={0x80073c5c:1,0x80071a74:2,0x80073c94:0,0x80073cc4:2,0x8007b558:4,0x8007b010:2}
for i in range(n):
 ram,_,_,_=c.prep(base,1);ram[0x140000:0x140100]=rng.randbytes(256);ptrs=(0x80140080,0x80140090,0x801400a0,0x801400b0)
 for a,p in zip((0x8009b27c,0x8009b288,0x8009b284,0x8009b28c),ptrs):struct.pack_into('<I',ram,a&0x1fffff,p)
 initial=rng.randrange(8);ready=rng.randrange(4);responses=[rng.randrange(3) for _ in range(4)];sync=rng.randrange(5);flag=rng.choice([0,0x10]);alternate=(0x801400c0,0x801400d0,0x801400e0,0x801400f0);trigger=rng.randrange(1,8);event1=[0];event2=[0];native=bytearray(ram);counts1=[0];counts2=[0];cbcounts1=[0];cbcounts2=[0];loads=[];stores=[];calls1=[];calls2=[]
 def hardware(mem,a,k,counts):
  if ((a&0x1fffff)==(ptrs[1]&0x1fffff) or (a&0x1fffff)==(alternate[1]&0x1fffff)) and k==1:
   counts[0]+=1;pos=a&0x1fffff;mem[pos]=(mem[pos]&~7)|(initial if counts[0]<=ready else 0)
 def mutate(mem,target,args,counts,calls,events):
  events[0]+=1
  if events[0]==trigger:
   for a,p in zip((0x8009b27c,0x8009b288,0x8009b284,0x8009b28c),alternate):struct.pack_into("<I",mem,a&0x1fffff,p)
  calls.append((target,*args[:arity[target]]))
  if target==0x8007b558:
   counts[0]+=1
   if counts[0]==1:struct.pack_into('<I',mem,0x9afc4,flag)
   return responses[counts[0]-1]
  return sync if target==0x8007b010 else 0
 def orig(target):return lambda mem,regs:mutate(mem,target,[regs[j]&0xffffffff for j in range(4,8)],cbcounts1,calls1,event1)
 def original_load(fmt,buf,a):hardware(buf,a,struct.calcsize(fmt),counts1);return oldload(fmt,buf,a)
 c.pl._ns['LOAD']=original_load;c.UNFAITHFUL[0]=False
 try:result=c.run_with_callees(ram,0x8007bbfc,[],{}, {a:orig(a) for a in arity})
 except (AssertionError,IndexError,struct.error) as e:fault.append([i,str(e)]);continue
 finally:c.pl._ns['LOAD']=oldload
 if c.UNFAITHFUL[0]:drops+=1;continue
 def ld(a,k):hardware(native,a,k,counts2);v=int.from_bytes(native[a&0x1fffff:(a&0x1fffff)+k],'little');loads.append((a,k,v));return v
 def st(a,v,k):native[a&0x1fffff:(a&0x1fffff)+k]=(v&((1<<(8*k))-1)).to_bytes(k,'little');stores.append((a,k,v&((1<<(8*k))-1)))
 def call(a,b,d,e,f):return mutate(native,a,[b,d,e,f],cbcounts2,calls2,event2)
 got=lib.draft(L(ld),S(st),C(call));done+=1;keep=lambda z:[x for x in z if (x[0]&0x1fffff)<0x1fe000]
 if got!=result[0] or calls1!=calls2 or loads!=keep(result[1]) or stores!=keep(result[2]) or native[0x9af00:0x9b300]!=ram[0x9af00:0x9b300] or native[0x140000:0x140100]!=ram[0x140000:0x140100]:bad.append(dict(case=i,returns=[got,result[0]],calls=[calls2,calls1],loads=[loads,keep(result[1])],stores=[stores,keep(result[2])]))
summary=dict(requested=n,compared=done,failures=len(bad),faults=len(fault),unfaithful=drops);(s/f'mutation-check-{n}.json').write_text(json.dumps(dict(summary=summary,original_sha256=hashlib.sha256(base[0x7bbfc:0x7bddc]).hexdigest(),failures=bad[:2],faults=fault[:2]),indent=2)+'\n');print(summary);sys.exit(bool(bad or fault or drops))
