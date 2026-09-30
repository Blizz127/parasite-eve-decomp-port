#!/usr/bin/env python3
"""Full original script76, including repeated VM execution and signed edges."""
import hashlib,itertools,random,struct,sys
from pe_battle_hud_oracle import ROOT,execute
RANGES=((0x9CE00,4),(0x9D1A0,4),(0x9D2F0,4),(0x9D300,16),
        (0x91120,4),(0x91278,4),(0x140000,0x300),
        (0x145000,0x280))
# Native VM argument pointers live at its fixed GA_VM_FRAME; original17018
# keeps them on the CPU stack. That transient host-adapted vector is not a
# shared game-state output, so compare actor/task/script state instead.

def cases():
 for heading,target,step in itertools.product((0,1,2047,2048,4095,4096,32768,65535),
                                              (0,1,2048,4095),(-32768,-1,0,1,8,2048,32767)):
  yield (heading,target,step,0,0,0,0,1)
 rng=random.Random(0x14BA0)
 for i in range(256):
  yield (rng.randrange(65536),rng.getrandbits(32),rng.getrandbits(32),
         0xA520,rng.getrandbits(32),rng.getrandbits(32),0,1)
 for heading,target,step,frames in itertools.product((0,4090),(512,10),(8,1),(1,2,63,64,65)):
  yield (heading,target,step,0,0,0,1,frames)

def fixture(exe,c):
 heading,target,step,flags,delta,saved,vm,frames=c
 r=bytearray(0x200000);r[0x10000:0x10000+len(exe)-0x800]=exe[0x800:]
 for a,n in RANGES:r[a:a+n]=bytes(n)
 def w(a,v):struct.pack_into('<I',r,a,v&0xFFFFFFFF)
 w(0x9D2F0,0x80145000);w(0x9D300,0x80140100);w(0x9CE00,0x80140210)
 w(0x145038,heading<<16);w(0x140108,flags);w(0x140110,1)
 w(0x140114,delta);w(0x140118,saved)
 w(0x140000,0x80140040);w(0x140004,0x80140044)
 w(0x140040,target);w(0x140044,step)
 w(0x91120,0x800172FC);w(0x91278,0x80014BA0)
 if vm:
  w(0x140100,0x80140200)
  for i,v in enumerate((0x4076,0,target,step,0x20,0)):w(0x140200+i*4,v)
 return r

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
  r=fixture(exe,c);initial={a+i:struct.unpack_from('<I',r,a+i)[0] for a,n in RANGES for i in range(0,n,4)}
  if common is None:common={a:v for a,v in initial.items() if v}
  first=len(patches);patches.extend((a,v) for a,v in initial.items() if v!=common.get(a,0))
  vm,frames=c[-2:]
  for frame in range(frames):
   struct.pack_into('<I',r,0x9D300,0x80140100)
   regs=execute(r,0x80017018 if vm else 0x80014BA0,() if vm else (0x80140000,))
  rows.append((vm,frames,first,len(patches),0 if vm else regs[2],fingerprint(r)))
  if '--dump' in sys.argv:(ROOT/f'local/live/script-turn-oracle-{k}.bin').write_bytes(r)
 out=['/* Generated complete original14BA0 and VM histories. */']
 for name,data in (('ranges',RANGES),('common',sorted(common.items())),('patches',patches)):
  out.append(f'static const uint32_t DAY1_turn_{name}[][2]={{')
  out.extend(f' {{0x{a:X}u,0x{v:X}u}},' for a,v in data);out.append('};')
 out.append('static const struct { unsigned vm,frames,first,end,result; uint64_t hash; } DAY1_turn_cases[]={')
 out.extend(' {'+','.join(str(v) for v in row[:-1])+f',UINT64_C(0x{row[-1]:016X})'+'},' for row in rows)
 out.append('};')
 if '--write-header' in sys.argv:(ROOT/'pc_port/tests/retail_script_turn_cases.h').write_text('\n'.join(out)+'\n')
 print('PASS:',len(rows),'original turn cases;',sum(row[1] for row in rows if row[0]),'VM frames')
if __name__=='__main__':main()
