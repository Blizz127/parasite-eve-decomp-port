#!/usr/bin/env python3
"""Original ED3100 ->6914C state handling; CD issue/poll states excluded."""
import hashlib,struct,sys
from pe_battle_hud_oracle import ROOT,execute
RANGES=((0x9CE00,4),(0x9D1A0,4),(0x9D300,4),(0xB0CD8,0x200),
        (0x150000,0x80),(0x154000,0x80),(0x170000,0x1800))
def fingerprint(r):
 h=14695981039346656037
 for a,n in RANGES:
  for b in r[a:a+n]:h=((h^b)*1099511628211)&0xFFFFFFFFFFFFFFFF
 return h

def main():
 ex=(ROOT/'build/disc1.candidate.exe').read_bytes()
 assert hashlib.sha1(ex).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
 common=None;patches=[];cases=[];seen=set()
 for state in range(256):
  if state in (0x34,0x35):continue # CD issue/poll require separate BIOS/host evidence.
  for variant in range(4):
   r=bytearray(0x200000);r[0x10000:0x10000+len(ex)-2048]=ex[2048:]
   for a,n in RANGES:r[a:a+n]=bytes(n)
   def sw(a,v):struct.pack_into('<I',r,a&0x1FFFFF,v&0xFFFFFFFF)
   def lw(a):return struct.unpack_from('<I',r,a&0x1FFFFF)[0]
   flags=(0,8,0xFFFFFFFF,0xFFFFFFF7)[variant]
   sw(0xB0CD8,flags);r[0xB0DC7]=state
   sw(0x9D1A0,(0x80,0x82,0xFFFFFFFF,0x80000080)[variant]) # state0 overlay already initialized
   sw(0x9CE00,0x801C5604);sw(0x9D300,0x80150000);sw(0x150010,(0,1,0xFFFFFFFF,17)[variant])
   sw(0xB0E6C,0x80170000);sw(0x170004,0x40)
   sw(0xB0E64,0x80171000);sw(0x171004,0x40) # empty texture/archive directories for state36
   for i in range(8):sw(0x154000+i*4,0x80154040+i*4)
   sw(0x154040,3100)
   seed={a+i:lw(a+i) for a,n in RANGES for i in range(0,n,4)}
   if common is None:common={a:v for a,v in seed.items() if v}
   first=len(patches);patches.extend((a,v) for a,v in seed.items() if v!=common.get(a,0))
   regs=execute(r,0x80016910,(0x80154000,),visited_pcs=seen,instruction_budget=100000)
   busy=state==0 and bool(flags&8)
   assert regs[2]==(0 if busy else 1)
   assert lw(0x9CE00)==(0x801C55DC if busy else 0x801C5604)
   assert r[0xB0DC7]==(0x34 if busy else 0 if state==0x36 else state)
   if busy:assert lw(0x150010)==1
   else:assert lw(0x150010)==(0,1,0xFFFFFFFF,17)[variant]
   cases.append((first,len(patches),regs[2],fingerprint(r)))
 assert {0x8006914C,0x80016DAC,0x80069468}<=seen
 lines=['/* Original ED3100/6914C; initialized overlay, no CD issue/poll. */']
 for name,rows in (('ranges',RANGES),('common',sorted(common.items())),('patches',patches)):
  lines.append(f'static const uint32_t SRD_{name}[][2]={{')
  lines.extend(f'{{0x{a:X}u,0x{b:X}u}},' for a,b in rows);lines.append('};')
 lines.append('static const struct { unsigned first,end,result; uint64_t hash; } SRD_cases[]={')
 lines.extend(f'{{{a},{b},{v},UINT64_C(0x{h:016X})}},' for a,b,v,h in cases);lines.append('};')
 header='\n'.join(lines)+'\n';target=ROOT/'pc_port/tests/retail_script_readiness_cases.h'
 if '--check' in sys.argv:assert target.read_text()==header
 else:target.write_text(header)
 print(f'PASS {len(cases)} original script readiness graphs')
if __name__=='__main__':main()
