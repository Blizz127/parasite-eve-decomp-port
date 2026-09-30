/*
 * Phase 6E-B4 — func_8003EAC8: GTE LZCS/LZCR-indexed registration leaf.
 * TRANSLATED retail logic (category 1: registration helper using GTE
 * arithmetic), no longer an unresolved provider.
 *
 * Complete raw-MIPS audit (asm/disc1/2EF54.s:255-271 — the LIVE split per
 * configs/USA/disc1.yaml; exe 0x8003EAC8-0x8003EB03, 15 words / 0x3C,
 * file offset 0x2F2C8, "Handwritten function"):
 *   sw    $a0, 0($sp)          ; stack scratch (transient, unobservable)
 *   mtc2  $a0, $30             ; GTE data reg 30 = LZCS source
 *   lui   $v0, 0x8000
 *   beq   $a0, $v0, .done      ; a0 == 0x80000000 special case ...
 *   addiu $v1, $zero, 0x1F     ; ... delay slot: idx = 31 (executes ALWAYS)
 *   swc2  $31, 0($sp)          ; GTE data reg 31 = LZCR result
 *   lw    $v0, 0($sp)
 *   nop
 *   subu  $v1, $v1, $v0        ; idx = 31 - LZCR
 * .done:
 *   sll   $v0, $v1, 2
 *   lui   $at, %hi(D_800A76F0)
 *   addu  $at, $at, $v0
 *   sw    $a1, %lo(D_800A76F0)($at)   ; D_800A76F0[idx] = a1 (word store)
 *   jr    $ra
 *   nop
 *
 * GTE LZCR semantics (cop2, no$psx spec): count of leading bits equal to
 * the sign bit — leading zeros for nonnegative input (0 -> 32), leading
 * ones for negative input (0xFFFFFFFF -> 32).
 *
 * Exact edge behavior, preserved verbatim (oracle: tools/lzcr_oracle.py,
 * a tiny interpreter over the verified retail words — NOT this formula):
 *   idx range [-1, 31];  dest = D_800A76F0 + 4*idx (mod 2^32 via sll/addu)
 *   a0 one-hot bit k          -> idx k        (k = 0..30)
 *   a0 = 0x80000000           -> idx 31       (special case; plain LZCR
 *                                              would give 30)
 *   a0 = 0                    -> idx -1       -> dest 0x800A76EC (one word
 *   a0 = 0xFFFFFFFF           -> idx -1          BELOW the table; valid
 *                                              guest RAM — preserved, NOT
 *                                              clamped)
 *   a0 positive multi-bit     -> highest SET bit index
 *   a0 negative (not 0x80000000) -> 31 - leading ones = highest ZERO bit
 * No GTE state is observably live after return (LZCS/LZCR are not read by
 * any caller); the sw/swc2/lw stack round-trip is transient scratch and is
 * not modeled.  No return value is consumed by any caller.
 *
 * Call-site audit correction: the Phase 6E-B3 handoff claimed "63 call
 * sites exe-wide".  The jal encoding (0x0C03FAB2) appears 20 times each in
 * THREE overlapping split files (2E7D0.s / 2EE80.s / 2EF54.s) covering the
 * SAME addresses; only 2EF54.s is linked.  The true count is 20 DISTINCT
 * call sites, ALL inside func_8003E974, all with constant arguments:
 * 19 one-hot masks (bits 0-10, 13, 24-31) + 0x80000000.  No zero, no
 * multi-bit, no dynamic, no other negative input is reachable in the exe.
 *
 * The bounded argument log remains as NON-SEMANTIC test instrumentation:
 * the guest-memory store above happens identically whether the recorder is
 * enabled or disabled, and production behavior never depends on it.
 *
 * PE_GTE_LZCR (the exact 32-bit LZCS/LZCR arithmetic) lives in
 * platform/pe_gte.c next to the other libgte state — a narrow helper, not
 * a GTE subsystem.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define GA_D_800A76F0   0x800A76F0u
#define PE_3EAC8_LOG_MAX  64

static struct { int a0, a1; } g_3eac8_log[PE_3EAC8_LOG_MAX];
static int g_3eac8_count;
static int g_3eac8_enabled = 1;

void func_8003EAC8(int a0, int a1)
{
    uint32_t mask = (uint32_t)a0;
    int32_t idx;
    pe_addr_t dest;

    /* Non-semantic instrumentation first (overflow visible: RecordCount
     * keeps climbing past PE_3EAC8_LOG_MAX; RecordAt stays bounded). */
    if (g_3eac8_enabled) {
        if (g_3eac8_count < PE_3EAC8_LOG_MAX) {
            g_3eac8_log[g_3eac8_count].a0 = a0;
            g_3eac8_log[g_3eac8_count].a1 = a1;
        }
        g_3eac8_count++;
    }

    /* Retail: idx = (a0 == 0x80000000) ? 31 : 31 - LZCR(a0).  For idx = -1
     * the (uint32_t)idx << 2 wrap reproduces the sll/addu chain exactly:
     * 0x800A76F0 + 0xFFFFFFFC = 0x800A76EC.  Bounds-checked store — the
     * whole reachable range 0x800A76EC..0x800A776C is valid guest RAM. */
    idx = (mask == 0x80000000u) ? 31 : 31 - (int32_t)PE_GTE_LZCR(mask);
    dest = GA_D_800A76F0 + ((uint32_t)idx << 2);
    PE_StoreU32(dest, (uint32_t)a1);
}

/* Test instrumentation (read-only accessors; ResetTestState resets). */
void PE_3EAC8_RecordReset(void) { g_3eac8_count = 0; g_3eac8_enabled = 1; }
void PE_3EAC8_RecordSetEnabled(int enabled) { g_3eac8_enabled = enabled != 0; }
int  PE_3EAC8_RecordCount(void) { return g_3eac8_count; }
int  PE_3EAC8_RecordAt(int index, int *a0, int *a1)
{
    if (index < 0 || index >= g_3eac8_count || index >= PE_3EAC8_LOG_MAX)
        return 0;
    *a0 = g_3eac8_log[index].a0;
    *a1 = g_3eac8_log[index].a1;
    return 1;
}
