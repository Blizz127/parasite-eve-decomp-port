/*
 * Phase 6E-B11 — func_800124F8: subsystem table clear (translated
 * retail logic, classification 1).
 *
 * Complete retail body (31 words / 0x7C, exe 0x800124F8-0x80012570,
 * file offset 0x2CF8; live split asm/disc1/2A0C.s glabel line 220 —
 * the ONLY split containing the body; all 31 words verified against
 * the SHA-1-exact retail executable, taddr 0x80010000).
 *
 * The function writes ONLY guest memory: no SDK/Psy-Q calls, no
 * hardware registers, no GTE/coprocessor ops, no callbacks, no
 * allocation, no polling, no reads.  Despite following the
 * display-record initializer in startup order it touches no display
 * state, submits no GP0/GP1 commands, and builds no ordering tables —
 * it is a pure zero-fill of a 72-entry subsystem table (72 x 0x2C-byte
 * records at D_8009D310, addressed once via .word D_8009D310 in the
 * rodata pointer at 0x80094554), a 16-word array at D_8009DF70
 * (immediately after the table: contiguous), and five scalars.
 *
 * ROM-order operation map (retail $gp = 0x8009CD70):
 *
 *   0x80012508  sw 0 -> 0x590($gp) = 0x8009D300  (word)
 *   0x8001250C  sh 0 -> 0x598($gp) = 0x8009D308  (halfword; 0x8009D304
 *               is NOT touched)
 *   0x80012510  sw 0 -> 0x8C($gp)  = 0x8009CDFC  (word)
 *
 *   Matrix loop: outer i = 0..0x47 (72 rows, row byte stride 0x2C —
 *   a2 += 0x2C in the outer bnez delay slot), inner j = 0..0xA (11
 *   words, v1 += 4 in the inner bnez delay slot):
 *     sw 0 -> D_8009D310 + i*0x2C + j*4
 *   Table span: 0x8009D310..0x8009DF6F (792 words, 0xC60 bytes).
 *
 *   0x80012544  sw 0 -> 0x90($gp) = 0x8009CE00   (word)
 *
 *   Array loop: i = 0..0xF (16 words, v1 += 4 in the bnez delay slot):
 *     sw 0 -> D_8009DF70 + i*4
 *   Array span: 0x8009DF70..0x8009DFAF (contiguous with the table end).
 *
 *   0x80012568  sw 0 -> 0x94($gp) = 0x8009CE04   (word)
 *   0x8001256C  jr $ra / nop
 *
 *   $v0 = 0 on return (final sltiu result); the sole caller never
 *   consumes it.  %hi fields use the +0x8000 sign compensation
 *   (lui 0x800A + negative %lo), resolving to 0x8009D310 / 0x8009DF70
 *   exactly.
 *
 * Write extent: 0x8009CDFC, 0x8009CE00, 0x8009CE04, 0x8009D300,
 * 0x8009D308..0x8009D309 (halfword), 0x8009D310..0x8009DFAF.
 * Read extent: none.  Idempotent by construction (pure zero stores),
 * including after PE_RamReset.
 *
 * Call-site audit (exe-wide scan for the encoded jal 0x0C00493E):
 * exactly ONE site — func_8003E680 @0x8003E718, nop delay slot,
 * immediately after the real func_80068D28 call, immediately before
 * the func_8001A890 call (the overlapping splits 2E7D0.s and 2EE80.s
 * are the same address, not two sites).  No arguments; return unused.
 * No callers on any reset/shutdown/scene/disc/interrupt path.
 *
 * No independent oracle is warranted: two fixed-trip-count loops with
 * immediate strides storing the constant zero, no input-dependent
 * control flow and no reads; the 31-word exe verification plus the
 * exact canary write-footprint test is complete proof.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define GA_D_8009CDFC 0x8009CDFCu
#define GA_D_8009CE00 0x8009CE00u
#define GA_D_8009CE04 0x8009CE04u
#define GA_D_8009D300 0x8009D300u
#define GA_D_8009D308 0x8009D308u
#define GA_D_8009D310 0x8009D310u
#define GA_D_8009DF70 0x8009DF70u

#define TABLE_ROWS   72u
#define TABLE_COLS   11u
#define TABLE_STRIDE 0x2Cu
#define ARRAY_WORDS  16u

void func_800124F8(void)
{
    unsigned int i, j;

    /* Scalar clears, retail store order. */
    PE_StoreU32(GA_D_8009D300, 0u);
    PE_StoreU16(GA_D_8009D308, 0u);
    PE_StoreU32(GA_D_8009CDFC, 0u);

    /* 72 x 11-word matrix clear, row-major, retail order. */
    for (i = 0; i < TABLE_ROWS; i++) {
        for (j = 0; j < TABLE_COLS; j++) {
            PE_StoreU32(GA_D_8009D310 + i * TABLE_STRIDE + j * 4u, 0u);
        }
    }

    PE_StoreU32(GA_D_8009CE00, 0u);

    /* 16-word array clear, retail order. */
    for (i = 0; i < ARRAY_WORDS; i++) {
        PE_StoreU32(GA_D_8009DF70 + i * 4u, 0u);
    }

    PE_StoreU32(GA_D_8009CE04, 0u);
}
