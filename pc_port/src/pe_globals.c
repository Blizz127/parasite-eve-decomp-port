/* Phase 6D-S — PE global variables backed by contiguous 2 MiB guest RAM.
 *
 * Symbols that name bytes inside guest RAM (D_800B0CD8 block, D_80094488,
 * D_8009448C, D_800BCE80, ...) are typed lvalue macros in psx_compat.h and
 * have NO host definition here — one store, no split-brain.  Only
 * host-owned scalars and pe_addr_t guest-address holders live here. */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "pe_guest_ram.h"
#include <stddef.h>

/* ── Host-owned scalar globals (plain data, no pointer arithmetic) ──── */


/* D_800A7918: guest-memory macro (psx_compat.h). */
unsigned short D_80093164[4] = {0};

/* D_8009D018: guest-memory macro (psx_compat.h); reset by PE_Sdk_ResetState. */

/* ── D_80011614 — retail guest pointer, held host-side as pe_addr_t ────
 *
 * The initial value is bootstrap-fixture policy: the D_8010BD00 arena base.
 * Real-disc startup replaces it from authenticated executable rodata via
 * PE_Globals_AdoptRetailImage. Real access sites translate via PE_Translate. */
pe_addr_t D_80011614 = 0x8010BD00u;

/* Adopt the image-backed authorities only after PE_GuestImage_LoadExe has
 * authenticated and copied the retail executable.  Bootstrap fixtures keep
 * the host defaults above, so they do not pretend to contain PE.IMG. */
int PE_Globals_AdoptRetailImage(void)
{
    pe_addr_t dest = PE_LoadU32(0x80011614u);
    uint16_t start = PE_LoadU16(0x80093164u);
    uint16_t end = PE_LoadU16(0x80093166u);
    uint32_t bytes;
    unsigned int i;

    if (end < start)
        return -1;
    bytes = (uint32_t)(end - start) * 0x800u;
    if (!PE_RangeIsRam(dest, bytes))
        return -1;

    D_80011614 = dest;
    for (i = 0; i < 4; i++)
        D_80093164[i] = PE_LoadU16(0x80093164u + i * 2u);
    return 0;
}

/* ── Arena pointer globals ────────────────────────────────────────────
 * Phase 6E-B16: the 19-slot table is now guest-RAM lvalue macros in
 * pe_port_compat.h (split-brain fix); no host storage remains. */
