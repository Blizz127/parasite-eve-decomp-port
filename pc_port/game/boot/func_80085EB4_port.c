/*
 * Phase 6E-B47 — func_80085EB4: SPU-memory address validation
 * (23 retail words / 0x5C bytes, exe 0x80085EB4–0x80085F0F,
 * file offset 0x766B4, live split asm/disc1/75F44.s:631–657;
 * all 23 instruction words verified exact against the SHA-exact
 * retail executable).
 *
 * Retail signature:
 *   pe_addr_t func_80085EB4(pe_addr_t spu_addr)
 *
 * Single call site in the executed path:
 *   func_800851A8 @0x800851D4 (jal, addiu $s0,$s0,4 delay slot):
 *   a0 = *(buffer + 0x10) — first word of the transfer-parameter block.
 *
 * ROM-order operation map:
 *   Prologue: save $s0/$ra on 0x18-frame.
 *   $s0 = a1 = a0 (save argument).
 *
 *   1. Range check: $v1 = a0 - 0x1010; $v0 = 0x7EFE8.
 *      sltu $v0, $v0, $v1 → if (0x7EFE8 < (a0 - 0x1010)) return 0.
 *      Valid range: [0x1010, 0x1010 + 0x7EFE8] = [0x1010, 0x7FFF8].
 *
 *   2. If in range: call func_8007DB24(-1) with $a1 = saved argument.
 *      func_8007DB24(-1, addr):
 *        - If D_8009B420 != 0: align addr to D_8009B428 boundary
 *          using ~D_8009B42C mask.
 *        - $a3 = addr >> D_8009B424
 *        - a0 == -1: return $a3 & 0xFFFF (truncated to halfword).
 *      Stores return to D_8009B414 (sh — halfword store).
 *
 *   3. Load D_8009B414 (lhu), load D_8009B424.
 *      Return: D_8009B414 << D_8009B424.
 *
 *   4. Out-of-range path: return 0.
 *
 *   Epilogue: restore $s0/$ra, jr $ra (nop delay slot).
 *
 * Classification: 3 — deterministic platform provider.  This is
 * pure bookkeeping over SPU-memory address metadata stored in guest
 * RAM (D_8009B414, D_8009B420, D_8009B424, D_8009B428, D_8009B42C).
 * No SPU MMIO is touched.  The allocator is a simple address validator
 * and format converter, not a bump allocator.
 *
 * SPU globals initialized by SsInit (collapsed in port, values set
 * in func_80085644):
 *   D_8009B420 = 2  (alignment mode flag)
 *   D_8009B424 = 3  (SPU address shift: 8-byte units)
 *   D_8009B428 = 8  (alignment divisor)
 *   D_8009B42C = 7  (alignment mask)
 *   D_8009B414 = 0x1010 (heap top, halfword)
 *
 * Dependency boundary:
 *   func_8007DB24  TRANSLATED (inlined: alignment + shift + truncation)
 */
#include "psx_compat.h"

/* ── Guest globals ──────────────────────────────────────────────────── */
#define GA_D_8009B414  0x8009B414u   /* SPU heap top (halfword)          */
#define GA_D_8009B420  0x8009B420u   /* alignment mode flag              */
#define GA_D_8009B424  0x8009B424u   /* SPU address shift amount         */
#define GA_D_8009B428  0x8009B428u   /* alignment divisor                */
#define GA_D_8009B42C  0x8009B42Cu   /* alignment mask                   */

/* SPU RAM valid range constants (from retail asm). */
#define PE_SPU_BASE       0x1010u
#define PE_SPU_MAX_OFFSET 0x7EFE8u

/* ── func_80085EB4: SPU-memory address validation ───────────────────── */
/* func_80085EB4: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80085EB4_port.c (src/func_80085EB4.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md, round 6). */
