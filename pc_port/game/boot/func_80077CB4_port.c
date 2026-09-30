/*
 * Phase 6E-B54I — func_80077CB4: packet length-budget append
 * (translated retail logic, classification 1 — real outlined leaf).
 *
 * Complete retail body (13 words, verified against the SHA-1-exact retail
 * executable SLUS_006.62 / SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b;
 * file 0x684B4).  Decoded from build/disc1.candidate.exe @ file 0x684B4:
 *
 *   0x80077CB4: 0x90820003  lbu  v0, 3(a0)        ; head len byte
 *   0x80077CB8: 0x90A30003  lbu  v1, 3(a1)        ; tail len byte
 *   0x80077CBC: 0x00000000  nop
 *   0x80077CC0: 0x00431021  addu v0, v0, v1       ; sum
 *   0x80077CC4: 0x24430001  addiu v1, v0, 1       ; sum + 1
 *   0x80077CC8: 0x28620011  slti v0, v1, 17       ; fits budget?
 *   0x80077CCC: 0x10400004  beq  v0, zero, 0x80077CE0
 *   0x80077CD0: 0x00001021  move v0, zero         ; delay slot
 *   0x80077CD4: 0xA0830003  sb   v1, 3(a0)        ; head len = sum+1
 *   0x80077CD8: 0x0801DF39  j    0x80077CE4       ; skip li v0,-1
 *   0x80077CDC: 0xACA00000  sw   zero, 0(a1)      ; delay slot: tail.tag = 0
 *   0x80077CE0: 0x2402FFFF  addiu v0, zero, -1    ; fail: return -1
 *   0x80077CE4: 0x03E00008  jr   ra
 *   (0x80077CE8 alignment nop — outside, verified)
 *
 * Semantics: with head = a0 (compound packet being extended) and
 * tail = a1 (the primitive being appended):
 *   sum  = head[3] + tail[3];
 *   len  = sum + 1;
 *   if (len < 17) { head[3] = len; *(uint32_t *)tail = 0; return 0; }
 *   return -1;                                   ; no stores on fail
 *
 * The 17-word cap is the max GP0 packet length budget; on failure the
 * callers (func_800370DC @0x80037110 / func_80037140 @0x80037174) detect
 * the nonzero return and jump to the func_800719E4 fail path.
 *
 * ABI: int32_t func_80077CB4(pe_addr_t head, pe_addr_t tail).  Success
 * writes byte head+3 and zeroes the four bytes at tail+0; failure writes
 * nothing.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

/* func_80077CB4: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80077CB4_port.c (src/func_80077CB4.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
