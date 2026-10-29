/* func_800D2104 (EXE 0x800D2104, 0x26C; handwritten GTE wrapper, no matched
 * C): glow point sprite.  Written from the retail asm (2026-10-08).
 *   pos   SVECTOR (lwc2 VXY0/VZ0), projected with RTPS (sf=1, lm=0)
 *   color 3 bytes, each scaled by scale/128 (signed divide, R3000 rounding)
 *   abr   255 = opaque; otherwise a draw-mode word (func_80077C84 with
 *         GetTPage(0, abr, 0, 0)) is linked after the sprite and the
 *         sprite's semi-transparency bit (code |= 2) is set.
 * The 12-byte sprite packet comes from the frame's packet buffer
 * (D_8009CDD8 cursor, D_800B0E58[bank]) and func_80077C24 initialises it;
 * SXY2 goes to +8, and otz = (SZ3 >> 2) - D_800F3374 picks the OT slot
 * (D_800B0E38[bank]); points with otz >= 0x1000 (unsigned) are not drawn,
 * though the packet space stays allocated, as in retail. */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "pe_sdk.h"

void func_80077C24(pe_addr_t p);

void func_800D2104(pe_addr_t pos, pe_addr_t color, int32_t scale, int32_t abr)
{
    uint32_t bank = PE_LoadU32(0x8009CDDCu), cur = PE_LoadU32(0x8009CDD8u);
    uint32_t xy, z, otz;
    pe_addr_t pk, ot;
    unsigned i;

    pk = PE_LoadU32(0x800B0E58u + bank * 4u) + cur;
    PE_StoreU32(0x8009CDD8u, cur + 12u);
    PE_GTE_SetV0((int16_t)PE_LoadU16(pos), (int16_t)PE_LoadU16(pos + 2u),
                 (int16_t)PE_LoadU16(pos + 4u));
    PE_GTE_RTPS_coordinates(&xy, &z);
    func_80077C24(pk);
    for (i = 0; i < 3u; i++)
        PE_StoreU8(pk + 4u + i, (uint8_t)((int32_t)PE_LoadU8(color + i) * scale / 128));
    PE_StoreU32(pk + 8u, xy);
    otz = (uint32_t)((int32_t)z >> 2) - PE_LoadU16(0x800F3374u);
    if (otz >= 0x1000u)
        return;
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
