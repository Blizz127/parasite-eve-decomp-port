/*
 * Phase 6E-B18 — func_800528F0: PRNG table generator.
 *
 * Raw body: 143 words / 0x23C, exe 0x800528F0–0x80052B2B, file 0x430F0,
 * live split asm/disc1/42FC8.s:104–260; all 143 instruction words
 * verified exact against the SHA-exact retail executable.
 *
 * Sole call site: func_800527C8 @0x800527F4 (nop delay slot, no
 * arguments, return ignored).  First unresolved callee in B17, now
 * translated.
 *
 * Operation (ROM order):
 *   1. Read D_800A76A4 (timer tick, 60 Hz) → divide by 60 via
 *      unsigned reciprocal (multu ×0x88888889, mfhi, srl 5).
 *   2. Seed a local 32-bit LCG: s(n+1) = lo32(s(n) × 0x5D588B65) + 1.
 *   3. Generate 17 32-bit words: each accumulates the MSB of 32
 *      successive LCG states, shifted in from the right.
 *   4. XorShift expand to 521 words: W[i] = (W[i-17]<<23)^(W[i-16]>>9)^W[i-1]
 *      with a pre-iteration at i=16 using W[16],W[0],W[15].
 *   5. Extract low byte of each word to D_800A1B90[0..520].
 *   6–8. Self-referential XOR mixing (two passes):
 *        output[0..31]  ^= output[489..520]   (via D_800A1D79 overlap)
 *        output[32..520] ^= output[0..488]    (via D_800A1B70 overlap)
 *      D_800A1D79 = D_800A1B90 + 489, D_800A1B70 = D_800A1B90 - 32.
 *   9. Store 0x208 (520) at D_8009D038 ($gp+0x2C8).
 *
 * Classification: 1 — translated retail logic.
 */
#include "psx_compat.h"
#include "pe_sdk.h"
#include <stdint.h>

#define GA_TIMER       0x800A76A4u
#define GA_OUT         0x800A1B90u
#define GA_CNT         0x8009D038u
#define TABLE_BYTES    521
#define XORA_OFF       489

/* func_800528F0: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800528F0_port.c (src/func_800528F0.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
