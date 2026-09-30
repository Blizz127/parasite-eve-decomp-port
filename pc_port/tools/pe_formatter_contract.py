#!/usr/bin/env python3
"""Source-pinned original formatter behavior; this is not native acceptance."""
import hashlib,json,struct
from pe_battle_hud_oracle import ROOT,execute
FORMATS=('%d','%i','%u','%o','%x','%X','%p','%hd','%hu','%ld','%Ld','%+08d','%#08x','%.0d','%#.0o','%-12d','% 6d','%*d','%.*u','A%qB','%%:%c','%#s','%#.2s','A%nB','A%hnB','bu%d0:BASLUS-00662000000%c%c','bu%ld0:')
VALUES=(0,1,9,10,15,16,255,256,32767,32768,65535,65536,0x7FFFFFFF,0x80000000,0xFFFFFFFE,0xFFFFFFFF)
def main():
 ex=(ROOT/'build/disc1.candidate.exe').read_bytes();assert hashlib.sha1(ex).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
 raw=ex[0x71A84-0xF800:0x72308-0xF800];digest=hashlib.sha256(raw).hexdigest();assert digest=='3eaf80663eeaeaa09560a87c5468b74afad3fa6ea1177d7a81b6f15eae4331e4'
 table={chr(0x4C+i):hex(struct.unpack_from('<I',ex,0x11644-0xF800+4*i)[0]) for i in range(45)}
 seen=set();cases=[]
 for fmt in FORMATS:
  for value in VALUES:
   r=bytearray(0x200000);r[0x10000:0x10000+len(ex)-0x800]=ex[0x800:]
   encoded=fmt.encode()+b'\0';r[0x140000:0x140000+len(encoded)]=encoded
   r[0x142000:0x142008]=b'\x03abcXYZ\0'
   args=[value,0x30,0x41]
   if fmt in ('%*d','%.*u'):args=[(-8 if value&1 else 8)&0xFFFFFFFF,value,0]
   if fmt in ('%#s','%#.2s','A%nB','A%hnB'):args=[0x80142000,0,0]
   struct.pack_into('<I',r,0x1FF010,args[2])
   regs=execute(r,0x80071A84,(0x80141000,0x80140000,args[0],args[1]),visited_pcs=seen)
   count=regs[2];assert count<512 and r[0x141000+count]==0
   cases.append(dict(format=fmt,value=hex(value),args=[hex(v) for v in args],count=count,output_hex=r[0x141000:0x141000+count].hex(),side_effect_hex=r[0x142000:0x142008].hex()))
 assert {0x80071D8C,0x80071DE4,0x80071F04,0x80072004,0x80072018,0x80072024,0x8007212C,0x8007214C,0x800721D8,0x8007220C,0x80072334}<=seen
 out=dict(sha256=digest,dispatch=table,cases=cases)
 path=ROOT/'local/live/formatter-contract.json';path.write_text(json.dumps(out,indent=2)+'\n')
 print(f'PASS{len(cases)} original formatter executions; artifact {path.relative_to(ROOT)}')
 print('dispatch',json.dumps({k:v for k,v in table.items() if v!='0x8007220c'},sort_keys=True))
if __name__=='__main__':main()
