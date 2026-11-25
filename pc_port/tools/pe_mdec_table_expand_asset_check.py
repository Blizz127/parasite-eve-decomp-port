from pathlib import Path
import sys,json,ctypes,random,hashlib,struct,time
r=Path(__file__).resolve().parents[2];sys.path.insert(0,str(r/'pc_port/tools'));import pe_cd_leaf_vectors as c
s=Path(sys.argv[1]);img=Path(sys.argv[2]).read_bytes();assert hashlib.sha1(img).hexdigest()=='146c0ce7308bf9fdc2ba5a84230e198db0663f3b';base=bytes(c.v.fixture((r/'build/disc1.candidate.exe').read_bytes(),0,0,0));movie=img[0x039f*0x800:0x03c5*0x800];off=0x8010bd4c-0x8010bcf8;codehash=hashlib.sha256(movie[off:off+0xf0]).hexdigest();L=ctypes.CFUNCTYPE(ctypes.c_uint32,ctypes.c_uint32,ctypes.c_uint32);S=ctypes.CFUNCTYPE(None,ctypes.c_uint32,ctypes.c_uint32,ctypes.c_uint32);lib=ctypes.CDLL(str(s/'retail-draft.so'));lib.draft.argtypes=[ctypes.c_uint32,L,S];lib.draft.restype=ctypes.c_uint32
streams=[None]
oldload=c.pl._ns['LOAD'];oldstore=c.v._store;results=[];start=time.monotonic()
for i,stream in enumerate(streams):
 ram,_,_,_=c.prep(base,1);ram[0x10bcf8:0x10bcf8+len(movie)]=movie;
 if stream is not None:ram[0x10cbfc:0x10cbfc+len(stream)]=stream
 rng=random.Random(0xbd4c+i);ram[0x13f000:0x152000]=rng.randbytes(0x13000);native=bytearray(ram);out=0x80140100
 def ld0(fmt,buf,a):return struct.unpack_from(fmt,buf,a)
 def st0(fmt,buf,a,val):struct.pack_into(fmt,buf,a,val)
 c.pl._ns['LOAD']=ld0;c.v._store=st0;stop=c.Stop((c.v.RET,))
 try:x=c.v.execute(ram,0x8010bd4c,(),stop_at=stop,initial_regs={4:out,31:c.v.RET},instruction_budget=5000000)
 finally:c.pl._ns['LOAD']=oldload;c.v._store=oldstore
 assert stop.hit==c.v.RET
 def ld(a,k):return int.from_bytes(native[a&0x1fffff:(a&0x1fffff)+k],'little')
 def st(a,v,k):native[a&0x1fffff:(a&0x1fffff)+k]=(v&((1<<(8*k))-1)).to_bytes(k,'little')
 got=lib.draft(out,L(ld),S(st));startoff=0x13f000;orig=ram[startoff:0x152000];actual=native[startoff:0x152000];results.append({'case':i,'original_sha256':hashlib.sha256(orig).hexdigest(),'native_sha256':hashlib.sha256(actual).hexdigest(),'returns':[got,x[2]&0xffffffff],'equal':orig==actual and got==(x[2]&0xffffffff)})
print('cases',len(results),'pass',sum(x['equal'] for x in results),'seconds',round(time.monotonic()-start,2));(s/'asset-check.json').write_text(json.dumps({'original_span_sha256':codehash,'cases':results,'original_asset_case':'RUN_HASH_ONLY'},indent=2)+'\n');sys.exit(not all(x['equal'] for x in results))
