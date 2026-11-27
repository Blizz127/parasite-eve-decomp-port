/* Host adapter for BIOS B(38h), exit. The current implementation returns
 * without terminating; that fidelity gap and the movie-fixture conflicts
 * from a stop-on-exit trial remain open. See
 * docs/evidence/native-bios-sys-20261029/REPORT.md. */
#include "psx_compat.h"
#include "pe_port_compat.h"

#include <string.h>

uint32_t pe_plat_bios_exit(uint32_t mode)
{
    /* B(38h) CD mode set: BIOS-internal only; no guest-RAM effect.
     * Return value unconsumed by all twelve retail callers. */
    (void)mode;
    return 0;
}

/* Retail trampoline 0x800719F4 is BIOS A(15h), strcat. Both current callers
 * pass native stack buffers after guest pointers have been range-translated. */
char *func_800719F4(char *destination, const char *source)
{
    return strcat(destination, source);
}
