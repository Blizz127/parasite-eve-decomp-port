from pathlib import Path
import sys,struct,json,ctypes,random
root=Path(__file__).resolve().parents[2];sys.path.insert(0,str(root/'pc_port/tools'));import pe_cd_leaf_vectors as c
here=Path(sys.argv[1]);base=bytes(c.v.fixture((root/'build/disc1.candidate.exe').read_bytes(),0,0,0));entry=0x80081414
CALL=ctypes.CFUNCTYPE(ctypes.c_uint32,ctypes.c_uint32,ctypes.c_uint32,ctypes.c_uint32);STORE=ctypes.CFUNCTYPE(None,ctypes.c_uint32,ctypes.c_uint32,ctypes.c_uint32)
lib=ctypes.CDLL(str(here/'retail-draft.so'));lib.draft.argtypes=[ctypes.POINTER(ctypes.c_uint8),CALL,STORE,ctypes.c_uint32,ctypes.c_uint32];lib.draft.restype=ctypes.c_uint32
rng=random.Random(81414);edges=[0,1,2,3,0x7fffffff,0x80000000,0xffffffff]
paths=['','A','\\','\\A','\\A\\B','\\A\\B\\C','\\\\A','\\A\\','\\A\\\\','\\'+'A'*33]
callees=(0x80071a74,0x80073c5c,0x8007f7a8,0x800816f4,0x80081714,0x800819d8,0x80081a7c)
def word(r,a,v):struct.pack_into('<I',r,a&0x1fffff,v)
def string(r,a):
 out=[]
 for k in range(96):
  v=r[(a+k)&0x1fffff]
  if not v:break
  out.append(v)
 return bytes(out).hex()
def canonical(r,fn,a,b):
 if fn in (0x8007f7a8,0x80081714):return [fn]
 if fn==0x800819d8:return [fn,a,string(r,b)]
 if fn==0x800816f4:return [fn,a,string(r,a),string(r,b)]
 if fn==0x80071a74:return [fn,a,string(r,b)]
 if fn==0x80073c5c:return [fn,a]
 return [fn,a,b]
fail=[];fault=[];drops=0;done=0;n=int(sys.argv[2]) if len(sys.argv)>2 else 6000
for i in range(n):
 ram,_,_,_=c.prep(base,7919*(i+1));spec={k:rng.choice(edges) for k in ['cache','disc','disc2','init','dir','load','match','debug']};spec.update(path=paths[i%len(paths)],records=i%5,found=(i//5)%6,out=[0x80151000,0x800A36B0,0x800A36B4,0x800A36AC,0x80150000][i%5])
 ram[0x150000:0x150100]=bytes(256);path=spec['path'].encode()+b'\0';ram[0x150000:0x150000+len(path)]=path;ram[0xA36B0:0xA3CB0]=bytes(0x600)
 for k in range(spec['records']):
  a=0xA36B0+24*k;ram[a:a+24]=struct.pack('<II16s',0x01020304+k,0x11223344+k,f'FILE{k};1'.encode())
 word(ram,0x8009B6E0,spec['cache']);word(ram,0x8009AFC0,spec['debug']);native=bytearray(ram)
 def models(mem,events,stores):
  seen={}
  def call(fn,a,b):
   events.append([canonical(mem,fn,a,b),len(stores())]);seen[fn]=seen.get(fn,0)+1
   if fn==0x8007f7a8:return spec['disc' if seen[fn]==1 else 'disc2']
   if fn==0x800816f4:return spec['match'] if seen[fn]==spec['found'] else 0
   return spec.get({0x80081714:'init',0x800819d8:'dir',0x80081a7c:'load'}.get(fn,''),0)
  return call
 original_events=[];native_events=[];native_stores=[]
 orig=models(ram,original_events,lambda:c.pl.STORES);c.UNFAITHFUL[0]=False
 stops={fn:(lambda mem,regs,fn=fn:orig(fn,regs[4]&0xffffffff,regs[5]&0xffffffff)) for fn in callees}
 try:res=c.run_with_callees(ram,entry,[spec['out'],0x80150000],{},stops)
 except AssertionError as e:fault.append([i,spec,str(e)]);continue
 if c.UNFAITHFUL[0]:drops+=1;continue
 callback=CALL(models(native,native_events,lambda:native_stores));store=STORE(lambda a,s,v:native_stores.append((a,s,v)))
 ret=lib.draft((ctypes.c_uint8*len(native)).from_buffer(native),callback,store,spec['out'],0x80150000)
 expected=[res[0],res[2],original_events];actual=[ret,native_stores,native_events];done+=1
 if expected!=actual:fail.append(dict(case=i,spec=spec,expected=expected,actual=actual))
summary=dict(requested=n,compared=done,failures=len(fail),faults=len(fault),unfaithful=drops)
(here/f'retail-check-{n}.json').write_text(json.dumps(dict(summary=summary,failures=fail,faults=fault),indent=2)+'\n');print(summary);sys.exit(bool(fail or fault or drops))
