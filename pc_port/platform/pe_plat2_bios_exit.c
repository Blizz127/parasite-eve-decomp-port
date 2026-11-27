/* BIOS B(38h) exit (retail stub address 0x800719E4; bound through
 * pe_plat/bios_sys.h as func_800719E4) and the BIOS A(15h) strcat veneer
 * func_800719F4.
 *
 * Clean-room provenance.  Written from the public psx-spx kernel/BIOS
 * reference only: the "Kernel (BIOS)" function list (entry B(38h) exit and
 * entry A(15h) strcat, whose semantics are the C-library strcat: append src,
 * including its terminator, at the end of dst and return dst), plus the public
 * project headers pe_plat/bios_sys.h and pe_sdk.h and the suite assertion that
 * documents the host contract of B(38h): both argument forms (-1 and 1) return
 * 0 and leave guest RAM untouched.  That host contract is a silent no-op, not a
 * model of the BIOS shell exit; it is the contract stated
 * in REPORT-4 section 5 and asserted by the suite (the evidence file
 * docs/evidence/native-bios-sys-20261029/REPORT.md was not opened).  The author did not open the
 * retail executable, its disassembly or listings, src/, asm/, vendor/,
 * game/decomp_hand/, game/boot/, pe_lib*.c, *_retail.c, any oracle source,
 * retail_*_cases.h, evidence that quotes original code, or any earlier version
 * of the file this one replaces.  See
 * docs/evidence/native-cdx-20261126/cdrw-h/AUTHOR-REPORT.md. */
#include <stdint.h>
#include <string.h>

#include "pe_plat/bios_sys.h"
#include "pe_sdk.h" /* canonical declaration of func_800719F4 */

/* Host contract: the exit request is accepted and ignored; the caller sees 0. */
uint32_t pe_plat_bios_exit(uint32_t mode)
{
    (void)mode;
    return 0u;
}

/* A(15h) strcat over host C strings (the declaration is in pe_sdk.h). */
char *func_800719F4(char *destination, const char *source)
{
    return strcat(destination, source);
}
