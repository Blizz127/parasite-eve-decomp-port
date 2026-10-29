/* func_800D3114 (EXE 0x800D3114, 0x9E8; handwritten GTE, no matched C):
 * textured ribbon strip.  Written from the retail asm (2026-10-10).
 *   pts    n+1 SVECTORs (8-byte stride), the ribbon spine
 *   n      segments (a1, sign-extended halfword)
 *   width  ribbon width; the edges are spine +-width/2 along the local x axis
 *          of a matrix rotated (RotMatrix, z angle) to the projected spine
 *          direction and translated to the view-space spine point
 *   u0     u at the first segment; segment i starts at u0 + i*du/n... (u of a
 *          quad's far edge is u0 + (i*du)/n, the last one u0 + du)
 *   v0, du, dv, tpage, clut, scale, col0, col1, abr  (stack args)
 * Builds n POLY_GT4 (0x34 bytes, code 0x3C, 0x3E semi-transparent with the
 * GetTPage(0, abr) bits or'ed into tpage when abr != 255) in the frame packet
 * buffer (D_8009CDD8 cursor advanced by n*0x34 up front), Gouraud colours
 * lerped col0 -> col1 (each scaled by scale/128) with func_80078554, and links
 * each into the OT (D_800B0E38[bank]) at its first spine point's
 * (SZ3 >> 2) - D_800F3374.  A spine point outside 0..0xFFF stops the strip
 * there, as in retail (quads already linked stay). */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "pe_sdk.h"
#include "pe_guest_ram.h"

static void rt_load(pe_addr_t m)
{
    unsigned i;
    for (i = 0; i < 8u; i++) PE_GTE_WriteCtrl(i, PE_LoadU32(m + i * 4u));
}

static void rtpt3(pe_addr_t a, pe_addr_t b, pe_addr_t c)
{
    PE_GTE_WriteData(0, PE_LoadU32(a)); PE_GTE_WriteData(1, PE_LoadU32(a + 4u));
    PE_GTE_WriteData(2, PE_LoadU32(b)); PE_GTE_WriteData(3, PE_LoadU32(b + 4u));
    PE_GTE_WriteData(4, PE_LoadU32(c)); PE_GTE_WriteData(5, PE_LoadU32(c + 4u));
    PE_GTE_Command(0x0280030u);   /* rtpt */
}

/* V0 = p; mvmva sf=1 RT*V0+TR; MAC1..3 -> out */
static void rt_v0(pe_addr_t p, pe_addr_t out)
{
    PE_GTE_WriteData(0, PE_LoadU32(p)); PE_GTE_WriteData(1, PE_LoadU32(p + 4u));
    PE_GTE_Command(0x0480012u);
    PE_StoreU32(out, PE_GTE_ReadData(25)); PE_StoreU32(out + 4u, PE_GTE_ReadData(26));
    PE_StoreU32(out + 8u, PE_GTE_ReadData(27));
}

static void add_prim(int32_t z, pe_addr_t pk)
{
    pe_addr_t ot = PE_LoadU32(0x800B0E38u + PE_LoadU32(0x8009CDDCu) * 4u);
    func_80077AC4(ot + (uint32_t)z * 4u, pk);
}

void func_800D3114(pe_addr_t pts, int32_t a1, int32_t width, int32_t u0, int32_t v0,
                   int32_t du, int32_t dv, int32_t tpage, int32_t clut, int32_t scale,
                   pe_addr_t col0, pe_addr_t col1, int32_t abr)
{
    const pe_addr_t fp = PE_GuestFrame_Enter(0x80u);
    /* guest frame: +0x00 angle SVECTOR, +0x08 +edge, +0x10 -edge, +0x18 c0,
     * +0x20 c1, +0x28 lerp colour, +0x30 sxy0, +0x38 sxy1, +0x40 matrix
     * (+0x54 translation), +0x60 sxy2, +0x64 z */
    const pe_addr_t ang = fp, ep = fp + 8u, en = fp + 0x10u, c0 = fp + 0x18u,
                    c1 = fp + 0x20u, cl = fp + 0x28u, sx0 = fp + 0x30u, sx1 = fp + 0x38u,
                    mtx = fp + 0x40u, mtr = fp + 0x54u, sx2 = fp + 0x60u;
    int n = (int16_t)a1, i;
    uint32_t bank = PE_LoadU32(0x8009CDDCu), cur = PE_LoadU32(0x8009CDD8u);
    pe_addr_t s4 = PE_LoadU32(0x800B0E58u + bank * 4u) + cur, s5 = pts;
    int32_t otoff = PE_LoadU16(0x800F3374u), z = 0, prevz = 0, acc = 0, s0;
    uint8_t vb = (uint8_t)v0, vb2 = (uint8_t)(v0 + dv);

    PE_StoreU32(0x8009CDD8u, cur + (uint32_t)n * 0x34u);
    for (i = 0; i < 3; i++) {
        PE_StoreU8(c0 + i, (uint8_t)((int32_t)PE_LoadU8(col0 + i) * scale / 128));
        PE_StoreU8(c1 + i, (uint8_t)((int32_t)PE_LoadU8(col1 + i) * scale / 128));
    }
    PE_StoreU16(ep, (uint16_t)(width / 2)); PE_StoreU16(ep + 2u, 0); PE_StoreU16(ep + 4u, 0);
    PE_StoreU16(en, (uint16_t)(-width / 2)); PE_StoreU16(en + 2u, 0); PE_StoreU16(en + 4u, 0);
    PE_StoreU16(ang, 0); PE_StoreU16(ang + 2u, 0);
    for (i = 0; i < n; i++, s5 += 8u, acc += du) {
        int32_t t;
        rt_load(PE_LoadU32(0x800BCFA4u));
        rtpt3(s5, s5 + 8u, s5);
        PE_StoreU8(s4 + 3u, 0xC);
        if (abr == 0xFF) {
            PE_StoreU8(s4 + 7u, 0x3C);
            PE_StoreU16(s4 + 0x1Au, (uint16_t)tpage);
        } else {
            PE_StoreU8(s4 + 7u, 0x3E);
            PE_StoreU16(s4 + 0x1Au, (uint16_t)((uint32_t)func_80077A64(0, abr, 0, 0) | (uint32_t)tpage));
        }
        PE_StoreU16(s4 + 0xEu, (uint16_t)clut);
        PE_StoreU32(sx0, PE_GTE_ReadData(12));
        PE_StoreU32(sx1, PE_GTE_ReadData(13));
        PE_StoreU32(sx2, PE_GTE_ReadData(14));
        {   /* avsz3 (result unused by retail) */
            uint32_t sz[3] = {PE_GTE_ReadData(17), PE_GTE_ReadData(18), PE_GTE_ReadData(19)};
            PE_GTE_AVSZ3(sz);
        }
        z = ((int32_t)PE_GTE_ReadData(19) >> 2) - otoff;
        if ((uint32_t)z >= 0x1000u)
            break;
        s0 = func_80079FB4((int16_t)PE_LoadU16(sx1 + 2u) - (int16_t)PE_LoadU16(sx0 + 2u),
                           (int16_t)PE_LoadU16(sx1) - (int16_t)PE_LoadU16(sx0)) + 0x400;
        t = (i << 12) / n;
        func_80078554(c0, c1, 0x1000 - t, t, cl);
        PE_StoreU16(ang + 4u, (uint16_t)s0);
        func_800794C4(ang, mtx);
        if (i == 0) {
            rt_v0(s5, mtr);
            /* MVMVA ran with the view matrix still loaded; the colour and
             * MAC stores interleave in retail but do not interact. */
            PE_StoreU8(s4 + 0x10u, PE_LoadU8(cl)); PE_StoreU8(s4 + 4u, PE_LoadU8(cl));
            PE_StoreU8(s4 + 0x11u, PE_LoadU8(cl + 1u)); PE_StoreU8(s4 + 5u, PE_LoadU8(cl + 1u));
            PE_StoreU8(s4 + 0x12u, PE_LoadU8(cl + 2u)); PE_StoreU8(s4 + 6u, PE_LoadU8(cl + 2u));
            rt_load(mtx);
            rtpt3(ep, en, ep);
            PE_StoreU8(s4 + 0xCu, (uint8_t)u0); PE_StoreU8(s4 + 0x18u, (uint8_t)u0);
            PE_StoreU8(s4 + 0xDu, vb); PE_StoreU8(s4 + 0x19u, vb2);
            prevz = z;
            PE_StoreU32(s4 + 8u, PE_GTE_ReadData(12));
            PE_StoreU32(s4 + 0x14u, PE_GTE_ReadData(13));
            PE_StoreU32(sx2, PE_GTE_ReadData(14));
            s4 += 0x34u;
            continue;
        }
        {
            uint8_t uq;
            rt_v0(s5, mtr);
            PE_StoreU32(s4 - 0xCu, PE_LoadU32(cl));
            PE_StoreU32(s4 - 0x18u, PE_LoadU32(cl));
            rt_load(mtx);
            rtpt3(ep, en, ep);
            uq = (uint8_t)(u0 + acc / n);
            PE_StoreU8(s4 - 0xFu, vb); PE_StoreU8(s4 - 3u, vb2);
            PE_StoreU8(s4 - 4u, uq); PE_StoreU8(s4 - 0x10u, uq);
            PE_StoreU32(s4 - 0x14u, PE_GTE_ReadData(12));
            PE_StoreU32(s4 - 8u, PE_GTE_ReadData(13));
            PE_StoreU32(sx2, PE_GTE_ReadData(14));
            add_prim(prevz, s4 - 0x34u);
            PE_StoreU32(s4 + 8u, PE_LoadU32(s4 - 0x14u));
            PE_StoreU32(s4 + 0x14u, PE_LoadU32(s4 - 8u));
            PE_StoreU8(s4 + 0x10u, PE_LoadU8(cl)); PE_StoreU8(s4 + 4u, PE_LoadU8(cl));
            PE_StoreU8(s4 + 0x11u, PE_LoadU8(cl + 1u)); PE_StoreU8(s4 + 5u, PE_LoadU8(cl + 1u));
            PE_StoreU8(s4 + 0x12u, PE_LoadU8(cl + 2u)); PE_StoreU8(s4 + 6u, PE_LoadU8(cl + 2u));
            PE_StoreU8(s4 + 0x18u, PE_LoadU8(s4 - 0x10u)); PE_StoreU8(s4 + 0xCu, PE_LoadU8(s4 - 0x10u));
            PE_StoreU8(s4 + 0xDu, vb); PE_StoreU8(s4 + 0x19u, vb2);
        }
        if (i < n - 1) {
            prevz = z;
            s4 += 0x34u;
            continue;
        }
        /* last segment: far edge at the end point, colour col1 */
        func_800794C4(ang, mtx);
        rt_load(PE_LoadU32(0x800BCFA4u));
        rt_v0(s5 + 8u, mtr);
        PE_StoreU32(s4 + 0x28u, PE_LoadU32(c1));
        PE_StoreU32(s4 + 0x1Cu, PE_LoadU32(c1));
        rt_load(mtx);
        rtpt3(ep, en, ep);
        PE_StoreU8(s4 + 0x25u, vb);
        PE_StoreU8(s4 + 0x30u, (uint8_t)(u0 + du)); PE_StoreU8(s4 + 0x24u, (uint8_t)(u0 + du));
        PE_StoreU8(s4 + 0x31u, vb2);
        PE_StoreU32(s4 + 0x20u, PE_GTE_ReadData(12));
        PE_StoreU32(s4 + 0x2Cu, PE_GTE_ReadData(13));
        PE_StoreU32(sx2, PE_GTE_ReadData(14));
        add_prim(z, s4);
    }
    PE_GuestFrame_Leave(fp, 0x80u);
}
