/* func_800D0E88 (EXE 0x800D0E88, 0x4FC): shaded blade.  Native port of the
 * matched C in src/func_800D0E88.c (era_o2_g0, inline GTE asm, so the
 * generator skips it; same split as the func_800D1384 / func_800D2104 hand
 * ports).
 *   pos          SVECTOR centre, transformed by the camera (D_800BCFA4) with
 *                RT; MAC1-3 become the blade matrix translation
 *   ang          SVECTOR angles: func_80079754 builds the blade rotation,
 *                func_800786E4 composes it with the camera rotation
 *   len, width   the fan triangle (0,0,0), (-width/2,0,len), (width/2,0,len)
 *   c0, c1, c2   vertex colours (3 bytes; 0 = D_800C22A0), scaled by
 *                bright/128
 *   abr          255 = opaque; otherwise a draw-mode word (func_80077C84
 *                with GetTPage(0, abr, 0, 0)) and the semi-transparency bit.
 * One POLY_G3 (28 bytes) is taken from the packet arena up front.  As in
 * retail, nothing is linked when MAC0 after the RTPT is 0 or the depth
 * (SZ3 >> 2, read by gte_stszotz after AVSZ3) minus D_800F3374 is out of
 * range (>= 0x1000 unsigned). */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "pe_sdk.h"
#include "pe_guest_ram.h"

#define D0E88_LOC_M   0x00u   /* MATRIX m: short[3][3], pad, int t[3] */
#define D0E88_LOC_DEF 0x20u   /* B4 def = D_800C22A0 */
#define D0E88_LOC_SZ  0x28u

static void d0e88_link(pe_addr_t ot, pe_addr_t prim)
{
    PE_StoreU32(prim, (PE_LoadU32(prim) & 0xFF000000u) | (PE_LoadU32(ot) & 0xFFFFFFu));
    PE_StoreU32(ot, (PE_LoadU32(ot) & 0xFF000000u) | (prim & 0xFFFFFFu));
}

void func_800D0E88(pe_addr_t pos, pe_addr_t ang, int32_t len, int32_t width, pe_addr_t c0,
                   pe_addr_t c1, pe_addr_t c2, int16_t bright, int32_t abr)
{
    const pe_addr_t loc = PE_GuestFrame_Enter(D0E88_LOC_SZ);
    const pe_addr_t m = loc + D0E88_LOC_M, def = loc + D0E88_LOC_DEF;
    pe_addr_t c[3];
    uint32_t bank, cur, bias, xy[3], z[3], otz;
    int16_t angles[3];
    pe_addr_t p, ot;
    unsigned i, k;

    PE_StoreU32(def, PE_LoadU32(0x800C22A0u));
    bank = PE_LoadU32(0x8009CDDCu);
    cur = PE_LoadU32(0x8009CDD8u);
    p = PE_LoadU32(0x800B0E58u + bank * 4u) + cur;
    PE_StoreU32(0x8009CDD8u, cur + 28u);
    bias = PE_LoadU16(0x800F3374u);
    PE_GTE_LoadRT(PE_LoadU32(0x800BCFA4u));
    PE_GTE_SetV0((int16_t)PE_LoadU16(pos), (int16_t)PE_LoadU16(pos + 2u),
                 (int16_t)PE_LoadU16(pos + 4u));
    PE_GTE_MVMVA(0x480012u);                      /* gte_rt */
    for (k = 0; k < 3u; k++)                      /* gte_stlvnl(m.t) */
        PE_StoreU32(m + 20u + k * 4u, PE_GTE_ReadData(25u + k));
    for (k = 0; k < 3u; k++) angles[k] = (int16_t)PE_LoadU16(ang + k * 2u);
    PE_RotMatrix79754(angles, m);                 /* func_80079754(ang, &m) */
    func_800786E4(m);
    PE_GTE_LoadRT(m);
    PE_GTE_SetV0(0, 0, 0);
    PE_GTE_SetV1((int16_t)(-width / 2), 0, (int16_t)len);
    PE_GTE_SetV2((int16_t)(width / 2), 0, (int16_t)len);
    PE_GTE_RTPT_coordinates(xy, z);
    func_80077B84(p);
    c[0] = c0 ? c0 : def; c[1] = c1 ? c1 : def; c[2] = c2 ? c2 : def;
    for (i = 0; i < 3u; i++)
        for (k = 0; k < 3u; k++)
            PE_StoreU8(p + 4u + i * 8u + k,
                       (uint8_t)((int32_t)PE_LoadU8(c[i] + k) * bright / 128));
    if (PE_GTE_ReadData(24u) == 0u) {             /* gte_stopz: MAC0 */
        PE_GuestFrame_Leave(loc, D0E88_LOC_SZ);
        return;
    }
    for (i = 0; i < 3u; i++) PE_StoreU32(p + 8u + i * 8u, xy[i]);
    PE_GTE_AVSZ3(z);
    otz = (uint32_t)((int32_t)PE_GTE_ReadData(19u) >> 2) - bias;   /* gte_stszotz */
    if (otz >= 0x1000u) {
        PE_GuestFrame_Leave(loc, D0E88_LOC_SZ);
        return;
    }
    bank = PE_LoadU32(0x8009CDDCu);
    ot = PE_LoadU32(0x800B0E38u + bank * 4u) + otz * 4u;
    if (abr != 0xFF) {
        pe_addr_t dm;
        cur = PE_LoadU32(0x8009CDD8u);
        dm = PE_LoadU32(0x800B0E58u + bank * 4u) + cur;
        PE_StoreU32(0x8009CDD8u, cur + 8u);
        (void)func_80077C84(dm, 0u, 1u, (uint32_t)func_80077A64(0, abr, 0, 0) & 0xFFFFu);
        if (p != 0) {
            PE_StoreU8(p + 7u, PE_LoadU8(p + 7u) | 2u);
            d0e88_link(ot, p);
        }
        d0e88_link(ot, dm);
    } else if (p != 0) {
        d0e88_link(ot, p);
    }
    PE_GuestFrame_Leave(loc, D0E88_LOC_SZ);
}
