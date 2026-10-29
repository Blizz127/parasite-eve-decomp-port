/* func_800D3BC8 (EXE 0x800D3BC8, 0x39C): camera-facing textured sprite, one
 * POLY_FT4.  Native port of the C candidate in
 * docs/evidence/func-800D3BC8/ (inline GTE asm; LINK residual is prologue
 * save scheduling only, see REPORT.md).
 *   pos        SVECTOR projected with the camera RT (D_800BCFA4, RTPS)
 *   sx, sy     4.12 scale of the D_800F3376 x D_800F3378 half extents
 *              (half-width = w * sx / 8192)
 *   cell       16x16 texture cell: u = (cell & 15) << 4, v = (cell / 16) << 4;
 *              with D_800F336E set, cells 8..15 of a row fold to u - 128,
 *              v + 32 (two 128-wide texture pages)
 *   clut       clut word as given
 *   abr        255 = opaque (code 0x2C); otherwise code 0x2E and the tpage
 *              gets GetTPage(0, abr, 0, 0)
 *   bright     colour = color[i] * bright / 128, or bright itself for NULL
 *   lerp       nonzero pins the depth to 14 and pulls the centre towards
 *              screen (160, 120): xy = xy*(4096-2*lerp) + c*(2*lerp) >> 12
 * The prim is taken from the packet arena up front (D_8009CDD8 += 0x28) and
 * linked at SZ3>>2 - D_800F3374 only when that is < 0x1000 unsigned. */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "pe_sdk.h"

#define D3BC8_LOC_SZ 0x18u   /* retail sp+0x18 centre, sp+0x20 xy, sp+0x28 otz */

void func_800D3BC8(pe_addr_t pos, int32_t sx, int32_t sy, int32_t cell, int32_t clut,
                   int32_t abr, int32_t bright, pe_addr_t color, int32_t lerp)
{
    const pe_addr_t loc = PE_GuestFrame_Enter(D3BC8_LOC_SZ);
    const pe_addr_t center = loc, xyp = loc + 8u;
    uint32_t bank, cur, xy, z;
    int32_t r, g, b, u, v, w, h, otz, x, y;
    uint16_t tpage;
    pe_addr_t p;

    bank = PE_LoadU32(0x8009CDDCu);
    cur = PE_LoadU32(0x8009CDD8u);
    PE_StoreU32(center, 0u);
    PE_StoreU32(center + 4u, 0u);
    PE_StoreU16(center, 160u);
    PE_StoreU16(center + 2u, 120u);
    PE_StoreU32(0x8009CDD8u, cur + 0x28u);
    p = PE_LoadU32(0x800B0E58u + bank * 4u) + cur;
    if (color == 0u) {
        r = g = b = bright;
    } else {
        r = (int32_t)PE_LoadU8(color) * bright / 128;
        g = (int32_t)PE_LoadU8(color + 1u) * bright / 128;
        b = (int32_t)PE_LoadU8(color + 2u) * bright / 128;
    }
    PE_StoreU8(p + 4u, (uint8_t)r);
    PE_StoreU8(p + 5u, (uint8_t)g);
    PE_StoreU8(p + 6u, (uint8_t)b);
    if (PE_LoadU16(0x800F336Eu) != 0u) {
        u = cell & 0xF;
        v = 0;
        if (u >= 8) {
            u -= 8;
            v = 32;
        }
        u <<= 4;
        v += (cell / 16) << 4;
    } else {
        u = (cell & 0xF) << 4;
        v = (cell / 16) << 4;
    }
    PE_GTE_LoadRT(PE_LoadU32(0x800BCFA4u));
    PE_GTE_SetV0((int16_t)PE_LoadU16(pos), (int16_t)PE_LoadU16(pos + 2u),
                 (int16_t)PE_LoadU16(pos + 4u));
    PE_GTE_RTPS_coordinates(&xy, &z);
    PE_StoreU8(p + 3u, 9u);
    if (abr == 0xFF) {
        PE_StoreU8(p + 7u, 0x2Cu);
        tpage = PE_LoadU16(0x800F3370u);
    } else {
        PE_StoreU8(p + 7u, 0x2Eu);
        tpage = (uint16_t)(PE_LoadU16(0x800F3370u) | (uint16_t)func_80077A64(0, abr, 0, 0));
    }
    PE_StoreU16(p + 0x16u, tpage);
    PE_StoreU16(p + 0x0Eu, (uint16_t)clut);
    w = PE_LoadU16(0x800F3376u);
    h = PE_LoadU16(0x800F3378u);
    PE_StoreU8(p + 0x1Cu, (uint8_t)u);                 /* u2 = u0 */
    PE_StoreU8(p + 0x0Cu, (uint8_t)u);
    PE_StoreU8(p + 0x0Du, (uint8_t)v);                 /* v0 = v1 */
    PE_StoreU8(p + 0x15u, (uint8_t)v);
    PE_StoreU8(p + 0x24u, (uint8_t)(u + w - 1));       /* u3 = u1 */
    PE_StoreU8(p + 0x14u, (uint8_t)(u + w - 1));
    PE_StoreU8(p + 0x25u, (uint8_t)(v + h - 1));       /* v3 = v2 */
    PE_StoreU8(p + 0x1Du, (uint8_t)(v + h - 1));
    w = w * sx / 8192;
    h = h * sy / 8192;
    otz = ((int32_t)PE_GTE_ReadData(19u) >> 2) - (int32_t)PE_LoadU16(0x800F3374u);
    if (lerp != 0)
        otz = 14;
    if ((uint32_t)otz < 0x1000u) {
        PE_StoreU32(xyp, xy);                          /* gte_stsxy */
        func_800783E4(xyp, center, 0x1000 - lerp * 2, lerp * 2, xyp);
        x = (int16_t)PE_LoadU16(xyp);
        y = (int16_t)PE_LoadU16(xyp + 2u);
        PE_StoreU16(p + 0x18u, (uint16_t)(x - w));     /* x2 = x0 */
        PE_StoreU16(p + 0x08u, (uint16_t)(x - w));
        PE_StoreU16(p + 0x12u, (uint16_t)(y - h));     /* y1 = y0 */
        PE_StoreU16(p + 0x0Au, (uint16_t)(y - h));
        PE_StoreU16(p + 0x20u, (uint16_t)(x + w));     /* x3 = x1 */
        PE_StoreU16(p + 0x10u, (uint16_t)(x + w));
        PE_StoreU16(p + 0x22u, (uint16_t)(y + h));     /* y3 = y2 */
        PE_StoreU16(p + 0x1Au, (uint16_t)(y + h));
        func_80077AC4(PE_LoadU32(0x800B0E38u + PE_LoadU32(0x8009CDDCu) * 4u) + (uint32_t)otz * 4u, p);
    }
    PE_GuestFrame_Leave(loc, D3BC8_LOC_SZ);
}
