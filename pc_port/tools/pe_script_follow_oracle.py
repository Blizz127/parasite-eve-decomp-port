#!/usr/bin/env python3
"""Original script36 target tracking, approach, identity changes and VM retries."""
import hashlib,itertools,random,struct,sys
from pe_battle_hud_oracle import ROOT,execute
RANGES=((0x9CE00,4),(0x9D1A0,4),(0x9D20C,4),(0x9D254,4),(0x9D2F0,4),
        (0x9D300,16),(0x91120,4),(0x91178,4),(0x140000,0x300),
        (0x145000,0xC00))
INPUTS=RANGES+((0x9589C,0x804),(0x966EC,0x4000),(0x9A6EC,0x804))

def cases():
 for heading,rate,target in itertools.product((0,1,2047,2048,4095,4096,32768,65535),
       (0,1,8,2048,4096,0xFFFFFFFF,0x80000000,0x7FFFFFFF),
       ((100<<16,0),(0,100<<16),(-100<<16,100<<16),(1,1))):
  yield dict(heading=heading,rate=rate,target=target)
 for lookup in ('camera','list','missing','hidden','next','empty','wide_type','wide_id'):
  for cached in (False,True):
   for invalid in ('none','hidden','identity','wide_identity'):
    yield dict(lookup=lookup,cached=cached,invalid=invalid)
 for speed,scale,camera,position in itertools.product((0,0x10000,0x80000000,0xFFFFFFFF),
       (0,1,4096,65535),(False,True),((0,0),(0x7FFFFFFF,0x80000000))):
  yield dict(speed=speed,scale=scale,camera=camera,target=position)
 rng=random.Random(0x13E84)
 for i in range(128):
  yield dict(heading=rng.randrange(65536),rate=rng.getrandbits(32),
             speed=rng.getrandbits(32),scale=rng.randrange(65536),
             source=(rng.getrandbits(32),rng.getrandbits(32)),
             target=(rng.getrandbits(32),rng.getrandbits(32)))
 for lookup,motion,frames in itertools.product(('camera','list'),('still','move','hide','replace'),(1,8,32)):
  yield dict(vm=True,lookup=lookup,motion=motion,frames=frames,rate=64)

def fixture(exe,c):
 r=bytearray(0x200000);r[0x10000:0x10000+len(exe)-0x800]=exe[0x800:]
 for a,n in RANGES:r[a:a+n]=bytes(n)
 def w(a,v):struct.pack_into('<I',r,a,v&0xFFFFFFFF)
 def h(a,v):struct.pack_into('<H',r,a,v&65535)
 actor,target,other=0x145000,0x145400,0x145800
 w(0x9D2F0,0x80000000+actor);w(0x9D300,0x80140100);w(0x9CE00,0x80140214)
 w(0x140110,1);w(0x140108,0xA520 if c.get('cached') else 0xA500)
 lookup=c.get('lookup','list' if c.get('camera') else 'camera');kind=0 if lookup=='camera' else 2;identity=7
 if lookup=='wide_type':kind=258
 if lookup=='wide_id':identity=263
 w(0x9D254,0 if lookup=='empty' else 0x80000000+(actor if c.get('camera') else target))
 w(0x9D20C,0 if lookup=='empty' else 0x80000000+target)
 for p in (actor,target,other):
  h(p+12,0x702);h(p+0x24,0x1234);h(p+0x26,c.get('scale',4096))
 w(actor+0x20,c.get('speed',0x20000));h(actor+0x3A,c.get('heading',0))
 for p,position in ((actor,c.get('source',(0,0))),(target,c.get('target',(100<<16,100<<16))),(other,(50<<16,-50<<16))):
  for off,value in zip((0x28,0x30),position):w(p+off,value)
 if lookup=='missing':h(target+12,0x803)
 if lookup in ('hidden','next'):w(target+0x98,0x10)
 if lookup=='next':w(target+4,0x80000000+other)
 w(0x140118,0x80000000+target);w(0x14011C,0x1234);w(0x140114,c.get('rate',8))
 invalid=c.get('invalid','none')
 if invalid=='hidden':w(target+0x98,0x10)
 if invalid=='identity':h(target+0x24,0x1235)
 if invalid=='wide_identity':w(0x14011C,0x10001234)
 for i,v in enumerate((kind,identity,c.get('rate',8))):w(0x140000+i*4,0x80140040+i*4);w(0x140040+i*4,v)
 w(0x91120,0x800172FC);w(0x91178,0x80013E84)
 if c.get('vm'):
  w(0x140100,0x80140200)
  for i,v in enumerate((0x6036,0,kind,identity,c.get('rate',8),0x20,0)):w(0x140200+i*4,v)
 return r

def frame_input(r,motion,frame):
 def w(a,v):struct.pack_into('<I',r,a,v&0xFFFFFFFF)
 w(0x9D300,0x80140100)
 if motion:
  # Explicit frame inputs: integrate prior output velocity and optionally
  # move/remove/reuse the target. Full actor movement is tested separately.
  for off in (0x28,0x30):w(0x145000+off,struct.unpack_from('<I',r,0x145000+off)[0]+struct.unpack_from('<I',r,0x145040+off)[0])
  if motion==2:w(0x145428,(100+frame*3)<<16);w(0x145430,(100-frame*2)<<16)
  if frame==3 and motion==3:w(0x145498,0x10)
  if frame==3 and motion==4:struct.pack_into('<H',r,0x145424,0x1235)

def fingerprint(r):
 h=14695981039346656037
 for a,n in RANGES:
  for b in r[a:a+n]:h=((h^b)*1099511628211)&0xFFFFFFFFFFFFFFFF
 return h

def main():
 exe=(ROOT/'build/disc1.candidate.exe').read_bytes()
 assert hashlib.sha1(exe).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
 common=None;patches=[];rows=[]
 for k,c in enumerate(cases()):
  r=fixture(exe,c);initial={a+i:struct.unpack_from('<I',r,a+i)[0] for a,n in INPUTS for i in range(0,n,4)}
  if common is None:common={a:v for a,v in initial.items() if v}
  first=len(patches);patches.extend((a,v) for a,v in initial.items() if v!=common.get(a,0))
  vm=c.get('vm',False);motion=('none','still','move','hide','replace').index(c.get('motion','none'))
  for frame in range(c.get('frames',1)):
   frame_input(r,motion,frame)
   regs=execute(r,0x80017018 if vm else 0x80013E84,() if vm else (0x80140000,))
   rows.append((k,vm,frame,motion,first,len(patches),0 if vm else regs[2],fingerprint(r)))
  if '--dump' in sys.argv:(ROOT/f'local/live/script-follow-oracle-{k}.bin').write_bytes(r)
  print(k,c,hex(rows[-1][-1]),flush=True)
 out=['/* Generated complete original13E84 / VM36 histories. */']
 for name,data in (('ranges',RANGES),('common',sorted(common.items())),('patches',patches)):
  out.append(f'static const uint32_t DAY1_follow_{name}[][2]={{')
  out.extend(f' {{0x{a:X}u,0x{v:X}u}},' for a,v in data);out.append('};')
 out.append('static const struct { unsigned history,vm,frame,motion,first,end,result; uint64_t hash; } DAY1_follow_cases[]={')
 out.extend(' {'+','.join(str(int(v)) for v in row[:-1])+f',UINT64_C(0x{row[-1]:016X})'+'},' for row in rows)
 out.append('};')
 if '--write-header' in sys.argv:(ROOT/'pc_port/tests/retail_script_follow_cases.h').write_text('\n'.join(out)+'\n')
 print('PASS:',k+1,'histories;',len(rows),'original actor-follow steps')
if __name__=='__main__':main()
