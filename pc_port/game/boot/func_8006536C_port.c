/*
 * Phase 6E-B14 — func_8006536C: subsystem record-table clear + index
 * byte clear (translated retail logic, classification 1).
 *
 * Complete retail body (19 words / 0x4C, exe 0x8006536C-0x800653B4,
 * file offset 0x55B6C; live split asm/disc1/55430.s glabel line 556 —
 * the ONLY split containing the body; all 19 words verified against
 * the SHA-1-exact retail executable, taddr 0x80010000).
 *
 * No SDK/Psy-Q calls, no hardware registers, no GTE/coprocessor ops,
 * no callbacks, no allocation, no polling, no GPU command construction
 * or submission, no ordering-table/display-environment work, no reads
 * at all — pure fixed-count zero-stores.
 *
 * ROM-order operation map (retail $gp = 0x8009CD70):
 *
 *   Nested loop (outer a2 = 0..0x1B (28 rows, a1 += 0xC in the outer
 *   bnez delay slot), inner a0 = 0..2 (3 words, v1 += 4 in the inner
 *   bnez delay slot)):
 *     sw 0 -> D_800A3180 + row*0xC + col*4
 *   Row stride 0xC = 3 words, so the table is CONTIGUOUS: 84 words,
 *   span 0x800A3180..0x800A32CF (0x150 bytes).
 *
 *   0x800653AC  sb 0 -> 0x44($gp) = 0x8009CDB4   (byte)
 *   0x800653B0  jr $ra ; nop
 *
 *   $v0 = 0 on return (last sltiu result); the sole caller never
 *   consumes it.
 *
 * Context: the immediately following function func_800653B8 reads
 * lbu 0x44($gp) and indexes D_800A3180 + byte*12 — i.e. 0x8009CDB4 is
 * the current-record index into this 28-record table of 12-byte
 * records.  (func_800653B8 is not on the audited call path; recorded
 * here only as structural evidence.)
 *
 * Write extent: 0x800A3180..0x800A32CF (84 words) and the single byte
 * 0x8009CDB4.  Read extent: none.  Idempotent: fixed-value zero-stores
 * — including after PE_RamReset (region starts at 0).
 *
 * Call-site audit (exe-wide byte scan for the encoded jal 0x0C0194DB
 * plus decomp .word/jump-table scan): exactly ONE site —
 * func_8003E680 @0x8003E730, nop delay slot, immediately after the
 * real func_80034F10 call, immediately before the func_80038D1C call
 * (the overlapping splits 2E7D0.s and 2EE80.s are the same address,
 * not two sites).  No arguments; return unused.  No callers on any
 * reset/shutdown/scene/disc/interrupt/timer/audio/input path.
 *
 * No independent oracle is warranted: fixed-trip-count loops storing
 * the constant zero plus one constant byte store; no input-dependent
 * control flow; the 19-word exe verification plus the exact canary
 * write-footprint test is complete proof.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define GA_D_800A3180 0x800A3180u
#define GA_D_8009CDB4 0x8009CDB4u

#define TABLE_ROWS   28u
#define TABLE_COLS   3u
#define TABLE_STRIDE 0xCu

void func_8006536C(void)
{
    unsigned int row, col;

    /* 28 x 3-word table clear, row-major, retail order. */
    for (row = 0; row < TABLE_ROWS; row++) {
        for (col = 0; col < TABLE_COLS; col++) {
            PE_StoreU32(GA_D_800A3180 + row * TABLE_STRIDE + col * 4u, 0u);
        }
    }

    /* Current-record index byte. */
    PE_StoreU8(GA_D_8009CDB4, 0u);
}
