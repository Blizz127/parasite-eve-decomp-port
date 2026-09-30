/*
 * Phase 6E-B16 — func_8006E498: archive directory lookup by 32-bit key.
 *
 * Raw body: 31 words / 0x7C, exe 0x8006E498–0x8006E513, file offset
 * 0x5EC98, sole live split asm/disc1/5B1E4.s:4264; all 31 instruction
 * words verified exact against the SHA-exact retail executable.
 *
 * Retail semantics (pure guest-memory table walk, no callees, no state):
 *   hdr   = lw(base + lw(base + 4) + 8)   — directory header word
 *   count = hdr >> 22                     — entry count, top 10 bits
 *   if (count == 0) return 0
 *   e = base + (hdr & 0x3FFFFF)           — first 0xC-byte entry
 *   for (i = 0; ; e += 0xC) {
 *       if (lw(e + 8) == key)             — key at entry +8
 *           return base + (lw(e + 4) & 0xFFFFFF);  — offset at entry +4
 *       if (++i >= count) break;          — t0 incremented in the
 *   }                                       bne delay slot (always)
 *   return 0
 *
 * The return value is a GUEST address (base + 24-bit offset); it is
 * kept as pe_addr_t end to end and is stored back into guest RAM by the
 * caller — never a host pointer.
 *
 * func_8006A9E4 calls it twice on the D_800E2858 archive copy with the
 * retail keys 0x57D40D84 and 0x57D41D84.
 *
 * Classification: 1 — translated retail logic.  Stateless; read-only;
 * repeated invocation with the same guest state returns the same result.
 */
#include "psx_compat.h"

pe_addr_t func_8006E498(pe_addr_t base, uint32_t key)
{
    uint32_t hdr   = PE_LoadU32(base + PE_LoadU32(base + 4) + 8);
    uint32_t count = hdr >> 22;
    pe_addr_t e;
    uint32_t  i;

    if (count == 0)
        return 0;
    e = base + (hdr & 0x3FFFFFu);
    for (i = 0; i < count; i++, e += 0xC) {
        if (PE_LoadU32(e + 8) == key)
            return base + (PE_LoadU32(e + 4) & 0xFFFFFFu);
    }
    return 0;
}
