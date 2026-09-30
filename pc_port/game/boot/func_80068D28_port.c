/*
 * Phase 6E-B10 — func_80068D28: double-buffered display-record data
 * initializer (translated retail logic, classification 1).
 *
 * Complete retail body (63 words / 0xFC, exe 0x80068D28-0x80068E20,
 * file offset 0x59528; live split asm/disc1/55430.s glabel line 4744 —
 * the ONLY split containing it; all 63 words verified against the
 * SHA-1-exact retail executable, taddr 0x80010000).
 *
 * The function writes ONLY guest memory: no SDK/Psy-Q calls, no
 * hardware registers, no GTE/coprocessor ops, no callbacks, no
 * allocation, no polling.  The 0xE1000400|… word is DATA shaped like a
 * GP0 draw-mode command stored into a guest structure (the 0x140/0xE0
 * halfwords are 320x224); nothing here touches the GPU.  The
 * D_800BCF88 structure is read/written by many later functions
 * (2A0C.s, 8804.s, 55430.s, func_8003F3FC's D_800BCFE8 block) — this
 * rung reproduces the initializer only.
 *
 * ROM-order operation map (base B = D_800BCF88 = 0x800BCF88):
 *
 *   Scalar block (ROM order):
 *     0x80068D60  sh 0x00FF -> B+0x64 (D_800BCFEC)
 *     0x80068D68  sh 0x00FF -> B+0x62 (D_800BCFEA)
 *     0x80068D70  sh 0x00FF -> B+0x60 (D_800BCFE8)
 *     0x80068D7C  sb 0x01   -> B+0x66 (D_800BCFEE)
 *     0x80068D88  sh 0      -> B+0x6C (D_800BCFF4)
 *     0x80068D90  sh 0      -> B+0x6A (D_800BCFF2)
 *     0x80068D98  sh 0      -> B+0x68 (D_800BCFF0)
 *     0x80068DA0  sb 0x02   -> B+0x67 (D_800BCFEF)
 *
 *   Loop, i = 0..1 (slti bound 2; a0 = B + i*0x10, a1 = B + i*0x8 —
 *   a1 advances in the bnez delay slot, a0 mid-body):
 *     sb 3    -> a0+0x33
 *     sb 0x60 -> a0+0x37
 *     sb (u8)lhu(B+0x60) -> a0+0x34     = 0xFF (just-stored scalar)
 *     sb (u8)lhu(B+0x62) -> a0+0x35     = 0xFF
 *     lhu v1 <- B+0x64 (stored at +0x36 below) = 0xFF
 *     lbu v0 <- a0+0x37 (the 0x60 just stored)
 *     sh 0     -> a0+0x38
 *     sh 0     -> a0+0x3A
 *     sh 0x140 -> a0+0x3C
 *     sh 0xE0  -> a0+0x3E
 *     sb v0|2  -> a0+0x37               -> final 0x62
 *     sb (u8)v1 -> a0+0x36              = 0xFF
 *     sb 1     -> a1+0x53
 *     sw 0xE1000400 | ((lbu(B+0x67) & 3) << 5) -> a1+0x54
 *                                       = 0xE1000440 (scalar is 2)
 *
 *   Post-loop:
 *     0x80068E18  sh 0 -> B+0x6E (D_800BCFF6)
 *     0x80068E20  sh 0 -> B+0x70 (D_800BCFF8)  (in the jr delay slot)
 *
 *   $v0 = 0 on return; the sole caller never consumes it.
 *
 * Write extent: min 0x800BCFBB (B+0x33), max 0x800BCFF9 (B+0x71).
 * Read extent: B+0x60..B+0x65 and B+0x67 — all values this same
 * invocation just stored, so the derived stores are deterministic and
 * the function is idempotent, including after PE_RamReset.
 *
 * Call-site audit (exe-wide scan for the encoded jal 0x0C01A34A):
 * exactly ONE site — func_8003E680 @0x8003E710, nop delay slot,
 * immediately after the real func_8005BCA8 call, immediately before
 * the func_800124F8 call.  No arguments; return unused (the decomp's
 * matched func_8003E680 C leaf declares it void(void) and is
 * byte-exact).
 *
 * No independent oracle is warranted: straight-line fixed-value and
 * load-after-store data initialization with one exact-trip-count loop;
 * the 63-word exe verification plus exact store-order reproduction is
 * complete proof.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define GA_D_800BCF88 0x800BCF88u

/* func_80068D28: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80068D28_port.c (src/func_80068D28.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
