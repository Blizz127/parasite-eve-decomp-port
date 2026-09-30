/*
 * Phase 6E-B54I — func_80077B34: SetShadeTex code-byte modifier
 * (translated retail logic, classification 1 — real outlined header inline).
 *
 * Complete retail body (10 words, verified against the SHA-1-exact retail
 * executable SLUS_006.62 / SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b;
 * file 0x68334).  Decoded from build/disc1.candidate.exe @ file 0x68334:
 *
 *   0x80077B34: 0x10A00004  beq  a1, zero, 0x80077B48
 *   0x80077B38: 0x00000000  nop
 *   0x80077B3C: 0x90820007  lbu  v0, 7(a0)
 *   0x80077B40: 0x0801DED5  j    0x80077B54
 *   0x80077B44: 0x34420001  ori  v0, v0, 0x01      ; delay slot
 *   0x80077B48: 0x90820007  lbu  v0, 7(a0)
 *   0x80077B4C: 0x00000000  nop
 *   0x80077B50: 0x304200FE  andi v0, v0, 0xFE
 *   0x80077B54: 0x03E00008  jr   ra
 *   0x80077B58: 0xA0820007  sb   v0, 7(a0)         ; delay slot
 *
 * Psy-Q libgpu origin: SetShadeTex(p, st) — set (st != 0) or clear
 * (st == 0) bit 0 of the primitive packet's code byte at offset 7.
 * Jal'd three times from func_80030894 (words 94/98/118 @ 0x80030E0C /
 * 0x80030E70 / 0x80030ED0).
 *
 * ABI: void func_80077B34(pe_addr_t p, uint32_t st).  Reads and writes
 * exactly byte p+7; every other byte untouched.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

/* func_80077B34: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80077B34_port.c (src/func_80077B34.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
