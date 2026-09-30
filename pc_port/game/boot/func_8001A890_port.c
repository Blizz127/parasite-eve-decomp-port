/*
 * Phase 6E-B12 — func_8001A890: subsystem scalar/array clear
 * (translated retail logic, classification 1).
 *
 * Complete retail body (34 words / 0x88, exe 0x8001A890-0x8001A914,
 * file offset 0xB090; live split asm/disc1/A404.s glabel line 998 —
 * the ONLY split containing the body; all 34 words verified against
 * the SHA-1-exact retail executable, taddr 0x80010000).
 *
 * The function writes ONLY guest memory and reads nothing: no
 * SDK/Psy-Q calls, no hardware registers, no GTE/coprocessor ops, no
 * callbacks, no allocation, no polling, no GPU command construction or
 * submission, no ordering-table or display-environment work.  Its
 * regions extend the subsystem state block that func_800124F8 began:
 * 0x8009CE08 is immediately above 124F8's 0x8009CE04 word, and the
 * 20-word array at D_8009DFB0 is immediately above 124F8's 16-word
 * array (..0x8009DFAF).
 *
 * ROM-order operation map (retail $gp = 0x8009CD70):
 *
 *   0x8001A890  sw 0 -> 0x98($gp) = 0x8009CE08        (word)
 *
 *   Halfword loop, v1 = 0, 4 (sltiu bound 8; v1 += 4 mid-body, a0 = 0
 *   materialized in the bnez delay slot for the next loop):
 *     sh 0 -> D_8009CE0C + v1   => 0x8009CE0C, 0x8009CE10
 *     sh 0 -> D_8009CE0E + v1   => 0x8009CE0E, 0x8009CE12
 *   (contiguous 8 bytes 0x8009CE0C..0x8009CE13; %hi = 0x800A via the
 *   +0x8000 sign compensation, %lo negative)
 *
 *   0x8001A8C0  sw 0 -> 0xA4($gp) = 0x8009CE14        (word)
 *
 *   Array loop, i = 0..0x13 (20 words, v1 += 4 in the bnez delay slot):
 *     sw 0 -> D_8009DFB0 + i*4  => 0x8009DFB0..0x8009DFFC
 *
 *   Halfword block (ROM order A8, B8, B4, B0, AC, BC):
 *     sh 0 -> 0xA8($gp) = 0x8009CE18
 *     sh 0 -> 0xB8($gp) = 0x8009CE28
 *     sh 0 -> 0xB4($gp) = 0x8009CE24
 *     sh 0 -> 0xB0($gp) = 0x8009CE20
 *     sh 0 -> 0xAC($gp) = 0x8009CE1C
 *     sh 0 -> 0xBC($gp) = 0x8009CE2C
 *   (six halfwords at 4-byte stride, 0x8009CE18..0x8009CE2D; the
 *   interleaved bytes CE1A/B, CE1E/F, CE22/3, CE26/7, CE2A/B, CE2E/F
 *   are NOT written)
 *
 *   Word block (ROM order 468, 48C, 588, 4D8):
 *     sw 0 -> 0x468($gp) = 0x8009D1D8
 *     sw 0 -> 0x48C($gp) = 0x8009D1FC
 *     sw 0 -> 0x588($gp) = 0x8009D2F8
 *     sw 0 -> 0x4D8($gp) = 0x8009D248
 *
 *   Final halfwords (ROM order 4F4, 45C):
 *     sh 0 -> 0x4F4($gp) = 0x8009D264
 *     sh 0 -> 0x45C($gp) = 0x8009D1CC
 *
 *   0x8001A910  jr $ra / nop;  $v0 = 0 on return (final sltiu result);
 *   the sole caller never consumes it.
 *
 * Write extent: 0x8009CE08..0x8009CE17 (words + stride-2 halfwords,
 * contiguous), 0x8009CE18..0x8009CE2D (six stride-4 halfwords — the
 * interleaved upper halfwords are NOT written), 0x8009D1CC..0x8009D1CD
 * (halfword), 0x8009D1D8, 0x8009D1FC, 0x8009D248 (words),
 * 0x8009D264..0x8009D265 (halfword), 0x8009D2F8 (word),
 * 0x8009DFB0..0x8009DFFC (20 words).  Read extent: none.
 * Idempotent by construction (pure zero stores), including after
 * PE_RamReset.
 *
 * Call-site audit (exe-wide scan for the encoded jal 0x0C006A24):
 * exactly ONE site — func_8003E680 @0x8003E720, nop delay slot,
 * immediately after the real func_800124F8 call, immediately before
 * the func_80034F10 call (the overlapping splits 2E7D0.s and 2EE80.s
 * are the same address, not two sites).  No arguments; return unused.
 * No .word/jump-table references; no callers on any reset/shutdown/
 * scene/disc/interrupt path.
 *
 * No independent oracle is warranted: two fixed-trip-count loops with
 * immediate strides storing the constant zero, no input-dependent
 * control flow and no reads; the 34-word exe verification plus the
 * exact canary write-footprint test is complete proof.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define GA_D_8009CE08 0x8009CE08u
#define GA_D_8009CE0C 0x8009CE0Cu
#define GA_D_8009CE14 0x8009CE14u
#define GA_D_8009CE18 0x8009CE18u
#define GA_D_8009CE1C 0x8009CE1Cu
#define GA_D_8009CE20 0x8009CE20u
#define GA_D_8009CE24 0x8009CE24u
#define GA_D_8009CE28 0x8009CE28u
#define GA_D_8009CE2C 0x8009CE2Cu
#define GA_D_8009D1CC 0x8009D1CCu
#define GA_D_8009D1D8 0x8009D1D8u
#define GA_D_8009D1FC 0x8009D1FCu
#define GA_D_8009D248 0x8009D248u
#define GA_D_8009D264 0x8009D264u
#define GA_D_8009D2F8 0x8009D2F8u
#define GA_D_8009DFB0 0x8009DFB0u

#define ARRAY_WORDS 20u

void func_8001A890(void)
{
    unsigned int i;

    PE_StoreU32(GA_D_8009CE08, 0u);

    /* Halfword loop: v1 = 0 then 4 — 0x8009CE0C/0x8009CE0E and
     * 0x8009CE10/0x8009CE12, retail store order per iteration. */
    for (i = 0; i < 2; i++) {
        PE_StoreU16(GA_D_8009CE0C + i * 4u, 0u);
        PE_StoreU16(GA_D_8009CE0C + 2u + i * 4u, 0u);
    }

    PE_StoreU32(GA_D_8009CE14, 0u);

    /* 20-word array clear, retail order. */
    for (i = 0; i < ARRAY_WORDS; i++) {
        PE_StoreU32(GA_D_8009DFB0 + i * 4u, 0u);
    }

    /* Halfword block, ROM order (A8, B8, B4, B0, AC, BC). */
    PE_StoreU16(GA_D_8009CE18, 0u);
    PE_StoreU16(GA_D_8009CE28, 0u);
    PE_StoreU16(GA_D_8009CE24, 0u);
    PE_StoreU16(GA_D_8009CE20, 0u);
    PE_StoreU16(GA_D_8009CE1C, 0u);
    PE_StoreU16(GA_D_8009CE2C, 0u);

    /* Word block, ROM order (468, 48C, 588, 4D8). */
    PE_StoreU32(GA_D_8009D1D8, 0u);
    PE_StoreU32(GA_D_8009D1FC, 0u);
    PE_StoreU32(GA_D_8009D2F8, 0u);
    PE_StoreU32(GA_D_8009D248, 0u);

    /* Final halfwords, ROM order (4F4, 45C). */
    PE_StoreU16(GA_D_8009D264, 0u);
    PE_StoreU16(GA_D_8009D1CC, 0u);
}

/*
 * PE-BTL8 — func_8001A918 (56 words, 0x8001A918..0x8001A9F8).
 * SHA-256 c38eb3c9…6496. Zero jal/jalr. Sole TEXT caller is
 * 3F074 @ 0x8003F23C after the 6C5BC poll exits.
 *
 * Rebases the object at D_800B1620 (overlay+0x948, writer
 * 0x8006B8E8 / zeroer 0x8006B3F0). If +0x18 <= 0x80000000 the
 * words at +0x18/+0x1C/+0x24 and nonzero +0x20 are obj-relative
 * offsets; a table of lhu(+2) words at +0x28 is likewise rebased.
 * Already-relocated +0x18 takes the short publish of +0x28 and +0x20.
 * Guest 0 maps through Kuseg 0x80000000 (host exception-vector
 * area is APPROXIMATION zeros). Sibling of func_8001A890's clears.
 */
#define GA_B1620 0x800B1620u

static pe_addr_t pe_1a918_kseg0(pe_addr_t addr)
{
    return 0x80000000u | (addr & 0x1FFFFFu);
}

void func_8001A918(void)
{
    pe_addr_t obj;
    unsigned int off18;
    unsigned int i;
    unsigned int count;
    unsigned int word;

    obj = pe_1a918_kseg0(PE_LoadU32(GA_B1620));
    off18 = PE_LoadU32(obj + 0x18u);
    PE_StoreU32(GA_D_8009D1FC, obj);

    if (off18 > 0x80000000u) {
        PE_StoreU32(GA_D_8009CE08, obj + 0x28u);
        PE_StoreU32(GA_D_8009D1D8, PE_LoadU32(obj + 0x20u));
        return;
    }

    PE_StoreU32(obj + 0x18u, obj + off18);
    PE_StoreU32(obj + 0x1Cu, obj + PE_LoadU32(obj + 0x1Cu));
    PE_StoreU32(obj + 0x24u, obj + PE_LoadU32(obj + 0x24u));
    word = PE_LoadU32(obj + 0x20u);
    if (word != 0u)
        PE_StoreU32(obj + 0x20u, obj + word);

    PE_StoreU32(GA_D_8009CE08, obj + 0x28u);
    PE_StoreU32(GA_D_8009D1D8, PE_LoadU32(obj + 0x20u));
    PE_StoreU32(GA_D_8009CE14, PE_LoadU32(obj + 0x24u));
    count = PE_LoadU16(obj + 2u);
    PE_StoreU16(obj + 8u, (uint16_t)((PE_LoadU16(obj + 8u) >> 5) + 1u));
    for (i = 0; i < count; i++) {
        word = PE_LoadU32(obj + 0x28u + i * 4u);
        PE_StoreU32(obj + 0x28u + i * 4u, obj + word);
    }
}
