/*
 * Phase 6E-B21a — func_80071A24: BIOS A(28h) bzero trampoline.
 *
 * Raw body: 3 words / 0xC, exe 0x80071A24–0x80071A2F, file 0x62224,
 * live split asm/disc1/5F3E4.s:3436–3440; all 3 instruction words
 * verified exact against the SHA-exact retail executable.
 *
 * BIOS operation:
 *   addiu $t2, $zero, 0x00A0   → BIOS A-function table vector
 *   jr $t2                      → enter kernel
 *   addiu $t1, $zero, 0x0028   → A(28h) = bzero(dst, len)
 *
 * bzero(dst, len): zeroes len bytes starting at guest address dst and
 * returns the incoming destination (the A(2Bh) memset-family convention).
 *
 * B21 call site: func_80064964 @0x80064974 ($a0=0x800A3060,
 * $a1=0x120, addiu delay slot).  Clears the 288-byte region at
 * POOL_END before func_80064964 sets eight flag bytes to 0xFF.  The
 * executable-wide scan has additional calls in unrelated later paths.
 *
 * Classification: 2 — known BIOS memory operation requiring a
 * checked guest-memory adaptation.
 */
#include "psx_compat.h"

pe_addr_t pe_plat_bios_bzero(pe_addr_t dst, uint32_t len)
{
    /* psx-spx "BIOS Memory Fill/Copy/Compare": A(28h) bzero is "Same as
     * memset, but uses 00h as fixed fillbyte value", and memset "Refuses to
     * fill memory when dst=00000000h or when len>7FFFFFFFh. The return value
     * is the incoming 'dst' value (or zero, when len=0 or len>7FFFFFFFh)."
     * (see pe_bios_string.h). */
    if (len == 0u || len > 0x7FFFFFFFu)
        return 0u;
    if (dst == 0u)
        return dst;
    PE_Fill(dst, len, 0);
    return dst;
}
