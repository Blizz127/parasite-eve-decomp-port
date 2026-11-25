from pathlib import Path
import sys,json,ctypes,random,struct
root=Path(__file__).resolve().parents[2];sys.path.insert(0,str(root/'pc_port/tools'));import pe_cd_leaf_vectors as c
s=Path(sys.argv[1]);base=bytes(c.v.fixture((root/'build/disc1.candidate.exe').read_bytes(),0,0,0));rng=random.Random(0x80076354);n=int(sys.argv[2]) if len(sys.argv)>2 else 6000;bad=[];fault=[];drops=done=0
L=ctypes.CFUNCTYPE(ctypes.c_uint32,ctypes.c_uint32);S=ctypes.CFUNCTYPE(None,ctypes.c_uint32,ctypes.c_uint32);C=ctypes.CFUNCTYPE(ctypes.c_uint32,ctypes.c_uint32)
lib=ctypes.CDLL(str(s/'retail-draft.so'));lib.draft.argtypes=[ctypes.c_uint32,ctypes.c_uint32,L,S,C];lib.draft.restype=ctypes.c_uint32
for i in range(n):
 ram,_,_,_=c.prep(base,1);ram[0x140000:0x140080]=rng.randbytes(128)
 def sw(mem,a,v):struct.pack_into('<I',mem,a&0x1fffff,v&0xffffffff)
 def lw(mem,a):return struct.unpack_from('<I',mem,a&0x1fffff)[0]
 ptrs=[0x80140000,0x80140004,0x80140008,0x8014000c]
 for j,p in enumerate(ptrs):sw(ram,0x80095864+j*4,p)
 address=rng.getrandbits(32);count=rng.getrandbits(32);clear_after=rng.randrange(4);timeout=rng.randrange(2);new_status=0x80140010;native=bytearray(ram);calls1=[];calls2=[];loads=[];stores=[]
 def original_call(a):
  def cb(mem,regs):
   calls1.append(a)
   if a==0x800773d0:
    sw(mem,0x8009586c,new_status);sw(mem,new_status,0 if clear_after==0 else 0x01000000)
   if a==0x80077404 and calls1.count(a)>=clear_after and not timeout:sw(mem,new_status,lw(mem,new_status)&~0x01000000)
   return timeout if a==0x80077404 else 0
  return cb
 c.UNFAITHFUL[0]=False
 try:out=c.run_with_callees(ram,0x80076354,[address,count],{}, {a:original_call(a) for a in (0x800773d0,0x80077404)})
 except (AssertionError,struct.error) as e:fault.append([i,str(e)]);continue
 if c.UNFAITHFUL[0]:drops+=1;continue
 def ld(a):v=lw(native,a);loads.append((a,4,v));return v
 def st(a,v):sw(native,a,v);stores.append((a,4,v))
 def call(a):
  calls2.append(a)
  if a==0x800773d0:
   sw(native,0x8009586c,new_status);sw(native,new_status,0 if clear_after==0 else 0x01000000)
  if a==0x80077404 and calls2.count(a)>=clear_after and not timeout:sw(native,new_status,lw(native,new_status)&~0x01000000)
  return timeout if a==0x80077404 else 0
 got=lib.draft(address,count,L(ld),S(st),C(call));done+=1
 keep=lambda z:[v for v in z if (v[0]&0x1fffff)<0x1fe000]
 if got!=out[0] or loads!=keep(out[1]) or stores!=keep(out[2]) or calls1!=calls2 or (native[0x140000:0x140080]!=ram[0x140000:0x140080] or native[0x95864:0x95874]!=ram[0x95864:0x95874]):bad.append(dict(case=i,returns=[got,out[0]],loads=[loads,keep(out[1])],stores=[stores,keep(out[2])],calls=[calls2,calls1]))
summary=dict(requested=n,compared=done,failures=len(bad),faults=len(fault),unfaithful=drops);(s/f'retail-mutating-check-{n}.json').write_text(json.dumps(dict(summary=summary,failures=bad[:3],faults=fault[:3]),indent=2)+'\n');print(summary);sys.exit(bool(bad or fault or drops))
