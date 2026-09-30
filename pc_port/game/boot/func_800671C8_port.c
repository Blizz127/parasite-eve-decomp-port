/*
 * PE-CH2 — func_800671C8: 56-byte record pan clamp (opcode 0x71 callee).
 *
 * 51 words 0x800671C8..0x80067294, SHA-256
 * bf67bc656e6a9c3ad4144ee1a15ae73d0ad32d2e3ded4620bae313c346f4f712.
 * Sole TEXT jal is opcode handler func_80018A9C @ 0x80018AE8.
 *
 *   X = (int16)(lhu(rec+8)  + x)
 *   Y = (int16)(lhu(rec+0xA) + y)
 *   X = clamp(X, (int16)rec+0x10, (int16)rec+0x12)
 *   Y = clamp(Y, (int16)rec+0x14, (int16)rec+0x16)
 *   page = (page + (lw(rec) >> 20)) & 0xFFF
 *   sh rec+0xC = X ; sh rec+0xE = Y
 *   lw rec = (lw(rec) & 0xFFF000FF) | (page << 8)
 *
 * The two clamps are the retail `slt`/branch order: the *minimum* arm
 * wins first, otherwise the *maximum* arm. Do not reorder.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

int func_800671C8(pe_addr_t rec, int32_t x, int32_t y, int32_t page)
{
    int32_t px = (int32_t)(int16_t)(PE_LoadU16(rec + 8u) + (uint16_t)x);
    int32_t py = (int32_t)(int16_t)(PE_LoadU16(rec + 0xAu) + (uint16_t)y);
    int32_t lo;
    uint32_t packed;

    lo = (int32_t)(int16_t)PE_LoadU16(rec + 0x10u);
    if (px < lo)
        px = lo;
    else {
        lo = (int32_t)(int16_t)PE_LoadU16(rec + 0x12u);
        if (lo < px)
            px = lo;
    }
    lo = (int32_t)(int16_t)PE_LoadU16(rec + 0x14u);
    if (py < lo)
        py = lo;
    else {
        lo = (int32_t)(int16_t)PE_LoadU16(rec + 0x16u);
        if (lo < py)
            py = lo;
    }

    packed = PE_LoadU32(rec);
    PE_StoreU16(rec + 0xCu, (uint16_t)px);
    PE_StoreU16(rec + 0xEu, (uint16_t)py);
    packed = (packed & 0xFFF000FFu)
           | (((uint32_t)page + (packed >> 20)) & 0xFFFu) << 8;
    PE_StoreU32(rec, packed);
    return 0;
}
