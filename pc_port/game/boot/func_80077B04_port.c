/*
 * Phase 6E-B54I — func_80077B04: SetSemiTrans code-byte modifier
 * (translated retail logic, classification 1 — real outlined header inline).
 *
 * Complete retail body (10 words, verified against the SHA-1-exact retail
 * executable SLUS_006.62 / SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b;
 * file 0x68304).  Decoded from build/disc1.candidate.exe @ file 0x68304:
 *
 *   0x80077B04: 0x10A00004  beq  a1, zero, 0x80077B18
 *   0x80077B08: 0x00000000  nop
 *   0x80077B0C: 0x90820007  lbu  v0, 7(a0)
 *   0x80077B10: 0x0801DEC9  j    0x80077B24
 *   0x80077B14: 0x34420002  ori  v0, v0, 0x02      ; delay slot
 *   0x80077B18: 0x90820007  lbu  v0, 7(a0)
 *   0x80077B1C: 0x00000000  nop
 *   0x80077B20: 0x304200FD  andi v0, v0, 0xFD
 *   0x80077B24: 0x03E00008  jr   ra
 *   0x80077B28: 0xA0820007  sb   v0, 7(a0)         ; delay slot
 *
 * Psy-Q libgpu origin: SetSemiTrans(p, abr) — set (abr != 0) or clear
 * (abr == 0) bit 1 of the primitive packet's code byte at offset 7.
 * Jal'd twice from func_80030894 (words 50/90 @ 0x80030A10/0x80030B18).
 *
 * ABI: void func_80077B04(pe_addr_t p, uint32_t abr).  Reads and writes
 * exactly byte p+7; every other byte untouched.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

/* func_80077B04: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80077B04_port.c (src/func_80077B04.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
