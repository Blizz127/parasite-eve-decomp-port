/*
 * Phase 6E-B15 — func_80038D1C: byte flag test-and-clear status leaf
 * (translated retail logic, classification 1; ALSO a matched C leaf in
 * the matching decomp — src/func_80038D1C.c, Phase 5ER, config entry
 * [0x2951C, c, func_80038D1C]).
 *
 * Complete retail body (11 words / 0x2C, exe 0x80038D1C-0x80038D44,
 * file offset 0x2951C; live split asm/disc1/2951C.s glabel line 12 —
 * the ONLY split containing the body; all 11 words verified against
 * the SHA-1-exact retail executable, taddr 0x80010000).
 *
 * No SDK/Psy-Q calls, no hardware registers, no GTE/coprocessor ops,
 * no callbacks, no allocation, no polling, no GPU/MDEC/SPU/DMA/input/
 * disc work, no loops.  One guest byte read; one conditional guest
 * byte write.
 *
 * ROM-order operation map (no $gp use):
 *
 *   0x80038D1C  lui/addiu  v1 = &D_80091A20
 *   0x80038D24  lbu  v0 <- D_80091A20              (byte read)
 *   0x80038D2C  bnez v0, .L80038D3C
 *   0x80038D30   addu v0, 0            (branch delay slot: v0 = 0)
 *   0x80038D34  j .L80038D40
 *   0x80038D38   addiu v0, 0xFF        (jump delay slot: v0 = 0xFF)
 *   .L80038D3C:
 *   0x80038D3C  sb 0 -> D_80091A20     (byte write, taken path only)
 *   .L80038D40:
 *   0x80038D40  jr $ra ; nop
 *
 * Return contract (int): if D_80091A20 != 0 -> clear it, return 0;
 * if D_80091A20 == 0 -> return 0xFF (255; addiu v0, zero, 0xFF — NOT
 * -1).  Both retail call sites IGNORE the return value:
 *
 *   1. func_8003E680 @0x8003E738 (nop delay slot, FINAL call —
 *      immediately followed by the lw $ra/lw $s0/addiu $sp/jr $ra
 *      epilogue; func_8003E680 is void, does not propagate $v0).
 *   2. func_8006E9A0 @0x8006EB7C (post-poll-loop call; the matched
 *      decomp source documents "return ignored here").
 *
 * Exe-wide byte scan for the encoded jal 0x0C00E347 found exactly
 * these two sites; no .word/jump-table references.
 *
 * Caller chain past func_8003E680: func_8003E680's sole caller is
 * func_8001220C @0x8001227C (nop delay slot); retail then calls
 * func_8006A9E4 @0x80012284 and stores $s3 -> D_8009D280.  With
 * func_80038D1C translated, func_8003E680 is FULLY translated and
 * the strict frontier advances to func_8006A9E4 (from func_8001220C).
 *
 * D_80091A20 context (not implemented here): a shared byte flag in
 * the 0x80038D1C..0x80039xxx subsystem — writers at 0x80039334/
 * 0x80039468 (sb $v0) and 0x8003935C/0x80039650 (sb $zero); readers
 * at 0x80038E04/0x800394BC.  func_80038D1C is the subsystem's
 * test-and-clear status leaf; it is fully self-contained.
 *
 * Read extent: D_80091A20 (one byte).  Write extent: D_80091A20
 * (one byte, ONLY when it was nonzero).  Repeated-call behavior:
 * first call with flag set returns 0 and clears; the very next call
 * returns 0xFF (guest state stable — flag stays 0).  After
 * PE_RamReset the flag is 0: return 0xFF, no write.
 *
 * No independent oracle is warranted: an 11-word two-path leaf, also
 * a matched C leaf in the matching decomp; the 11-word exe
 * verification plus both return paths plus the exact canary
 * write-footprint tests (including the no-write path) are complete
 * proof.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define GA_D_80091A20 0x80091A20u

/* func_80038D1C: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80038D1C_port.c (src/func_80038D1C.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
