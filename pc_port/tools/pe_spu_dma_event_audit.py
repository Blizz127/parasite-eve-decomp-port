#!/usr/bin/env python3
"""Original DMA IRQ event/callback dispatch contract; BIOS provider is a boundary."""
import hashlib,struct
from pe_battle_hud_oracle import ROOT,execute
ex=(ROOT/'build/disc1.candidate.exe').read_bytes()
assert hashlib.sha1(ex).hexdigest()=='452fb033f2eaa4b18aa20a5bca60b8125af3a37b'
for callback in (0,0x80085098):
 for control in (0,0x80,0x30,0xffff):
  r=bytearray(0x200000);r[0x10000:0x10000+len(ex)-2048]=ex[2048:]
  struct.pack_into('<I',r,0x9b3fc,0x80150000)
  struct.pack_into('<I',r,0x9b44c,1) # completion bookkeeping already performed
  struct.pack_into('<I',r,0x9b434,callback)
  struct.pack_into('<H',r,0x1501aa,control)
  regs=execute(r,0x8007D614,stop_at=(0x80073A34,0x80085098))
  assert struct.unpack_from('<H',r,0x1501aa)[0]==control&0xffcf
  if not callback:assert regs[4]==0xf0000009 and regs[5]==0x20 and regs[31]==0x8007d6c0
  else:assert regs[31]==0x8007d6ac
print('PASS 8 original DMA IRQ dispatch paths; event delivery/consumption is an explicit host contract')
