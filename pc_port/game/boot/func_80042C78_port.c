/*
 * Phase 6E-B30 — func_80042C78: proven initialization prefix.
 *
 * Retail body: 16 instructions / 0x40 bytes,
 * 0x80042C78..0x80042CB4 (exclusive end), file offset 0x33478,
 * live split asm/disc1/33478.s.  The complete transcription is checked and
 * executed by tools/b30_oracle.py.
 *
 * B31 translates func_80042CC4.  This function preserves the five preceding
 * retail stores, calls it with the exact post-delay arguments, and commits
 * the final store after that leaf returns.
 */
#include "psx_compat.h"
#include "pe_guest_ram.h"

extern void func_80042CC4(int a0, int a1);

/* func_80042C78: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80042C78_port.c (src/func_80042C78.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
