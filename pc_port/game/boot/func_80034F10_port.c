/*
 * Phase 6E-B13 — func_80034F10: subsystem table clear + flag-bit
 * clear (translated retail logic, classification 1).
 *
 * Complete retail body (45 words / 0xB4, exe 0x80034F10-0x80034FC0,
 * file offset 0x25710; live split asm/disc1/2422C.s glabel line 1439 —
 * the ONLY split containing the body; all 45 words verified against
 * the SHA-1-exact retail executable, taddr 0x80010000).
 *
 * No SDK/Psy-Q calls, no hardware registers, no GTE/coprocessor ops,
 * no callbacks, no allocation, no polling, no GPU command construction
 * or submission, no ordering-table/display-environment work.  One
 * guest READ exists: the final read-modify-write of the D_800B0CD8
 * flags word.
 *
 * ROM-order operation map (retail $gp = 0x8009CD70):
 *
 *   0x80034F10  sw 0 -> 0x578($gp) = 0x8009D2E8        (word)
 *
 *   Loop 1: i = 0..0x1FF (512 words, v1 += 4 in the bnez delay slot):
 *     sw 0 -> D_800A77F0 + i*4  => 0x800A77F0..0x800A7FEC
 *
 *   Loop 2 (0x80034F40..0x80034F4C): a1 = 1..0x40 with
 *   `sw 0 -> D_800B6A80` in the branch DELAY SLOT and no pointer
 *   advance — the same word is stored 64 times (63 taken branches +
 *   1 fall-through); net guest effect: D_800B6A80 = 0.  Reproduced
 *   here as one store, with the retail redundancy documented; the
 *   observable end state is identical and no timing/ordering consumer
 *   exists (sole caller, boot path, no interrupt model).
 *
 *   Loop 3 (nested): outer i = 0..0xD (14 rows, a2 += 0x280 in the
 *   outer bnez delay slot), inner j = 0..0x9F (160 words, v1 += 4 in
 *   the inner bnez delay slot):
 *     sw 0 -> D_800BEA90 + i*0x280 + j*4
 *   Matrix span: 0x800BEA90..0x800C0D8F (2240 words, 0x2300 bytes).
 *
 *   Scalar block (ROM order 53C, 49C, 580, 536, 4E4, 4B4):
 *     sw 0 -> 0x53C($gp) = 0x8009D2AC
 *     sw 0 -> 0x49C($gp) = 0x8009D20C
 *     sw 0 -> 0x580($gp) = 0x8009D2F0
 *     sh 0 -> 0x536($gp) = 0x8009D2A6   (halfword)
 *     sw 0 -> 0x4E4($gp) = 0x8009D254
 *     sw 0 -> 0x4B4($gp) = 0x8009D224
 *
 *   Final read-modify-write (the only read):
 *     0x80034F98  lw  v1 <- D_800B0CD8
 *     0x80034FB8  and v1 &= 0xFFFFCFFF  (addiu a0, -0x3001)
 *     0x80034FC0  sw  v1 -> D_800B0CD8  (in the jr $ra delay slot)
 *   Clears bits 12-13 of D_800B0CD8; all other bits preserved.
 *
 *   $v0 = &D_800B0CD8 on return; the sole caller never consumes it.
 *
 * Write extent: 0x8009D2E8, 0x800A77F0..0x800A7FEC (512 words),
 * 0x800B6A80 (word), 0x800BEA90..0x800C0D8F (2240 words),
 * 0x8009D2AC, 0x8009D20C, 0x8009D2F0, 0x8009D2A6..0x8009D2A7
 * (halfword), 0x8009D254, 0x8009D224, D_800B0CD8 (RMW, bits 12-13).
 * Read extent: D_800B0CD8 (one word).  Idempotent: zero-stores are
 * fixed-value, the RMW clears already-clear bits on repeat —
 * including after PE_RamReset (D_800B0CD8 starts at 0).
 *
 * Call-site audit (exe-wide scan for the encoded jal 0x0C00D3C4):
 * exactly ONE site — func_8003E680 @0x8003E728, nop delay slot,
 * immediately after the real func_8001A890 call, immediately before
 * the func_8006536C call (the overlapping splits 2E7D0.s and 2EE80.s
 * are the same address, not two sites).  No arguments; return unused.
 * No .word/jump-table references; no callers on any reset/shutdown/
 * scene/disc/interrupt path.
 *
 * No independent oracle is warranted: fixed-trip-count loops storing
 * the constant zero plus one fixed-mask RMW of a single flags word;
 * no input-dependent control flow; the 45-word exe verification plus
 * the exact canary write-footprint test is complete proof.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define GA_D_8009D2E8 0x8009D2E8u
#define GA_D_800A77F0 0x800A77F0u
#define GA_D_800B6A80 0x800B6A80u
#define GA_D_800BEA90 0x800BEA90u
#define GA_D_8009D2AC 0x8009D2ACu
#define GA_D_8009D20C 0x8009D20Cu
#define GA_D_8009D2F0 0x8009D2F0u
#define GA_D_8009D2A6 0x8009D2A6u
#define GA_D_8009D254 0x8009D254u
#define GA_D_8009D224 0x8009D224u
#define GA_D_800B0CD8 0x800B0CD8u

#define ARRAY1_WORDS 512u
#define MATRIX_ROWS  14u
#define MATRIX_COLS  160u
#define MATRIX_STRIDE 0x280u

void func_80034F10(void)
{
    unsigned int i, j;

    PE_StoreU32(GA_D_8009D2E8, 0u);

    /* 512-word array clear, retail order. */
    for (i = 0; i < ARRAY1_WORDS; i++) {
        PE_StoreU32(GA_D_800A77F0 + i * 4u, 0u);
    }

    /* Retail stores this same word 64 times (delay-slot loop with no
     * pointer advance); net effect and end state are one clear. */
    PE_StoreU32(GA_D_800B6A80, 0u);

    /* 14 x 160-word matrix clear, row-major, retail order. */
    for (i = 0; i < MATRIX_ROWS; i++) {
        for (j = 0; j < MATRIX_COLS; j++) {
            PE_StoreU32(GA_D_800BEA90 + i * MATRIX_STRIDE + j * 4u, 0u);
        }
    }

    /* Scalar block, ROM order. */
    PE_StoreU32(GA_D_8009D2AC, 0u);
    PE_StoreU32(GA_D_8009D20C, 0u);
    PE_StoreU32(GA_D_8009D2F0, 0u);
    PE_StoreU16(GA_D_8009D2A6, 0u);
    PE_StoreU32(GA_D_8009D254, 0u);
    PE_StoreU32(GA_D_8009D224, 0u);

    /* Final RMW: D_800B0CD8 &= ~0x3000 (retail: in the jr delay
     * slot). */
    PE_StoreU32(GA_D_800B0CD8, PE_LoadU32(GA_D_800B0CD8) & ~0x3000u);
}

/*
 * PE-BTL9 — func_80034FC4: 14-slot actor freelist at D_800BEA90.
 * 29 words 0x80034FC4..0x80035038. Sole TEXT caller 3F074 @
 * 0x8003F0B0 (before the 6C5BC poll). Links 13 next-pointers at
 * +4, stride 0x280, nulls the 14th +4, publishes the head to
 * gp+0x53C (D_8009D2AC), clears gp+0x536 / gp+0x49C / gp+0x4E4,
 * and zeros 16 qwords at D_800A7624. This is the pool 35038 pops.
 */
#define GA_D_800A7624 0x800A7624u
#define GA_D_800C0B14 0x800C0B14u
#define SLOT_STRIDE   0x280u

void func_80034FC4(void)
{
    unsigned int i;
    pe_addr_t next;

    PE_StoreU16(GA_D_8009D2A6, 0u);
    PE_StoreU32(GA_D_8009D20C, 0u);
    PE_StoreU32(GA_D_8009D2AC, GA_D_800BEA90);
    next = GA_D_800BEA90 + SLOT_STRIDE;
    for (i = 0; i < 13u; i++) {
        PE_StoreU32(GA_D_800BEA90 + 4u + i * SLOT_STRIDE, next);
        next += SLOT_STRIDE;
    }
    PE_StoreU32(GA_D_800C0B14, 0u);
    for (i = 0; i < 16u; i++)
        PE_StoreU32(GA_D_800A7624 + i * 8u, 0u);
    PE_StoreU32(GA_D_8009D254, 0u);
}
