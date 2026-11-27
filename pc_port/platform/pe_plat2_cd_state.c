/* Host CD state helpers: XA service query, disc file presence, Setloc telemetry word.
 *
 * Provenance: clean-room, written from public contracts only.  Sources consulted:
 *   - platform/pe_sdk.h (comments and prototypes of PE_Cd_XaServiceAvailable, PE_Cd_FilePresent,
 *     PE_Cd_GetSetlocRaw) and include/pe_plat/cd.h (pe_plat_cd_store_setloc_hint prototype);
 *   - include/pe_plat/disc_image.h (PE_Disc_FindFile / active-disc slot API), platform/pe_guest_ram.h;
 *   - the tests that exercise the entries (test_credits_native.h credits_entry_gates,
 *     test_native.c B54KAL_blocking_setloc_arm, test_cd_{command_wait,reset,public_init}_native.h);
 *   - the clean-room adapters platform/pe_plat_cd_{command_wait,public_init,reset}_host.c (callers of the
 *     setter), platform/pe_plat_cd_search_file_host.c (path-check pattern), and the header comments of
 *     tools/pe_cdx_cdgen49_trace.c.
 * Written without reading the file it replaces. */
#include <stdint.h>
#include <string.h>
#include "pe_sdk.h"
#include "pe_disc.h"

/* The last CdlLOC the host proved valid (4 raw bytes as a little-endian word); 0 = none/disarmed. */
static uint32_t s_setloc_raw;

void pe_plat_cd_store_setloc_hint(uint32_t raw) { s_setloc_raw = raw; }

uint32_t PE_Cd_GetSetlocRaw(void) { return s_setloc_raw; }

/* The host CD provider always serves the XA commands (Setfilter, ReadS, GetlocP, Stop) through the libcd
 * queue; there is no build in which it is absent. */
int PE_Cd_XaServiceAvailable(void) { return 1; }

/* 1 when the active disc holds the file the NUL-terminated guest string names, 0 with no disc, a
 * non-RAM or unterminated name, or a missing file. */
int PE_Cd_FilePresent(pe_addr_t guest_name)
{
    char path[256];
    uint32_t lba = 0, size = 0;
    size_t i;
    PE_Disc *disc = PE_Disc_GetActive();
    if (!disc) return 0;
    for (i = 0; i < sizeof path; ++i) {
        if (!PE_RangeIsRam(guest_name + (uint32_t)i, 1u)) return 0;
        path[i] = (char)PE_LoadU8(guest_name + (uint32_t)i);
        if (!path[i]) break;
    }
    if (i == sizeof path || path[0] != '\\') return 0;
    return PE_Disc_FindFile(disc, path, &lba, &size, NULL, 0) ? 1 : 0;
}
