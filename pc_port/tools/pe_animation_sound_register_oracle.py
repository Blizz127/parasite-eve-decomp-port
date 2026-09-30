#!/usr/bin/env python3
"""Original EA406/407 event registration followed by sound projection/FIFO."""
import hashlib,struct,sys
from pe_script_sound_oracle import ROOT,execute,fixture as sound_fixture,RANGES as SOUND_RANGES
RANGES=SOUND_RANGES+((0x94488,0x840),(0x9D1A0,4))
def fingerprint(r):
 h=14695981039346656037
 for a,n in RANGES:
  for b in r[a:a+n]:h=((h^b)*1099511628211)&0xFFFFFFFFFFFFFFFF
 return h

def main():
 ex=(ROOT/'build/disc1.candidate.exe').read_bytes()
 assert hashlib.sha1(ex).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
 common=None;patches=[];cases=[]
 for key in (406,407):
  for count in (0,1,15,16,255):
   for variant in range(8):
    r=sound_fixture(ex,dict(key=300,sound=1,x=-300,queue=0))
    def sw(a,v):struct.pack_into('<I',r,a&0x1FFFFF,v&0xFFFFFFFF)
    def sh(a,v):struct.pack_into('<H',r,a&0x1FFFFF,v&65535)
    r[0x94488:0x94CC8]=bytes([0xA5])*0x840
    sw(0x9D1A0,0);r[0xB0CE9]=count;r[0xB0CEA]=variant&1
    r[0x15000E]=5;r[0x15000F]=8;sw(0x150014,3<<16);sw(0x150018,0);sw(0x15001C,65536)
    vals=[key,5|(0x123400 if variant&2 else 0),1|(0x345600 if variant&2 else 0),0 if variant&4 else 0x123407E2,0x432107E3,0xDEADBEEF]
    for i,v in enumerate(vals):sw(0x140240+i*4,v)
    seed={a+i:struct.unpack_from('<I',r,a+i)[0] for a,n in RANGES for i in range(0,n,4)}
    if common is None:common={a:v for a,v in seed.items() if v}
    first=len(patches);patches.extend((a,v) for a,v in seed.items() if v!=common.get(a,0))
    regs=execute(r,0x80015DAC,(0x80140200,));assert regs[2]==1
    assert r[0xB0CE9]==(count+1 if count<16 else count)
    if count<16:
     assert bytes(r[0x944A8+count*8:0x944A8+count*8+8])==struct.pack('<4B2H',2,7,5,1,vals[3]&65535,vals[3 if key==406 else 4]&65535)
    else:assert r[0x94488:0x94CC8]==bytes([0xA5])*0x840
    registered=fingerprint(r)
    execute(r,0x8006A318,(0x80150000,),instruction_budget=900000)
    cases.append((first,len(patches),registered,fingerprint(r)))
 lines=['/* Original EA406/407 registration and sound FIFO, synthetic sound bank. */']
 for name,rows in (('ranges',RANGES),('common',sorted(common.items())),('patches',patches)):
  lines.append(f'static const uint32_t ASR_{name}[][2]={{')
  lines.extend(f'{{0x{a:X}u,0x{b:X}u}},' for a,b in rows);lines.append('};')
 lines.append('static const struct { unsigned first,end; uint64_t registered,played; } ASR_cases[]={')
 lines.extend(f'{{{a},{b},UINT64_C(0x{h:016X}),UINT64_C(0x{p:016X})}},' for a,b,h,p in cases);lines.append('};')
 header='\n'.join(lines)+'\n';path=ROOT/'pc_port/tests/retail_animation_sound_register_cases.h'
 if '--check' in sys.argv:assert path.read_text()==header
 else:path.write_text(header)
 print(f'PASS {len(cases)} original animation sound registration/playback graphs')
if __name__=='__main__':main()
