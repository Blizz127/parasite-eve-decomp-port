/*
 * Phase 6E-B24 — func_8005BCBC: resource-state pointer/count selector.
 *
 * Raw body: 21 words / 0x54, executable 0x8005BCBC..0x8005BD0F, file
 * offset 0x4C4BC, live split asm/disc1/4C4BC.s:12-32 (yaml segment
 * [0x4C4BC, asm]).  All 21 words verified exact against the SHA-exact
 * retail executable (SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b).
 *
 * ROM-order operation map (retail $gp = 0x8009CD70):
 *   8005BCBC sw $a0, 0x358($gp)   D_8009D0C8 = a0          (ALWAYS first)
 *   8005BCC0 beqz $a0 → 8005BCE8  [slot: $v0 = 9]
 *   a0 != 0 path:
 *     8005BCC8/C lui/addiu $a1    a1 = 0x800C20A4
 *     8005BCD0 lbu $v1, 6($a0)    guest byte read at a0+6
 *     8005BCD8 bne $v1,$v0 → 8005BD00  [slot: $v0 = 8]
 *       byte6 == 9: j 8005BD00   [slot: a1 += 0x10 → 0x800C20B4]
 *       byte6 != 9: branch taken, a1 stays 0x800C20A4
 *   a0 == 0 path:
 *     8005BCE8 lw $v0, 0x4A8($gp) v0 = D_8009D218 (guest word read)
 *     8005BCEC/F lui/addiu $a1    a1 = 0x800C0DE0
 *     8005BCF4 beqz $v0 → 8005BD00  [slot: $v0 = 8]
 *       D_8009D218 != 0: falls through, a1 += 0x10 → 0x800C0DF0
 *       D_8009D218 == 0: branch taken, a1 stays 0x800C0DE0
 *   8005BD00 sw $a1, 0x350($gp)   D_8009D0C0 = a1
 *   8005BD04 sw $v0, 0x354($gp)   D_8009D0C4 = v0 (= 8 on every path)
 *   8005BD08 jr $ra; nop          returns $v0 = 8 on every path
 *
 * Signature: int func_8005BCBC(pe_addr_t a0).  a0 is a guest record
 * pointer or 0 (guest-address role; the only dereference is the byte at
 * a0+6).  Returns 8 unconditionally; neither call site consumes it.
 *
 * Call sites (exe-wide scan for jal word 0x0C016F2F; exactly two):
 *   1. func_800527C8 @0x80052834 — the bootstrap dispatcher; a0 = 0 in
 *      the jal delay slot, nop-like; return unconsumed; unconditional;
 *      one-shot per dispatcher invocation.  Immediately preceded by the
 *      jal func_80052C6C @0x8005282C, immediately followed by the jal
 *      func_8005D6F4 @0x8005283C.  D_8009D218 = 1 at entry (set by the
 *      dispatcher's func_8005BC98 call), so the dispatcher path stores
 *      D_8009D0C0 = 0x800C0DF0, D_8009D0C4 = 8, D_8009D0C8 = 0.
 *   2. func_8004DD64 @0x8004DF28 — a0 = lw 0x294($gp) (D_8009D004, an
 *      earlier func_8005332C result); nop delay slot; return unconsumed
 *      (next jal is func_8005BE1C).  Off the current boot frontier.
 *
 * Storage (split-brain audit): D_8009D0C0/C4/C8 are read and written by
 * several distinct retail functions (func_8005BD10 and func_8005BE1C
 * below in the same split; func_8005D6F4-region stores at 0x8005D730-38
 * and 0x8005D7F4-F8), all through $gp-relative guest accesses.  They are
 * therefore guest-RAM resident (PE_StoreU32), matching the B22 pattern
 * for shared $gp state.  The a0 == 0 path reads D_8009D218 from guest
 * RAM — the same word func_8005BC98 stores via PE_StoreU32.
 *
 * Idempotent: every call unconditionally overwrites all three words;
 * repeated calls do not accumulate.  PE_RamReset zeroes the three words
 * and D_8009D218, restoring initial conditions (a post-reset a0 == 0
 * call selects 0x800C0DE0 because the flag reads back 0).
 *
 * No callees, no SDK/Psy-Q/GTE/GPU/MDEC/SPU/disc/input activity, no
 * blocking.  Deterministic from guest state alone.
 *
 * Classification: 1 — translated retail logic.
 */
#include "psx_compat.h"

#define GA_D_8009D0C8  0x8009D0C8u   /* $gp+0x358: record pointer slot  */
#define GA_D_8009D0C0  0x8009D0C0u   /* $gp+0x350: selected buffer base */
#define GA_D_8009D0C4  0x8009D0C4u   /* $gp+0x354: selected count       */
#define GA_D_8009D218  0x8009D218u   /* $gp+0x4A8: func_8005BC98 flag   */

#define GA_BUF_REC_LO  0x800C20A4u   /* a0 != 0 base                    */
#define GA_BUF_NULL_LO 0x800C0DE0u   /* a0 == 0 base                    */

int func_8005BCBC(pe_addr_t a0)
{
    unsigned int v0 = 9u;      /* beqz delay slot; compared against byte6 */
    pe_addr_t a1;

    PE_StoreU32(GA_D_8009D0C8, a0);

    if (a0 != 0u) {
        a1 = GA_BUF_REC_LO;
        if (PE_LoadU8(a0 + 6u) == v0)
            a1 += 0x10u;       /* j delay slot: 0x800C20B4 */
        v0 = 8u;               /* bne delay slot */
    } else {
        v0 = PE_LoadU32(GA_D_8009D218);
        a1 = GA_BUF_NULL_LO;
        if (v0 != 0u)
            a1 += 0x10u;       /* fall-through: 0x800C0DF0 */
        v0 = 8u;               /* beqz delay slot */
    }

    PE_StoreU32(GA_D_8009D0C0, a1);
    PE_StoreU32(GA_D_8009D0C4, v0);
    return 8;
}
