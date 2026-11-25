/*
 * pe_plat_ds_gen30.c -- libds "text_360" (0x8007E8F4), pending-command
 * dispatch predicate.
 *
 * Independent clean-room re-implementation, written from black-box vectors
 * only (batch cdgen30, vectors.v1.txt, sha256
 * 7b3f072a859bdeedf5e1f7908c609857f46091b537f4ae6e480ea758b0cbacab,
 * 6000 cases; also checked unchanged against vectors.v2.txt, sha256
 * 5d2dcb2b1c33440cd083e18844196e9fadd8e52de677c915124a2a7b7bcd5e2d, whose
 * high-half-only callee returns decide the full-word test in rule 4 —
 * integrator note).  Nothing was copied from any other implementation of this
 * function or from any decompilation, disassembly or other source.
 *
 * Behaviour (as observed):
 *   1. Call the status-lane getter (func_8007FBF0) with argument 0.  Unless
 *      its full 32-bit result equals 1, return 0 without touching guest RAM
 *      and without any further call.
 *   2. Otherwise read the 32-bit queue index at 0x800A3604 and form the
 *      record address 0x800A3540 + 24 * index (32-bit wrap-around; the index
 *      word is used as a plain 32-bit value, so 8 puts the record on top of
 *      the index word and 0xFFFFFFF8 wraps to a record below 0x800A3540).
 *   3. Read the record's first word (32-bit).  If it is zero, return 0 with
 *      no further call.
 *   4. Otherwise read the unsigned byte at record+4 and the 32-bit word at
 *      record+0xC and call func_8007FB44(byte, word).  Return 1 if its
 *      32-bit result is non-zero (any sign, any bit pattern), else 0.
 *   No guest memory is written.
 */
#include <stdint.h>
#include "pe_plat/cd.h"
#include "pe_guest_ram.h"

int pe_plat_ds_text_360(void)
{
    uint32_t rec;
    uint32_t cmd;
    uint32_t data;

    if (func_8007FBF0(0) != 1)
        return 0;
    rec = 0x800A3540u + 24u * PE_LoadU32(0x800A3604u);
    if (PE_LoadU32(rec) == 0u)
        return 0;
    cmd = PE_LoadU8(rec + 4u);
    data = PE_LoadU32(rec + 0xCu);
    return func_8007FB44(cmd, data) != 0 ? 1 : 0;
}
