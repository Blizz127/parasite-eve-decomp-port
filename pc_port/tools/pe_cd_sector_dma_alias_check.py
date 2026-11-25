from pathlib import Path
import sys,json,ctypes,random,hashlib,struct
r=Path(__file__).resolve().parents[2];sys.path.insert(0,str(r/'pc_port/tools'));import pe_cd_leaf_vectors as c
s=Path(sys.argv[1]);exe=(r/'build/disc1.candidate.exe').read_bytes();assert hashlib.sha1(exe).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b';base=bytes(c.v.fixture(exe,0,0,0));rng=random.Random(0x8007bf44);n=int(sys.argv[2]) if len(sys.argv)>2 else 6000;bad=[];fault=[];drops=done=0
L=ctypes.CFUNCTYPE(ctypes.c_uint32,ctypes.c_uint32,ctypes.c_uint32);S=ctypes.CFUNCTYPE(None,ctypes.c_uint32,ctypes.c_uint32,ctypes.c_uint32);lib=ctypes.CDLL(str(s/'draft.so'));lib.draft.argtypes=[ctypes.c_uint32,ctypes.c_uint32,L,S];lib.draft.restype=ctypes.c_uint32
orig_load=c.pl._ns['LOAD']
for i in range(n):
 ram,_,_,_=c.prep(base,1);ram[0x140000:0x140100]=rng.randbytes(256);dst=0x80140040;count=rng.getrandbits(32);ready=rng.randrange(9);busy=rng.randrange(9);noise=rng.randrange(256)&0xbf;ps=0x80140000;pc=0x8014001c
 for a,off in [(0x9b27c,0),(0x9b288,4),(0x9b2b0,8),(0x9b28c,12),(0x9b2b4,16),(0x9b2b8,20),(0x9b2bc,24),(0x9b2c0,28)]:struct.pack_into('<I',ram,a,0x80140000+off)
 struct.pack_into('<I',ram,0x9b2b8,0x8009b27c);ram[0x140040]=0x40;native=bytearray(ram);counters1=[0,0];counters2=[0,0];loads=[];stores=[]
 def change(mem,a,k,counters):
  if a==0x140000 and k==1:
   counters[0]+=1;mem[a]=noise|(0x40 if counters[0]>ready else 0)
  if a==0x14001c and k==4:
   counters[1]+=1;v=struct.unpack_from('<I',mem,a)[0];v=(v|0x1000000) if counters[1]<=busy else (v&~0x1000000);struct.pack_into('<I',mem,a,v)
 def wrapped(fmt,buf,a):change(buf,a,struct.calcsize(fmt),counters1);return orig_load(fmt,buf,a)
 c.pl._ns['LOAD']=wrapped;c.UNFAITHFUL[0]=False
 try:res=c.run_with_callees(ram,0x8007bf44,[dst,count],{}, {})
 except (AssertionError,IndexError,struct.error) as e:fault.append([i,str(e)]);continue
 finally:c.pl._ns['LOAD']=orig_load
 if c.UNFAITHFUL[0]:drops+=1;continue
 def ld(a,k):
  off=a&0x1fffff;change(native,off,k,counters2);v=int.from_bytes(native[off:off+k],'little');loads.append((a,k,v));return v
 def st(a,v,k):
  off=a&0x1fffff;native[off:off+k]=(v&((1<<(8*k))-1)).to_bytes(k,'little');stores.append((a,k,v&((1<<(8*k))-1)))
 got=lib.draft(dst,count,L(ld),S(st));done+=1;keep=lambda z:[x for x in z if (x[0]&0x1fffff)<0x1fe000]
 if got!=res[0] or loads!=keep(res[1]) or stores!=keep(res[2]) or native[0x140000:0x140100]!=ram[0x140000:0x140100] or native[0x9b200:0x9b300]!=ram[0x9b200:0x9b300]:bad.append(dict(case=i,returns=[got,res[0]],loads=[loads,keep(res[1])],stores=[stores,keep(res[2])]))
summary=dict(requested=n,compared=done,failures=len(bad),faults=len(fault),unfaithful=drops);(s/f'alias-status-check-{n}.json').write_text(json.dumps(dict(summary=summary,scope='DMA destination aliases status pointer; finite scripted device reads',failures=bad[:2],faults=fault[:2]),indent=2)+'\n');print(summary);sys.exit(bool(bad or fault or drops))
