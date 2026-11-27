/* BIOS A(28h) bzero(dst, len) over guest memory (retail stub address
 * 0x80071A24; bound through pe_plat/bios_string.h as func_80071A24).
 *
 * Clean-room provenance.  Written from the public psx-spx kernel/BIOS
 * reference only: the section "BIOS Memory Fill/Copy/Compare" (A(28h) bzero is
 * the same as A(2Bh) memset with a fixed fill byte of 00h; the memset family
 * returns the incoming dst value, or zero when len = 0 or len > 7FFFFFFFh, and
 * refuses a null dst), plus the public project headers pe_plat/bios_string.h,
 * pe_guest_ram.h and psx_compat.h and the suite tests that call this entry
 * (test_native.c 80071A24_*, test_port_absent.h).  The author did not open the
 * retail executable, its disassembly or listings, src/, asm/, vendor/,
 * game/decomp_hand/, game/boot/, pe_lib*.c, *_retail.c, any oracle source,
 * retail_*_cases.h, evidence that quotes original code, or any earlier version
 * of the file this one replaces.  See
 * docs/evidence/native-cdx-20261126/cdrw-h/AUTHOR-REPORT.md. */
#include <stdint.h>

#include "pe_guest_ram.h"
#include "pe_plat/bios_string.h"

#define PE_BZERO_MAX_LEN 0x7FFFFFFFu

pe_plat_guest_addr pe_plat_bios_bzero(pe_plat_guest_addr dst, uint32_t n)
{
    /* No-op (return 0): empty request, null destination, or a length that
     * has the sign bit set. */
    if (n == 0u || dst == 0u || n > PE_BZERO_MAX_LEN)
        return 0u;
    /* Checked fill: a range outside guest RAM aborts loudly. */
    PE_Fill(dst, n, 0u);
    return dst;
}
