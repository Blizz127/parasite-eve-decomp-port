/* func_800D2B58 (EXE 0x800D2B58, 0x5BC): streak.  Native port of the matched
 * C in src/func_800D2B58.c (era_o2_g0, inline GTE asm, so the generator skips
 * it; same split as the func_800D0E88 / func_800D1384 hand ports).
 *   p0, p1     SVECTORs projected with the current RT (RTPS each)
 *   c0, c1     3-byte colours; point 0 = c0 * b0/128, point 1 = c1 * b1/128
 *              except its blue channel, which retail scales by b0 (kept)
 *   abr        255 = opaque; otherwise a draw-mode word (func_80077C84 with
 *              GetTPage(0, abr, 0, 0)), the line's semi bit and code 0x3E
 * A LINE_G2 (0x14) and a POLY_GT4 (0x34) are taken from the packet arena up
 * front.  Depth = mean of the two SZ3>>2 minus D_800F3374; out of range
 * (>= 0x1000 unsigned) returns before anything is linked.  The quad is the
 * line widened by (DC4(a)/256, CF4(a)/256) along and (CF4(-a)/512,
 * DC4(-a)/512) across, a = ratan2(dy, dx); UVs 0x90..0x9F x 0xD0..0xDF, clut
 * (0x20, 0x1D6), tpage D_800E2852.  No pointer is NULL-checked: retail reads
 * c1 even when the caller passes 0 (guest RAM 0x80000000 here, as kuseg 0). */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "pe_sdk.h"

static void d2b58_link(pe_addr_t ot, pe_addr_t prim)
{
    PE_StoreU32(prim, (PE_LoadU32(prim) & 0xFF000000u) | (PE_LoadU32(ot) & 0xFFFFFFu));
    PE_StoreU32(ot, (PE_LoadU32(ot) & 0xFF000000u) | (prim & 0xFFFFFFu));
}

static void d2b58_project(pe_addr_t v, uint32_t *xy, int32_t *sz)
{
    uint32_t z;
    PE_GTE_SetV0((int16_t)PE_LoadU16(v), (int16_t)PE_LoadU16(v + 2u),
                 (int16_t)PE_LoadU16(v + 4u));
    PE_GTE_RTPS_coordinates(xy, &z);
    *sz = (int32_t)PE_GTE_ReadData(19u) >> 2;      /* gte_stszotz */
}

void func_800D2B58(pe_addr_t p0, pe_addr_t p1, pe_addr_t c0, pe_addr_t c1,
                   int32_t b0, int32_t b1, int32_t abr)
{
    uint32_t bank, cur, xy, otz;
    int32_t sz0, sz1, a, cs, sn, cl, sl, x0, y0, x1, y1;
    pe_addr_t line, quad, ot;
    unsigned k;

    bank = PE_LoadU32(0x8009CDDCu);
    cur = PE_LoadU32(0x8009CDD8u);
    PE_StoreU32(0x8009CDD8u, cur + 0x48u);
    line = PE_LoadU32(0x800B0E58u + bank * 4u) + cur;
    quad = line + 0x14u;

    d2b58_project(p0, &xy, &sz0);
    for (k = 0; k < 3u; k++)
        PE_StoreU8(line + 4u + k, (uint8_t)((int32_t)PE_LoadU8(c0 + k) * b0 / 128));
    PE_StoreU32(line + 8u, xy);
    PE_StoreU8(line + 3u, 4u);
    PE_StoreU8(line + 7u, 0x50u);
    d2b58_project(p1, &xy, &sz1);
    PE_StoreU8(line + 0xCu, (uint8_t)((int32_t)PE_LoadU8(c1) * b1 / 128));
    PE_StoreU8(line + 0xDu, (uint8_t)((int32_t)PE_LoadU8(c1 + 1u) * b1 / 128));
    PE_StoreU8(line + 0xEu, (uint8_t)((int32_t)PE_LoadU8(c1 + 2u) * b0 / 128));
    PE_StoreU32(line + 0x10u, xy);

    otz = (uint32_t)((sz0 + sz1) / 2 - (int32_t)PE_LoadU16(0x800F3374u));
    if (otz >= 0x1000u) return;
    bank = PE_LoadU32(0x8009CDDCu);
    ot = PE_LoadU32(0x800B0E38u + bank * 4u) + otz * 4u;
    if (abr != 0xFF) {
        pe_addr_t dm;
        cur = PE_LoadU32(0x8009CDD8u);
        dm = PE_LoadU32(0x800B0E58u + bank * 4u) + cur;
        PE_StoreU32(0x8009CDD8u, cur + 8u);
        (void)func_80077C84(dm, 0u, 1u, (uint32_t)func_80077A64(0, abr, 0, 0) & 0xFFFFu);
        PE_StoreU8(line + 7u, PE_LoadU8(line + 7u) | 2u);
        d2b58_link(ot, line);
        d2b58_link(ot, dm);
    } else {
        d2b58_link(ot, line);
    }

    x0 = (int16_t)PE_LoadU16(line + 8u);   y0 = (int16_t)PE_LoadU16(line + 0xAu);
    x1 = (int16_t)PE_LoadU16(line + 0x10u); y1 = (int16_t)PE_LoadU16(line + 0x12u);
    a = func_80079FB4(y1 - y0, x1 - x0);
    cl = func_80077CF4(-a) / 512;
    sl = func_80077DC4(-a) / 512;
    sn = func_80077DC4(a) / 256;
    cs = func_80077CF4(a) / 256;

    PE_StoreU8(quad + 3u, 0xCu);
    if (abr == 0xFF) {
        PE_StoreU8(quad + 7u, 0x3Cu);
        PE_StoreU16(quad + 0x1Au, PE_LoadU16(0x800E2852u));
    } else {
        PE_StoreU8(quad + 7u, 0x3Eu);
        PE_StoreU16(quad + 0x1Au, (uint16_t)(PE_LoadU16(0x800E2852u) |
                                             (uint16_t)func_80077A64(0, abr, 0, 0)));
    }
    PE_StoreU16(quad + 0xEu, (uint16_t)func_80077AA4(0x20, 0x1D6));
    PE_StoreU16(quad + 0x08u, (uint16_t)(x0 - sn + cl));
    PE_StoreU16(quad + 0x0Au, (uint16_t)(y0 - cs + sl));
    PE_StoreU16(quad + 0x14u, (uint16_t)(x0 - sn - cl));
    PE_StoreU16(quad + 0x16u, (uint16_t)(y0 - cs - sl));
    PE_StoreU16(quad + 0x20u, (uint16_t)(x1 + sn + cl));
    PE_StoreU16(quad + 0x22u, (uint16_t)(y1 + cs + sl));
    PE_StoreU16(quad + 0x2Cu, (uint16_t)(x1 + sn - cl));
    PE_StoreU16(quad + 0x2Eu, (uint16_t)(y1 + cs - sl));
    PE_StoreU8(quad + 0x0Cu, 0x90u); PE_StoreU8(quad + 0x0Du, 0xD0u);
    PE_StoreU8(quad + 0x18u, 0x9Fu); PE_StoreU8(quad + 0x19u, 0xD0u);
    PE_StoreU8(quad + 0x24u, 0x90u); PE_StoreU8(quad + 0x25u, 0xDFu);
    PE_StoreU8(quad + 0x30u, 0x9Fu); PE_StoreU8(quad + 0x31u, 0xDFu);
    for (k = 0; k < 3u; k++) {
        uint8_t ca = PE_LoadU8(line + 4u + k), cb = PE_LoadU8(line + 0xCu + k);
        PE_StoreU8(quad + 0x04u + k, ca);
        PE_StoreU8(quad + 0x10u + k, ca);
        PE_StoreU8(quad + 0x1Cu + k, cb);
        PE_StoreU8(quad + 0x28u + k, cb);
    }
    func_80077AC4(PE_LoadU32(0x800B0E38u + PE_LoadU32(0x8009CDDCu) * 4u) + otz * 4u, quad);
}
