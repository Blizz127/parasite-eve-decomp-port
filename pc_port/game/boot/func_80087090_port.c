/*
 * Phase 6E-B45 — func_80087090: SPU upload retry wrapper
 * (20 retail words / 0x50 bytes, exe 0x80087090–0x800870DF,
 * file offset 0x77890, live split asm/disc1/75F44.s:2102–2123;
 * all 20 instruction words verified exact against the SHA-exact
 * retail executable).
 *
 * Retail signature:
 *   int func_80087090(pe_addr_t buffer, int transfer_count)
 *
 * Two call sites in the executable:
 *   1. func_8006A9E4 @0x8006AC28 (jal, nop delay slot):
 *      a0 = lw(D_800B0E6C), a1 = 1; return IGNORED (v0 dead).
 *   2. func_8006CC68 @0x8006CF98 (jal, addu $a1,$zero,$zero delay slot):
 *      a0 = $s5, a1 = 0; return consumed (stored in $a2, compared
 *      against $s6).
 *
 * ROM-order operation map:
 *   Prologue: save $s0/$s1/$s2/$ra on 0x20-frame.
 *   $s0 = a0 (buffer), $s1 = a1 (count), $s2 = 1.
 *   Loop:
 *     jal func_800851A8        (a0 = buffer, a1 = count via delay slots)
 *     beq v0, 1 → retry        (delay slot reloads a0 = s0 for next iter)
 *   Epilogue: restore $s2/$s1/$s0/$ra, jr $ra (nop delay slot).
 *   Return: last v0 from func_800851A8 (0 on success, negative on error;
 *   1 is consumed by the retry branch and never returned).
 *
 * Classification: 1 — translated retail logic.  func_800851A8 is
 * translated (B46 prefix: magic number check + error path; success
 * path routes through the centralized bootstrap boundary).
 *
 * Dependency:
 *   func_800851A8  TRANSLATED (B46 prefix; hardware success path
 *                  routes through bootstrap boundary)
 */
#include "psx_compat.h"

/* func_800851A8 — SPU DMA upload prefix (B46).
 * Returns 0 on success (via bootstrap), -1 on magic number failure. */
extern int func_800851A8(pe_addr_t buffer, int count);

/* ── func_80087090: SPU upload retry wrapper ───────────────────────────── */
int func_80087090(pe_addr_t buffer, int count)
{
    int ret;
    do {
        ret = func_800851A8(buffer, count);
    } while (ret == 1);
    return ret;
}

/* func_800870E0: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800870E0_port.c (src/func_800870E0.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */
