/* func_800C5A40 (engine B6240.s, 0x800C5A40..0x800C5EB0): chain/tail actor
 * segment renderer (Day 2 Central Park random battles, m0075i/m0080i via
 * func_80190AF4), the draw half of func_800C5538.  NON_MATCHING host port
 * written from the retail asm.
 *
 * For each of h(arg0+4)-1 segment pairs: the quad between segment i's two
 * side points (chain+68i+20/+28) and segment i+1's (+88/+96) goes through
 * the camera D_800BCFA4 with RotTransPers4 (func_80079304: RTPT v0..v2 +
 * RTPS v3, otz = SZ3>>2) into a POLY_FT4 (code 0x2C, 0x2E semi-transparent
 * when D_800F337A) whose UVs come from the texture-page byte arg0+16 plus
 * the per-segment offset (chain+68i+3), colour from func_800C608C with the
 * segment style, CLUT from func_80077AA4; it is linked at OT[otz] when
 * 0 < otz < 4096 and the segment is visible (chain+68i byte 0). */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "pe_sdk.h"
#include "pe_guest_ram.h"

static void set_v(int which, pe_addr_t v)
{
    int16_t x = (int16_t)PE_LoadU16(v), y = (int16_t)PE_LoadU16(v + 2u), z = (int16_t)PE_LoadU16(v + 4u);
    if (which == 0) PE_GTE_SetV0(x, y, z);
    else if (which == 1) PE_GTE_SetV1(x, y, z);
    else PE_GTE_SetV2(x, y, z);
}

void func_800C5A40(pe_addr_t a0)
{
    pe_addr_t chain = PE_LoadU32(a0);
    pe_addr_t sp = PE_GuestFrame_Enter(128u);
    pe_addr_t scr, pkt, seg, cam;
    uint32_t i, n, bank, otz, xy[3], z[3], xy3, z3;
    uint8_t tp;
    int k;

    PE_StoreU32(0x800F33B4u, 0x1F800000u);
    bank = PE_LoadU32(0x8009CDDCu);
    PE_StoreU8(0x1F800012u, (uint8_t)(PE_LoadU8(a0 + 17u) << 4));
    PE_StoreU8(0x1F800013u, (uint8_t)(PE_LoadU8(a0 + 17u) >> 4));
    pkt = PE_LoadU32(0x800B0E58u + bank * 4u) + PE_LoadU32(0x8009CDD8u);
    n = (uint32_t)((int32_t)(int16_t)PE_LoadU16(a0 + 4u) - 1);
    if (n == 0u) {
        PE_GuestFrame_Leave(sp, 128u);
        return;
    }
    i = 0;
    do {
        seg = chain + i * 68u;
        tp = PE_LoadU8(a0 + 16u);
        scr = PE_LoadU32(0x800F33B4u);
        PE_StoreU8(scr + 17u, (uint8_t)(tp & 0xF0u));
        PE_StoreU8(scr + 16u, (uint8_t)((PE_LoadU8(a0 + 16u) - PE_LoadU8(scr + 17u)) << 4));
        tp = (uint8_t)(tp + PE_LoadU8(seg + 3u));
        PE_StoreU8(scr + 17u, (uint8_t)(tp & 0xF0u));
        PE_StoreU8(scr + 16u, (uint8_t)((tp - PE_LoadU8(scr + 17u)) << 4));
        for (k = 0; k < 8; k++) {
            PE_StoreU8(sp + 40u + k, PE_LoadU8(seg + 20u + k));
            PE_StoreU8(sp + 48u + k, PE_LoadU8(seg + 28u + k));
            PE_StoreU8(sp + 56u + k, PE_LoadU8(seg + 88u + k));
            PE_StoreU8(sp + 64u + k, PE_LoadU8(seg + 96u + k));
        }
        cam = PE_LoadU32(0x800BCFA4u);
        PE_GTE_LoadRT(cam);
        /* func_80079304 RotTransPers4(sp40, sp48, sp56, sp64, pkt+8/16/24/32, ...) */
        set_v(0, sp + 40u); set_v(1, sp + 48u); set_v(2, sp + 56u);
        PE_GTE_RTPT_coordinates(xy, z);
        set_v(0, sp + 64u);
        PE_GTE_RTPS_coordinates(&xy3, &z3);
        PE_StoreU32(pkt + 8u, xy[0]);
        PE_StoreU32(pkt + 16u, xy[1]);
        PE_StoreU32(pkt + 24u, xy[2]);
        PE_StoreU32(pkt + 32u, xy3);
        otz = (uint32_t)((int32_t)z3 >> 2);
        func_800C608C((int16_t)PE_LoadU16(seg + 4u), seg + 16u, pkt + 4u);
        scr = PE_LoadU32(0x800F33B4u);
        PE_StoreU8(pkt + 12u, PE_LoadU8(scr + 16u));
        PE_StoreU8(pkt + 13u, PE_LoadU8(scr + 17u));
        PE_StoreU8(pkt + 20u, (uint8_t)(PE_LoadU8(scr + 16u) + PE_LoadU8(0x800F345Cu)));
        PE_StoreU8(pkt + 21u, PE_LoadU8(scr + 17u));
        PE_StoreU8(pkt + 28u, PE_LoadU8(scr + 16u));
        PE_StoreU8(pkt + 29u, (uint8_t)(PE_LoadU8(scr + 17u) + PE_LoadU8(0x800F345Du)));
        PE_StoreU8(pkt + 36u, (uint8_t)(PE_LoadU8(scr + 16u) + PE_LoadU8(0x800F345Cu)));
        PE_StoreU8(pkt + 37u, (uint8_t)(PE_LoadU8(scr + 17u) + PE_LoadU8(0x800F345Du)));
        PE_StoreU16(pkt + 22u, PE_LoadU16(0x800E27ACu));
        {
            int c = func_80077AA4((int)PE_LoadU16(0x800F341Cu) + PE_LoadU8(scr + 18u),
                                  (int)PE_LoadU16(0x800F341Eu) + PE_LoadU8(scr + 19u));
            PE_StoreU8(pkt + 3u, 9u);
            PE_StoreU8(pkt + 7u, 44u);
            PE_StoreU16(pkt + 14u, (uint16_t)c);
            PE_StoreU8(pkt + 7u, PE_LoadU8(0x800F337Au) ? 46u : 44u);
        }
        if (otz != 0u && otz < 4096u && PE_LoadU8(seg) != 0u) {
            pe_addr_t ot = PE_LoadU32(0x800B0E38u + PE_LoadU32(0x8009CDDCu) * 4u) + otz * 4u;
            PE_StoreU32(pkt, (PE_LoadU32(pkt) & 0xFF000000u) | (PE_LoadU32(ot) & 0xFFFFFFu));
            PE_StoreU32(ot, (PE_LoadU32(ot) & 0xFF000000u) | (pkt & 0xFFFFFFu));
        }
        i++;
        PE_StoreU32(0x8009CDD8u, PE_LoadU32(0x8009CDD8u) + 40u);
        pkt += 40u;
    } while (i < (uint32_t)((int32_t)(int16_t)PE_LoadU16(a0 + 4u) - 1));
    PE_GuestFrame_Leave(sp, 128u);
}
