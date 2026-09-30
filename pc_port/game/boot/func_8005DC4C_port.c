/*
 * Phase 6E-B26 — func_8005DC4C: PE.IMG message/string-table lookup.
 *
 * Raw body: 20 words / 0x50, executable 0x8005DC4C..0x8005DC9B
 * (exclusive end 0x8005DC9C = func_8005DC9C), file offset 0x4E44C, live
 * split asm/disc1/4CC98.s:1779-1802 (yaml segment [0x4CC98, asm],
 * exclusive end [0x4E914, c, func_8005E114]).  Exactly one body exists
 * in the tree: `grep -rn "glabel func_8005DC4C" asm/` returns that one
 * line, there is no matching or nonmatching C source, and the yaml
 * selects the asm segment (func_8005DC4C is not a C leaf).  All 20 words
 * verified exact against the SHA-exact retail executable (SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b).
 *
 * ROM-order operation map.  The splat symbols D_800A802C / D_800A8028
 * already carry the sign-extension: `lui 0x800B` + a 16-bit immediate
 * with bit 15 set means the real accesses are 0x800A802C / 0x800A8028,
 * NOT 0x800B802C / 0x800B8028 (same addressing-mode finding as B23's
 * func_8005DB44).
 *
 *   addr       raw       insn                     operation
 *   8005DC4C xxxxxxxx  lui   $v0,0x800B          -
 *   8005DC50 xxxxxxxx  lw    $v0,%lo(D_800A802C)($v0)
 *                                                READ32 @0x800A802C = R
 *   8005DC54 xxxxxxxx  lui   $v1,0x800B          -
 *   8005DC58 xxxxxxxx  addiu $v1,$v1,%lo(D_800A8028)
 *                                                $v1 = 0x800A8028 = HDR
 *   8005DC5C xxxxxxxx  addu  $v0,$v0,$v1         ptr = R + HDR   (u32 wrap)
 *   8005DC60 xxxxxxxx  lw    $v1,4($v0)          READ32 @ptr+4 = S
 *   8005DC64 xxxxxxxx  nop                       (load delay)
 *   8005DC68 xxxxxxxx  addu  $v1,$v0,$v1         tbl = ptr + S   (u32 wrap)
 *   8005DC6C xxxxxxxx  lhu   $v0,0($v1)          READ16 @tbl = count (zero-ext)
 *   8005DC70 xxxxxxxx  nop                       (load delay)
 *   8005DC74 xxxxxxxx  sltu  $v0,$a0,$v0         in = (idx <u count)
 *   8005DC78 xxxxxxxx  beqz  $v0,0x8005DC90      out of range -> return 0
 *   8005DC7C xxxxxxxx  sll   $v0,$a0,1           (delay) idx*2  — STRIDE 2
 *   8005DC80 xxxxxxxx  addu  $v0,$v0,$v1         tbl + idx*2
 *   8005DC84 xxxxxxxx  lh    $v0,2($v0)          READ16 @tbl+2+idx*2, SIGNED
 *   8005DC88 xxxxxxxx  j     0x8005DC94          -
 *   8005DC8C xxxxxxxx  addu  $v0,$v1,$v0         (delay) ret = tbl + off
 *   8005DC90 xxxxxxxx  addu  $v0,$zero,$zero     ret = 0
 *   8005DC94 xxxxxxxx  jr    $ra                 -
 *   8005DC98 xxxxxxxx  nop                       -
 *
 * Branch/jump accounting (complete): one conditional branch (beqz
 * @0x8005DC78, delay slot `sll $v0,$a0,1` — executed on BOTH paths,
 * exactly once, its result dead on the taken path), one unconditional
 * jump (j @0x8005DC88, delay slot `addu $v0,$v1,$v0` = the return
 * value), one `jr $ra` with a nop slot.  No calls of any kind.
 *
 * Signature: pe_addr_t func_8005DC4C(uint32_t idx).  ONE argument.  No
 * instruction in the body reads $a1 ($5) or any register other than
 * $a0 — verified by decoding all 20 words.  Retail call sites do leave
 * varying values in $a1 (in func_8005D6F4 the residual 0xFF fill
 * constant; elsewhere unrelated values, and at 0x8004685C the jal delay
 * slot is `sb $v0,0($s1)` — not argument setup at all), so $a1 is NOT a
 * parameter and is deliberately absent from this signature.
 *
 * Structure (a nested relative-offset PE.IMG archive):
 *   0x800A8028  HDR   — cycle-A streaming destination (func_8006A9E4)
 *   HDR+4       R     — relative offset to the sub-chunk: ptr = HDR + R
 *   ptr+4       S     — relative offset to this table: tbl = ptr + S
 *   tbl+0       u16   — entry count
 *   tbl+2+2*i   s16   — SIGNED relative offset of record i from tbl
 * The twin func_8005DC9C is the same routine reading ptr+8 instead of
 * ptr+4, i.e. a second table in the same sub-chunk.
 *
 * Return value: the guest address of a 0xFF-terminated record, computed
 * as tbl + sext16(entry[idx]) with exact 32-bit wraparound; or 0 when
 * idx >= count.  Zero is retail's own failure encoding (`addu $v0,$zero,
 * $zero`), not a sentinel invented here.  No call site anywhere in the
 * executable tests the return against zero — all 39 sites move it
 * straight into a register or store it (see the call-site census below)
 * — so an out-of-range index is a caller bug on hardware too, and this
 * port must not paper over it: a 0 return flows to the caller unchanged
 * and any dereference of it fails the checked-access guard.
 *
 * Observed real-disc contract (Disc 1 USA, measured in guest RAM at the
 * func_8005D6F4 call, not taken from port output): R = 0x30 so ptr =
 * 0x800A8058; S = 0x14 so tbl = 0x800A806C; count = 0x78 (120); the
 * entry values span 242..1971, so returns span 0x800A815E..0x800A881F.
 * min(entry) == 242 == 2 + 120*2 exactly — the record pool begins where
 * the offset table ends, which independently confirms the stride-2
 * layout.  idx 0x1E -> entry 573 -> 0x800A82A9, whose bytes are
 * `10 48 30 FF`: a 3-byte record plus the 0xFF terminator the caller's
 * copy loop stops on.
 *
 * Reads/writes: exactly three guest reads on the failure path (32 @
 * 0x800A802C, 32 @ ptr+4, 16 @ tbl) and four on the success path (plus
 * 16 @ tbl+2+idx*2).  ZERO guest writes, zero callees, no SDK/Psy-Q,
 * GTE, GPU, ordering-table, MDEC, DMA, SPU, controller, disc, event or
 * interrupt activity, no status polling, no executable/rodata reads.
 * Cannot block.  Deterministic from guest state alone; repeated calls
 * with the same guest state return the same address (stable, never
 * advancing).  It owns no state, so first-call, repeated-call and
 * post-PE_RamReset behaviour differ only through the archive contents.
 *
 * Call sites (exe-wide scan for jal word 0x0C017713): 39 sites in 24
 * distinct callers — func_800447F0, func_80044F8C, func_800466C0 (x2),
 * func_80046C20, func_8004790C, func_800494AC, func_8004B970 (x3),
 * func_8004BCB4, func_8004C520, func_8004C608 (x5), func_8004CC50 (x2),
 * func_8004CE28 (x2), func_8004D6D4, func_8004DAA4 (x2), func_8004E074,
 * func_800500A8, func_80050878, func_80054A88 (x4), func_8005BE1C,
 * func_8005D6F4 (x3), func_8005F5B8, func_800602D0, func_80062A7C,
 * func_80064C54.  Observed constant indices run 3..117, all below the
 * measured count of 120.  Every return consumer is a register move
 * (`addu $aX/$sX,$v0,$zero`) or one store (`sw $v0,484($gp)` at
 * func_800500A8 @0x800500DC); none dereferences $v0 in place and none
 * compares it to zero.  Only the three func_8005D6F4 sites are on the
 * current boot frontier.
 *
 * Classification: 1 — translated retail logic.
 */
#include "psx_compat.h"

#define GA_5DC4C_HDR      0x800A8028u  /* lui 0x800B + sext(0x8028) */
#define GA_5DC4C_HDR_REL  0x800A802Cu  /* lui 0x800B + sext(0x802C) */

pe_addr_t func_8005DC4C(uint32_t idx)
{
    /* ptr = mem32[HDR+4] + HDR, exact 32-bit wraparound. */
    pe_addr_t ptr = (pe_addr_t)(PE_LoadU32(GA_5DC4C_HDR_REL) + GA_5DC4C_HDR);
    /* tbl = ptr + mem32[ptr+4], exact 32-bit wraparound. */
    pe_addr_t tbl = (pe_addr_t)(ptr + PE_LoadU32(ptr + 4u));
    /* sltu against the zero-extended halfword count: unsigned compare. */
    uint32_t count = PE_LoadU16(tbl);

    if (!(idx < count))
        return 0u;                          /* addu $v0,$zero,$zero */

    /* sll $v0,$a0,1 then lh 2($v0): stride 2, signed 16-bit offset. */
    uint32_t off = (uint32_t)(int32_t)(int16_t)
                   PE_LoadU16(tbl + 2u + idx * 2u);
    return (pe_addr_t)(tbl + off);          /* addu $v0,$v1,$v0 */
}
