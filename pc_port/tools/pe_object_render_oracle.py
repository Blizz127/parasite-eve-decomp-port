#!/usr/bin/env python3
"""Original object wrappers with complete real visibility/matrix/emitter graphs."""
import hashlib,struct,sys
from pe_subdiv_model_oracle import fixture as subdiv_fixture,RANGES,load_overlay,execute,ROOT,fnv
from pe_depth_model_oracle import fixture as model_fixture
from pe_transition_visibility_oracle import FIELDS
EXTRA=((0x150000,0x140),(0x19BFF0,4),(0x19CC30,32),(0x18F014,32),(0x19CA90,0x190),(0x1EA5E4,4))
def main():
 ex,o,b=load_overlay();base=bytearray(0x200000);base[0x10000:0x10000+len(ex)-0x800]=ex[0x800:];base[b&0x1fffff:(b&0x1fffff)+len(o)]=o
 digest=hashlib.sha256(o[0x80190D3C-b:0x80191580-b]).hexdigest()
 assert digest=='d430a28dfa21a41f473027f8bc3c6247511b48a0440d938af53abe89cef742d7'
 print('source SHA256',digest)
 cases=[]
 for kind in range(3):
  for n in range(192 if kind==2 else 128):cases.append((kind,n,0))
 for kind in range(3):
  for trap in (1,2):
   if kind or trap==1:cases.append((kind,1,trap))
 out=['/* Original object wrapper comparison fixtures. */','static const struct { uint32_t seed,mask,mode,bias,shape,depth,kind,args[3],patch_count,patches[256][2],trap,state[28]; uint64_t hash; } DAY1_object_render_cases[]={']
 visited=set();traps=0
 for kind,n,trap in cases:
  seed=53+n*101+kind*65537;mask=(n*37)&255
  r,c=subdiv_fixture(base,seed,mask,0,0,1,1024) if kind==0 else model_fixture(base,seed,mask,0,0,1)
  patches={}
  def sw(a,v):patches[a]=v&0xffffffff;struct.pack_into('<I',r,a,v&0xffffffff)
  for a,length in EXTRA:
   for i in range(0,length,4):sw(a+i,int.from_bytes(bytes(((i+j)*13+19)&255 for j in range(4)),'little'))
  obj=0x150000;mo=12 if kind==2 else 8
  for i in range(16):sw(obj+i*4,0)
  sw(obj+(8 if kind==2 else 4),0x80140040 if kind==0 else 0x80140000)
  if kind==2:sw(obj,0);sw(obj+4,2<<16|1);sw(obj+0x38,2000);sw(obj+0x3C,1000)
  for i in range(3):sw(0x140000+i*4,0x40+i*0x30)
  for h,hm in ((1,0x55),(2,0xAA)):
   for j in range(4):sw(0x140040+h*0x30+j*4,(int(bool(mask&hm&(1<<(j*2)))))|(int(bool(mask&hm&(1<<(j*2+1)))))<<16)
   for j in range(8):sw(0x140050+h*0x30+j*4,0xC0+j*0x200-h*0x30)
  angles=obj+(0x2C if kind==2 else 0x28)
  sw(angles,0 if n%4==0 else ((n*17)&65535)|(((n*31)&65535)<<16));sw(angles+4,0)
  sw(obj+mo+20,(n%7)-3);sw(obj+mo+24,(n%9)-4)
  z=(384,385,999,1000,1999,2000,5000,9000)[n%8];sw(obj+mo+28,z)
  sw(obj+(0x34 if kind==2 else 0x30),10<<16)
  dest=(0x80150100,0x80150000+mo,0x80150000+mo+4,0x8019CC30)[n//8%4];sw(0x19BFF0,dest)
  for a in (0x19CC30,0x18F014):
   for j,v in enumerate((4096,0,4096,0,4096,0,0,0)):sw(a+j*4,v)
  if n&1:sw(0x18F014,0x10000000);sw(0x18F01C,0xF000);sw(0x18F028,4)
  for j,a in enumerate(FIELDS):sw(a,0 if j<12 else (0 if n%4<2 else -10000) if j<16 else 1)
  if trap==2:
   sw(FIELDS[16],0);sw(FIELDS[20],0)
  if trap==1:sw(0x19CC4C,0x7FFFFFFF);sw(0x19BFF0,0x80150100)
  sw(0x1EA5E0,0);sw(0x1EA5E4,0)
  args=(n%3,(n//3)%2,(n//6)%3) if kind==1 else ((n//3)%2,n%4,0)
  argv=(0x80150000,0x80149999)+ (args if kind==1 else args[:2] if kind==2 else ())
  state={};failed=0
  try:execute(r,(0x80190D3C,0x80190E04,0x80191114)[kind],argv,initial_cop_control=c,strict_gte_flags=True,final_gte=state,visited_pcs=visited,instruction_budget=500000)
  except AssertionError as exc:
   msg=exc.args[0];assert trap and (msg=='retail signed arithmetic overflow' or isinstance(msg,tuple) and msg[0]=='division by zero in original fixture'),exc
   failed=1;traps+=1
  if failed:vs=[0]*28
  else:
   d=state['data'];cc=state['control'];vs=d[7:15]+d[16:20]+d[24:28]+list(struct.unpack('<10h',struct.pack('<5I',*cc[:5])))[:9]+cc[5:8]
  h=fnv(b''.join(r[a:a+length] for a,length in RANGES+EXTRA))
  assert len(patches)<=256,len(patches)
  arr=lambda values:'{'+','.join(f'0x{x&0xffffffff:X}u' for x in values)+'}'
  out.append(f' {{{seed},{mask},0,0,1,1024,{kind},'+arr(args)+f',{len(patches)},'+'{'+','.join(arr(p) for p in patches.items())+'}'+f',{failed},'+arr(vs)+f',UINT64_C(0x{h:016X})'+'},')
 out.append('};')
 if '--write-header' in sys.argv:(ROOT/'pc_port/tests/retail_object_render_cases.h').write_text('\n'.join(out)+'\n')
 print(f'PASS: {len(cases)} original wrapper cases; {traps} arithmetic-stop prefixes; reached emitters '+','.join(hex(pc) for pc in (0x80197BA0,0x801995BC,0x8019A318,0x8019B1D0) if pc in visited))
if __name__=='__main__':main()
