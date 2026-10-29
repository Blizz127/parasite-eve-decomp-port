/* func_800D1384 (EXE 0x800D1384, 0x75C; handwritten GTE wrapper, no matched
 * C): gradient trail ribbon.  Written from the retail asm (2026-10-08).
 *   head0, head1  the two new edge points (SVECTOR), or 0 to repeat the
 *                 previous ones
 *   n             segment count; n >= 1000 instead clears the "valid" flags
 *                 of the first n - 999 trail entries and draws nothing
 *   col0, col1    edge colours (3 bytes, or 0 = black), scaled by scale/128
 *   trail         n + 1 entries of { SVECTOR a; SVECTOR b; } whose pad
 *                 halves (+6, +14) are the valid flags
 *   abr           255 = opaque; otherwise each quad gets a draw-mode word
 *                 (func_80077C84 with GetTPage(0, abr, 0, 0)) and its
 *                 semi-transparency bit.
 * The trail shifts down one entry and entry 0 takes the new head (flags 1),
 * or the old entry 1 coordinates (flags kept).  Packet space for n POLY_G4
 * (36 bytes each) is taken up front.  RT/TR = the camera (D_800BCFA4).
 * Segment i draws when all four flags are set: RTPT on a[i], b[i], a[i+1]
 * and RTPS on b[i+1]; colours fade by 128 - i*128/n.  As in retail, the
 * whole call ends early (later segments undrawn) when MAC0 after the RTPT
 * is 0 or a segment's otz = (SZ3 >> 2) - D_800F3374 is out of range
 * (>= 0x1000 unsigned); retail runs AVSZ3 but links by SZ3. */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "pe_sdk.h"

static void d1384_v(unsigned k, pe_addr_t v)
{
    const int16_t x = (int16_t)PE_LoadU16(v), y = (int16_t)PE_LoadU16(v + 2u),
                  z = (int16_t)PE_LoadU16(v + 4u);
    if (k == 0u) PE_GTE_SetV0(x, y, z);
    else if (k == 1u) PE_GTE_SetV1(x, y, z);
    else PE_GTE_SetV2(x, y, z);
}

static uint8_t d1384_scale(uint32_t c, int32_t s)
{
    return (uint8_t)((int32_t)c * s / 128);
}

void func_800D1384(pe_addr_t head0, pe_addr_t head1, uint32_t n, pe_addr_t col0,
                   pe_addr_t col1, int32_t scale, pe_addr_t trail, int32_t abr)
{
    const pe_addr_t camera = PE_LoadU32(0x800BCFA4u);
    const uint32_t bias = PE_LoadU16(0x800F3374u);
    uint8_t c0[3] = { 0, 0, 0 }, c1[3] = { 0, 0, 0 };
    uint32_t i, k, bank, cur;
    pe_addr_t pk;

    if (n >= 1000u) {
        for (i = 0; i < n - 999u; i++) {
            PE_StoreU16(trail + i * 16u + 6u, 0u);
            PE_StoreU16(trail + i * 16u + 14u, 0u);
        }
        return;
    }
    for (i = n; i-- > 0u;)
        for (k = 0; k < 16u; k += 4u)
            PE_StoreU32(trail + (i + 1u) * 16u + k, PE_LoadU32(trail + i * 16u + k));
    if (head0 != 0) {
        for (k = 0; k < 6u; k += 2u) PE_StoreU16(trail + k, PE_LoadU16(head0 + k));
        PE_StoreU16(trail + 6u, 1u);
        for (k = 0; k < 6u; k += 2u) PE_StoreU16(trail + 8u + k, PE_LoadU16(head1 + k));
        PE_StoreU16(trail + 14u, 1u);
    } else {
        for (k = 0; k < 6u; k += 2u) {
            PE_StoreU16(trail + k, PE_LoadU16(trail + 16u + k));
            PE_StoreU16(trail + 8u + k, PE_LoadU16(trail + 24u + k));
        }
    }
    bank = PE_LoadU32(0x8009CDDCu);
    cur = PE_LoadU32(0x8009CDD8u);
    PE_StoreU32(0x8009CDD8u, cur + n * 36u);
    pk = PE_LoadU32(0x800B0E58u + bank * 4u) + cur;
    if (col0 != 0)
        for (k = 0; k < 3u; k++) c0[k] = d1384_scale(PE_LoadU8(col0 + k), scale);
    if (col1 != 0)
        for (k = 0; k < 3u; k++) c1[k] = d1384_scale(PE_LoadU8(col1 + k), scale);
    PE_GTE_LoadRT(camera);
    for (i = 0; i < n; i++, pk += 36u) {
        const pe_addr_t e = trail + i * 16u;
        uint32_t xy[3], z[3], xy3, z3, otz;
        int16_t t0, t1;
        pe_addr_t ot;
        if (!(PE_LoadU16(e + 6u) & PE_LoadU16(e + 14u) & PE_LoadU16(e + 22u) & PE_LoadU16(e + 30u)))
            continue;
        d1384_v(0u, e); d1384_v(1u, e + 8u); d1384_v(2u, e + 16u);
        PE_GTE_RTPT_coordinates(xy, z);
        if (g_pe_gte.mac0 == 0)
            return;
        PE_StoreU32(pk + 8u, xy[0]);
        PE_StoreU32(pk + 16u, xy[1]);
        PE_StoreU32(pk + 24u, xy[2]);
        PE_GTE_AVSZ3(z);
        otz = (uint32_t)((int32_t)g_pe_gte.sz[3] >> 2) - bias;
        if (otz >= 0x1000u)
            return;
        d1384_v(0u, e + 24u);
        PE_GTE_RTPS_coordinates(&xy3, &z3);
        func_80077BC4(pk);
        PE_StoreU32(pk + 32u, xy3);
        t0 = (int16_t)(128u - (i << 7) / n);
        t1 = (int16_t)(128u - ((i + 1u) << 7) / n);
        for (k = 0; k < 3u; k++) {
            PE_StoreU8(pk + 4u + k, d1384_scale(c0[k], t0));
            PE_StoreU8(pk + 12u + k, d1384_scale(c1[k], t0));
            PE_StoreU8(pk + 20u + k, d1384_scale(c0[k], t1));
            PE_StoreU8(pk + 28u + k, d1384_scale(c1[k], t1));
        }
        bank = PE_LoadU32(0x8009CDDCu);
        ot = PE_LoadU32(0x800B0E38u + bank * 4u) + otz * 4u;
        if (abr != 255) {
            pe_addr_t dm;
            cur = PE_LoadU32(0x8009CDD8u);
            PE_StoreU32(0x8009CDD8u, cur + 8u);
            dm = PE_LoadU32(0x800B0E58u + bank * 4u) + cur;
            (void)func_80077C84(dm, 0u, 1u, (uint32_t)func_80077A64(0, abr, 0, 0) & 0xFFFFu);
            if (pk != 0) {
                PE_StoreU8(pk + 7u, PE_LoadU8(pk + 7u) | 2u);
                PE_StoreU32(pk, (PE_LoadU32(pk) & 0xFF000000u) | (PE_LoadU32(ot) & 0xFFFFFFu));
                PE_StoreU32(ot, (PE_LoadU32(ot) & 0xFF000000u) | (pk & 0xFFFFFFu));
            }
            PE_StoreU32(dm, (PE_LoadU32(dm) & 0xFF000000u) | (PE_LoadU32(ot) & 0xFFFFFFu));
            PE_StoreU32(ot, (PE_LoadU32(ot) & 0xFF000000u) | (dm & 0xFFFFFFu));
        } else if (pk != 0) {
            PE_StoreU32(pk, (PE_LoadU32(pk) & 0xFF000000u) | (PE_LoadU32(ot) & 0xFFFFFFu));
            PE_StoreU32(ot, (PE_LoadU32(ot) & 0xFF000000u) | (pk & 0xFFFFFFu));
        }
    }
}
