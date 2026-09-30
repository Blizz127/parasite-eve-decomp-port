/*
 * PE-BTL6 — func_8003DFD8 live copy leaf (51 words, zero callees).
 *
 * Native translation, not matching src/ C. Authority is
 * pc_port/tools/pe_btl6_3d050_remainder_oracle.py against EXE SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 *
 * Exclusive 0x8003DFD8..0x8003E0A4 (first jr). The 73-word find_fn_end
 * window walks into two later leaves. Live 3D834 a1==0 site is
 * jal 3DFD8(0x800B1638, dest+0x34, 1). Copies count 32-byte records:
 * halfwords +0..+16, words +0x14/+0x18/+0x1C; +0x12 is not touched.
 * blez count returns. Does not andi 0xFC.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

/* func_8003DFD8: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8003DFD8_port.c (src/func_8003DFD8.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
