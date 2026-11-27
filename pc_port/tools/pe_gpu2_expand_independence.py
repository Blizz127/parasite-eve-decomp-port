#!/usr/bin/env python3
"""Evidence for the gpu2d expander vectors: (a) the original's output does not depend on its second argument (the
public prototype names it `count`), (b) the original reads only table bytes 0..0xE0C of the 0x120FC-byte range, so
the remainder of the range (0x8010DA0A..0x8011ECF8) cannot influence the expansion.  Prints hashes only."""
import hashlib
import sys

import pe_gpu2_vectors as V
import pe_gpu2_common as C

blob = V.ovl_blob()
base = C.fixture((blob, V.OVL_VRAM))
off, size, entry = C.func_range('func_8010BD4C', V.OVL_YAML, V.OVL_VRAM)
outs = {}
for cnt in (0, 1, 5, 0x100, 0xFFFFFFFF):
    ram = bytearray(base)
    stp = C.RangeStops(entry, entry + size, ())
    saved = C.R.logged
    C.R.logged = lambda a: False
    try:
        r = C.R.execute(ram, entry, (), stop_at=stp, initial_regs={4: 0x80140000, 5: cnt, 31: C.RET}, instruction_budget=V.EXP_BUDGET)
    finally:
        C.R.logged = saved
    h = hashlib.sha256(bytes(ram[0x140000:0x151000])).hexdigest()
    outs[cnt] = (r[2] & 0xFFFFFFFF, h)
    print('count=%X ret=%X output sha256=%s' % (cnt, outs[cnt][0], h))
assert len(set(outs.values())) == 1, 'output depends on the second argument'
# tail of the table: scribble over 0x8010DA0A..0x8011ECF8 and re-run
ram = bytearray(base)
lo = 0x8010DA0A & 0x1FFFFF
hi = 0x8011ECF8 & 0x1FFFFF
for i in range(lo, hi):
    ram[i] = (i * 37 + 11) & 0xFF
stp = C.RangeStops(entry, entry + size, ())
C.R.logged = lambda a: False
r = C.R.execute(ram, entry, (), stop_at=stp, initial_regs={4: 0x80140000, 5: 0, 31: C.RET}, instruction_budget=V.EXP_BUDGET)
h = hashlib.sha256(bytes(ram[0x140000:0x151000])).hexdigest()
print('tail scribbled: ret=%X output sha256=%s equal=%s' % (r[2] & 0xFFFFFFFF, h, h == outs[0][1]))
print('table range sha256 (0x8010CBFC..0x8011ECF8): %s' % hashlib.sha256(blob[0xF04:0xF04 + 0x120FC]).hexdigest())
print('read range sha256 (0x8010CBFC..0x8010DA09): %s' % hashlib.sha256(blob[0xF04:0xF04 + 0xE0E]).hexdigest())
print('overlay sha1 %s' % hashlib.sha1(blob).hexdigest())
