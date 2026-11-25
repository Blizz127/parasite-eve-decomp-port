from pathlib import Path
import sys,json,ctypes,random,struct,hashlib
r=Path(__file__).resolve().parents[2];sys.path.insert(0,str(r/'pc_port/tools'));import pe_cd_leaf_vectors as c
s=Path(sys.argv[1]);exe=(r/'build/disc1.candidate.exe').read_bytes();assert hashlib.sha1(exe).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b';base=bytes(c.v.fixture(exe,0,0,0));rng=random.Random(0x8007ace0);n=int(sys.argv[2]) if len(sys.argv)>2 else 6000;bad=[];fault=[];drops=done=0;oldload=c.pl._ns['LOAD']
L=ctypes.CFUNCTYPE(ctypes.c_uint32,ctypes.c_uint32,ctypes.c_uint32);S=ctypes.CFUNCTYPE(None,ctypes.c_uint32,ctypes.c_uint32,ctypes.c_uint32);C=ctypes.CFUNCTYPE(ctypes.c_uint32,*([ctypes.c_uint32]*4));lib=ctypes.CDLL(str(s/'full-all-draft.so'));lib.draft.argtypes=[L,S,C];lib.draft.restype=ctypes.c_uint32
for i in range(n):
 ram,_,_,_=c.prep(base,1);ram[0x140000:0x140100]=rng.randbytes(256);ptrs=(0x80140080,0x80140090,0x801400a0,0x801400b0)
 for a,p in zip((0x8009b27c,0x8009b288,0x8009b284,0x8009b280),ptrs):struct.pack_into('<I',ram,a&0x1fffff,p)
 irq=i%8;targets=[0x8007ae5c,0x8007ae10,0x8007ad10,0x8007aedc,0x8007af5c];rng.shuffle(targets)
 for j,target in enumerate(targets):struct.pack_into('<I',ram,0x11b8c+4*j,target)
 changed_target=rng.choice(targets);data=rng.randbytes(rng.randrange(9));index=rng.randrange(16);ram[0x9afd5]=index;struct.pack_into('<I',ram,0x9b17c+4*index,rng.choice([0,rng.getrandbits(32)]));debug=rng.choice([0,1,2,0xffffffff]);changed=rng.choice([0,1,2,0xffffffff]);struct.pack_into('<I',ram,0x9afc0,debug);native=bytearray(ram);calls1=[];calls2=[];counts1=[0];counts2=[0];loads=[];stores=[]
 def hardware(mem,a,k,counts):
  if (a&0x1fffff)==(ptrs[1]&0x1fffff) and k==1:mem[a&0x1fffff]=(mem[a&0x1fffff]&~7)|irq
  if (a&0x1fffff)==(ptrs[0]&0x1fffff) and k==1:mem[a&0x1fffff]=(mem[a&0x1fffff]&~0x20)|(0x20 if counts[0]<len(data) else 0)
  if (a&0x1fffff)==(ptrs[3]&0x1fffff) and k==1:mem[a&0x1fffff]=data[counts[0]];counts[0]+=1
 def original_load(fmt,buf,a):hardware(buf,a,struct.calcsize(fmt),counts1);return oldload(fmt,buf,a)
 c.pl._ns['LOAD']=original_load;c.pl.LOADS.clear();c.pl.STORES.clear();c.UNFAITHFUL[0]=False;stop=c.Stop((0x80071a74,0x80073c5c,c.v.RET))
 try:
  state={31:c.v.RET};pc=0x8007aab4
  while True:
   regs=c.v.execute(ram,pc,(),stop_at=stop,initial_regs=state,instruction_budget=10000)
   if stop.hit not in (0x80071a74,0x80073c5c):break
   fmt=regs[4]&0xffffffff;calls1.append((fmt,) if fmt in (0x80011b44,0x80011b6c) else (fmt,regs[5]&0xffffffff) if fmt==0x80011b80 else (fmt,regs[5]&0xffffffff,regs[6]&0xffffffff,regs[7]&0xffffffff))
   if fmt==0x80011b44:
    struct.pack_into('<I',ram,0x9afc0,changed);struct.pack_into('<I',ram,0x11b8c+16,changed_target)
   state=dict(enumerate(regs));state[2]=0;pc=regs[31];stop=c.Stop((0x80071a74,0x80073c5c,c.v.RET))
 except (AssertionError,IndexError,struct.error) as e:fault.append([i,str(e)]);continue
 finally:c.pl._ns['LOAD']=oldload
 if stop.hit!=c.v.RET or c.UNFAITHFUL[0]:drops+=1;continue
 original_loads=list(c.pl.LOADS);original_stores=list(c.pl.STORES);loads=[];stores=[]
 def ld(a,k):hardware(native,a,k,counts2);v=int.from_bytes(native[a&0x1fffff:(a&0x1fffff)+k],'little');loads.append((a,k,v));return v
 def st(a,v,k):native[a&0x1fffff:(a&0x1fffff)+k]=(v&((1<<(8*k))-1)).to_bytes(k,'little');stores.append((a,k,v&((1<<(8*k))-1)))
 def call(fmt,a1,a2,a3):
  calls2.append((fmt,) if fmt in (0x80011b44,0x80011b6c) else (fmt,a1) if fmt==0x80011b80 else (fmt,a1,a2,a3))
  if fmt==0x80011b44:
   struct.pack_into('<I',native,0x9afc0,changed);struct.pack_into('<I',native,0x11b8c+16,changed_target)
  return 0
 got=lib.draft(L(ld),S(st),C(call));done+=1;keep=lambda z:[x for x in z if (x[0]&0x1fffff)<0x1fe000]
 if calls1!=calls2 or got!=(regs[2]&0xffffffff) or loads!=keep(original_loads) or stores!=keep(original_stores) or native[0x9af00:0x9b300]!=ram[0x9af00:0x9b300] or native[0x140000:0x140100]!=ram[0x140000:0x140100] or native[0xa3460:0xa3478]!=ram[0xa3460:0xa3478] or native[0x11b8c:0x11ba0]!=ram[0x11b8c:0x11ba0]:bad.append(dict(case=i,irq=irq,returns=[got,regs[2]],calls=[calls2,calls1],loads=[loads,keep(original_loads)],stores=[stores,keep(original_stores)]))
summary=dict(requested=n,compared=done,failures=len(bad),faults=len(fault),unfaithful=drops);(s/f'full-table-check-{n}.json').write_text(json.dumps(dict(summary=summary,failures=bad[:2],faults=fault[:2]),indent=2)+'\n');print(summary);sys.exit(bool(bad or fault or drops))
