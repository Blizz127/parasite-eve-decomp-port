#!/usr/bin/env python3
"""Original equipment initialization with zero/nonzero relative table offsets."""
import hashlib,struct,sys
from pe_equipment_oracle import ROOT,fixture,execute,fingerprint

def main():
 ex=(ROOT/'build/disc1.candidate.exe').read_bytes();assert hashlib.sha1(ex).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
 out=['/* Original2F76C graph, including zero relative offsets. */','static const uint64_t equipment_offsets_expected[]={']
 for n in range(132):
  r=fixture(ex,n%33);mask=n//33
  if not mask&1:struct.pack_into('<I',r,0xA8038,0)
  if not mask&2:struct.pack_into('<I',r,0xA803C,0)
  execute(r,0x8002F76C,(0x80140000,))
  out.append(f'UINT64_C(0x{fingerprint(r):016X}),')
 out.append('};');header='\n'.join(out)+'\n';p=ROOT/'pc_port/tests/retail_equipment_offsets.h'
 if '--write-header' in sys.argv:p.write_text(header)
 else:assert p.read_text()==header
 print('PASS132 original equipment graphs with zero/nonzero relative offsets')
if __name__=='__main__':main()
