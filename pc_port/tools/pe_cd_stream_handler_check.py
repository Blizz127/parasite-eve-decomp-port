from pathlib import Path
import sys,json,ctypes,random,hashlib,struct
r=Path(__file__).resolve().parents[2];sys.path.insert(0,str(r/'pc_port/tools'));import pe_cd_leaf_vectors as c
s=Path(sys.argv[1]);exe=(r/'build/disc1.candidate.exe').read_bytes();assert hashlib.sha1(exe).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b';base=bytes(c.v.fixture(exe,0,0,0));rng=random.Random(0x8007c564);n=int(sys.argv[2]) if len(sys.argv)>2 else 6000;bad=[];fault=[];drops=done=0;markers=(0x8007c5e8,0x8007c62c,0x8007c698,0x8007c7bc,0x8007c834,0x8007c8b4,0x8007c92c,0x8007c9e8,0x8007ca34,0x8007cad4,0x8007cb78,0x8007cc20,0x8007cc34,0x8007ccdc,0x8007cd9c,0x8007ce14,0x8007ceac,0x8007ce80,0x8007c214,0x8007c444);coverage={hex(a):0 for a in markers};callcounts={}
L=ctypes.CFUNCTYPE(ctypes.c_uint32,ctypes.c_uint32,ctypes.c_uint32);S=ctypes.CFUNCTYPE(None,ctypes.c_uint32,ctypes.c_uint32,ctypes.c_uint32);C=ctypes.CFUNCTYPE(ctypes.c_uint32,*([ctypes.c_uint32]*8));lib=ctypes.CDLL(str(s/'combined.so'));lib.draft.argtypes=[L,S,C];lib.draft.restype=ctypes.c_uint32
orig_load=c.pl._ns['LOAD'];orig_store=c.v._store
for i in range(n):
 ram,_,_,_=c.prep(base,1);ram[0x140000:0x140400]=rng.randbytes(1024);ram[0x141000:0x141100]=rng.randbytes(256);ram[0x160000:0x164000]=rng.randbytes(0x4000);idx=rng.randrange(4);size=rng.randrange(1,7);entry=0x80140000+idx*32;entry2=0x80140180;status=rng.choice([0,0,0,1,2]);mode=rng.randrange(2);busy=rng.randrange(2);pending=rng.choice([0,0,0x80160000]);seq=rng.choice([0,1,2,3]);old=rng.choice([seq,seq,0xffff]);count=rng.choice([1,2,3,5]);idv=rng.choice([0,1,2]);target=rng.choice([0,idv,3]);filterflag=rng.randrange(2);want=rng.choice([0,idv,3]);resp=rng.choice([0,0,0,5]);b0=rng.choice([0,0,4]);b1=rng.randrange(256);newpending=rng.choice([0,0x80160000]);mutation=rng.randrange(2);callback=0x80170000;callback2=0x80170040;hascb=rng.randrange(2);ret=rng.getrandbits(32);t=rng.randrange(3);incoming=rng.choice([0x160,0x160,0x160,0x123]);incoming_header=rng.choice([0x0400,0x0400,0x0800])
 for a,v in ((0xb89f4,status),(0xa801c,mode),(0x9b34c,0x80141000),(0x141000,busy<<24),(0xc0db8,pending),(0xbcd7c,t),(0xbe998,idx),(0xc0dc8,0x80140000),(0xc20c4,size),(0xc0dbc,rng.choice([0,0,2,4])),(0xc0dc0,filterflag),(0xb6918,want),(0xb8620,1),(0xa5d54,target),(0xbe9e4,rng.randrange(idx+1)),(0xb0ccc,callback if hascb else 0),(0xb0cc8,callback2 if rng.randrange(2) else 0),(0xb6914,2),(0x9b32c,0x80141004),(0x9b338,0x80141008),(0x9b33c,0x8014100c),(0x9b340,0x80141010),(0x9b334,0x80141014),(0x9b35c,0x1f8010b8),(0x9b348,0x8014101c),(0x9b344,0x80141020),(0x10b8,0)):
  struct.pack_into('<I',ram,a,v)
 struct.pack_into('<H',ram,0xa8018,old);struct.pack_into('<H',ram,(entry&0x1fffff),0);struct.pack_into('<H',ram,(entry&0x1fffff)+2,0x0400);struct.pack_into('<H',ram,(entry&0x1fffff)+4,seq);struct.pack_into('<H',ram,(entry&0x1fffff)+6,count);struct.pack_into('<H',ram,(entry&0x1fffff)+8,idv)
 if pending:struct.pack_into('<HHHHH',ram,(pending&0x1fffff)+(t<<11),incoming,incoming_header,seq,count,idv)
 native=bytearray(ram);delay_ready=i%5;delay_busy=(i//5)%5;ct1=[0,0,False];ct2=[0,0,False];loads=[];stores=[];calls1=[];calls2=[];visited=set();c.pl.LOADS.clear();c.pl.STORES.clear();c.UNFAITHFUL[0]=False;pc=0x8007c564;state={31:c.v.RET};targets=(0x8007b290,callback,callback2);stop=c.Stop((c.v.RET,)+targets)
 def act(mem,calls,target,args):
  arity={0x8007b290:2,callback:2,callback2:4}[target];calls.append((target,*args[:arity]))
  if target==0x8007b290:
   off=args[1]&0x1fffff;mem[off]=b0;mem[off+1]=b1;return resp
  return ret
 def hwload(mem,a,k,ct):
  if a==0x141004 and k==1:
   ct[0]+=1;mem[a]=(mem[a]|0x40) if ct[0]>delay_ready else (mem[a]&~0x40)
  if a==0x10b8 and k==4 and ct[2]:
   ct[1]+=1;v=struct.unpack_from('<I',mem,a)[0]
   if ct[1]>delay_busy:v&=~0x01000000;ct[2]=False
   struct.pack_into('<I',mem,a,v)
 def hwstore(mem,a,k,v,ct):
  if a==0x10b8 and k==4 and v&0x01000000:
   ct[1]=0;ct[2]=True
   dest=struct.unpack_from('<I',mem,0x10b0)[0]&0x1fffff;words=struct.unpack_from('<I',mem,0x10b4)[0]&0xffff
   if words==8:struct.pack_into('<HHHHH',mem,dest,incoming,incoming_header,seq,count,idv)
   elif words==0x1f8:mem[dest:dest+words*4]=mem[0x160000:0x160000+words*4]
 def wrappedload(fmt,buf,a):hwload(buf,a,struct.calcsize(fmt),ct1);return orig_load(fmt,buf,a)
 def wrappedstore(fmt,buf,a,v):orig_store(fmt,buf,a,v);hwstore(buf,a,struct.calcsize(fmt),v,ct1)
 c.pl._ns['LOAD']=wrappedload;c.v._store=wrappedstore
 try:
  for _ in range(10):
   rr=c.v.execute(ram,pc,(),stop_at=stop,initial_regs=state,instruction_budget=100000,visited_pcs=visited)
   if stop.hit==c.v.RET:break
   if stop.hit not in targets:raise AssertionError('unexpected stop')
   args=[rr[j]&0xffffffff for j in range(4,8)]+[struct.unpack_from('<I',ram,(rr[29]+0x10+4*k)&0x1fffff)[0] for k in range(3)]
   rr[2]=act(ram,calls1,stop.hit,args);state=dict(enumerate(rr));pc=rr[31];stop=c.Stop((c.v.RET,)+targets)
  else:raise AssertionError('callback budget')
 except (AssertionError,IndexError,struct.error) as e:fault.append([i,str(e)]);continue
 finally:c.pl._ns['LOAD']=orig_load;c.v._store=orig_store
 if c.UNFAITHFUL[0]:drops+=1;continue
 expected=rr[2]&0xffffffff;ol=list(c.pl.LOADS);os=list(c.pl.STORES)
 for a in markers:
  if a in visited:coverage[hex(a)]+=1
 for ev in calls1:callcounts[hex(ev[0])]=callcounts.get(hex(ev[0]),0)+1
 def ld(a,k):off=a&0x1fffff;hwload(native,off,k,ct2);v=int.from_bytes(native[off:off+k],'little');loads.append((a,k,(v|0xffff0000) if k==2 and a in (0x800a8018,0x80140000) and v&0x8000 else v));return v
 def st(a,v,k):off=a&0x1fffff;native[off:off+k]=(v&((1<<(8*k))-1)).to_bytes(k,'little');stores.append((a,k,v&((1<<(8*k))-1)));hwstore(native,off,k,v,ct2)
 def call(a,b,d,e,f,g,h,j):return act(native,calls2,a,[b,d,e,f,g,h,j])
 got=lib.draft(L(ld),S(st),C(call));done+=1
 def keep(z):return [x for x in z if (x[0]&0x1fffff)<0x1fe000 and not 0xa3490<=(x[0]&0x1fffff)<0xa3494 and not (0x140000<=(x[0]&0x1fffff)<0x140400 and ((x[0]&31)>=28))]
 if got!=expected or calls1!=calls2 or keep(loads)!=keep(ol) or keep(stores)!=keep(os) or native[0x140000:0x142000]!=ram[0x140000:0x142000] or native[0x141000:0x141100]!=ram[0x141000:0x141100] or native[0x10b0:0x10bc]!=ram[0x10b0:0x10bc] or native[0xa3490:0xa3498]!=ram[0xa3490:0xa3498] or native[0x9b300:0x9b380]!=ram[0x9b300:0x9b380] or native[0xbe998:0xbe9e8]!=ram[0xbe998:0xbe9e8]:bad.append(dict(case=i,returns=[got,expected],calls=[calls2,calls1],loads=[keep(loads),keep(ol)],stores=[keep(stores),keep(os)]))
summary=dict(requested=n,compared=done,failures=len(bad),faults=len(fault),unfaithful=drops);(s/f'combined-delay-check-{n}.json').write_text(json.dumps(dict(summary=summary,scope='full handler with original ring clear, memory copy, DMA issuer, ready wrapper and completion callback; held 7B290 scripted; finite delayed readiness and completion',coverage=coverage,callcounts=callcounts,failures=bad[:2],faults=fault[:2]),indent=2)+'\n');print(summary);sys.exit(bool(bad or fault or drops))
