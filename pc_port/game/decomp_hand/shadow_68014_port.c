/* func_80068014: per-actor floor shadow (retail 58814.s, 0x80068014..0x8006869C,
 * 0x228 words). Translated from retail asm, not yet matched C (still asm in
 * configs/USA/disc1.yaml); a hand translation pending matching.
 * Called by the field dest tick 359B8 after 3AF14 for every drawn actor.
 *
 * Gate: +0x98 & 0x400 or +0x252 == 0 -> no shadow.  Builds a ground matrix
 * whose Z axis is the actor's horizontal facing (row 2 of the actor root
 * matrix at +0x238[+0x27C], Y component zeroed, VectorNormal'd) and whose
 * X axis is facing x (0,1,0) (GTE OP), translated to the actor root x/z on
 * the floor (D_800942EC when +0x98 & 0x4000000, else the actor Y at +0x2E).
 * It is composed with the view D_800B89F8 and the four corners
 * (+-size, 0, +-size) (size = u16 model+0x14) are RotTransPers'd into the
 * POLY_FT4 at +0x278 + bank*40: texpage 0xCB (x 704, 4bpp, ABR 2 =
 * subtract), CLUT 0x7210, uv (0..63, 64..127), gray level
 * (brightness * +0x27D / 128) / 2, where brightness is the +0x248..24A
 * average when +0x250 & 6, else 0x80.  Linked at the deepest corner OTZ.
 * Stack locals live on the host; the SDK helpers take guest addresses, so
 * their operands use a saved/restored scratchpad window. */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "pe_sdk.h"

#define SCR 0x1F800340u   /* 0x40 bytes, saved and restored */

int32_t func_80068014(pe_addr_t actor)
{
    uint8_t saved[0x40];
    int16_t m[9], r[9], size, neg;
    int32_t t[3], tr[3], n[3];
    pe_addr_t root, view = 0x800B89F8u, packet;
    uint32_t bank, xy, level;
    int32_t prod, lvl, depth, deepest;
    unsigned i, j;
    static const int8_t corner[4][2] = {{-1, 1}, {1, 1}, {-1, -1}, {1, -1}};

    if (PE_LoadU32(actor + 0x98u) & 0x400u) return 0;
    if (!PE_LoadU8(actor + 0x252u)) return 0;
    size = (int16_t)PE_LoadU16(PE_LoadU32(actor + 0x1B4u) + 0x14u);
    root = PE_LoadU32(actor + 0x238u) + (uint32_t)PE_LoadU8(actor + 0x27Cu) * 32u;
    for (i = 0; i < 9; i++) m[i] = (int16_t)PE_LoadU16(root + i * 2u);

    /* RT = root rotation, TR = 0; MVMVA sf=1 RT*V0(0,0,4096)+TR. */
    for (i = 0; i < 9; i++) g_pe_gte.rt[i / 3u][i % 3u] = m[i];
    g_pe_gte.tr[0] = g_pe_gte.tr[1] = g_pe_gte.tr[2] = 0;
    PE_GTE_SetV0(0, 0, 4096);
    PE_GTE_MVMVA(0x80012u);
    {
        uint8_t *save = saved;
        for (i = 0; i < 0x40u; i++) save[i] = PE_LoadU8(SCR + i);
        PE_StoreU32(SCR + 0x00u, (uint32_t)g_pe_gte.mac[0]);
        PE_StoreU32(SCR + 0x04u, 0u);
        PE_StoreU32(SCR + 0x08u, (uint32_t)g_pe_gte.mac[2]);
        if (g_pe_gte.mac[0] != 0 || g_pe_gte.mac[2] != 0) {
            (void)func_80078134(SCR, SCR + 0x10u);
            for (i = 0; i < 3; i++) n[i] = (int32_t)PE_LoadU32(SCR + 0x10u + i * 4u);
        } else { n[0] = 0; n[1] = 0; n[2] = 4096; }
    }
    /* OP with D = (nx,ny,nz) (ctc2 0/2/4 words), IR = (0,4096,0). */
    g_pe_gte.rt[0][0] = (int16_t)n[0]; g_pe_gte.rt[0][1] = (int16_t)((uint32_t)n[0] >> 16);
    g_pe_gte.rt[1][1] = (int16_t)n[1]; g_pe_gte.rt[1][2] = (int16_t)((uint32_t)n[1] >> 16);
    g_pe_gte.rt[2][2] = (int16_t)n[2];
    PE_GTE_SetIR(0, 4096, 0);
    PE_GTE_OP(1, 0);
    m[0] = (int16_t)g_pe_gte.mac[0]; m[1] = 0;    m[2] = (int16_t)n[0];
    m[3] = (int16_t)g_pe_gte.mac[1]; m[4] = 4096; m[5] = (int16_t)n[1];
    m[6] = (int16_t)g_pe_gte.mac[2]; m[7] = 0;    m[8] = (int16_t)n[2];
    t[0] = (int32_t)PE_LoadU32(root + 0x14u);
    t[1] = (PE_LoadU32(actor + 0x98u) & 0x4000000u) ? (int16_t)PE_LoadU16(0x800942ECu)
                                                    : (int16_t)PE_LoadU16(actor + 0x2Eu);
    t[2] = (int32_t)PE_LoadU32(root + 0x1Cu);

    /* R = view * M, column by column (MVMVA sf=1 RT*IR, no translation). */
    PE_GTE_LoadRT33(view);
    for (j = 0; j < 3; j++) {
        PE_GTE_SetIR(m[j], m[3 + j], m[6 + j]);
        PE_GTE_MVMVA(0x9E012u);
        for (i = 0; i < 3; i++) r[i * 3u + j] = (int16_t)g_pe_gte.ir[i];
    }
    /* T = view * t + view.t, IR outputs (swc2 IR1..3). */
    for (i = 0; i < 3; i++) g_pe_gte.tr[i] = (int32_t)PE_LoadU32(view + 0x14u + i * 4u);
    PE_GTE_SetV0((int16_t)t[0], (int16_t)t[1], (int16_t)t[2]);
    PE_GTE_MVMVA(0x80012u);
    for (i = 0; i < 3; i++) tr[i] = (int32_t)(int16_t)g_pe_gte.ir[i];
    for (i = 0; i < 9; i++) g_pe_gte.rt[i / 3u][i % 3u] = r[i];
    for (i = 0; i < 3; i++) g_pe_gte.tr[i] = tr[i];
    func_80079024((int)PE_LoadU32(0x800B8A18u));

    bank = PE_LoadU32(0x8009CDDCu);
    packet = PE_LoadU32(actor + 0x278u) + bank * 40u;
    if (!PE_RangeIsRam(packet, 40u)) {   /* no packet buffer: nothing to draw */
        for (i = 0; i < 0x40u; i++) PE_StoreU8(SCR + i, saved[i]);
        return 0;
    }
    PE_StoreU8(packet + 3u, 9u);
    PE_StoreU8(packet + 7u, 0x2Cu);
    if (PE_LoadU16(actor + 0x250u) & 6u)
        lvl = ((int32_t)PE_LoadU8(actor + 0x248u) + PE_LoadU8(actor + 0x249u) +
               PE_LoadU8(actor + 0x24Au)) / 3;
    else lvl = 0x80;
    prod = lvl * (int32_t)PE_LoadU8(actor + 0x27Du);
    if (prod < 0) prod += 0x7F;
    lvl = prod >> 7;
    level = (uint32_t)((lvl + (int32_t)((uint32_t)prod >> 31)) >> 1);
    PE_StoreU8(packet + 4u, (uint8_t)level);
    PE_StoreU8(packet + 5u, (uint8_t)level);
    PE_StoreU8(packet + 6u, (uint8_t)level);
    PE_StoreU8(packet + 0x0Cu, 0u);    PE_StoreU8(packet + 0x0Du, 0x40u);
    PE_StoreU16(packet + 0x0Eu, 0x7210u);
    PE_StoreU8(packet + 0x14u, 0x3Fu); PE_StoreU8(packet + 0x15u, 0x40u);
    PE_StoreU16(packet + 0x16u, 0xCBu);
    PE_StoreU8(packet + 0x1Cu, 0u);    PE_StoreU8(packet + 0x1Du, 0x7Fu);
    PE_StoreU8(packet + 0x24u, 0x3Fu); PE_StoreU8(packet + 0x25u, 0x7Fu);
    PE_StoreU8(packet + 7u, (uint8_t)(PE_LoadU8(packet + 7u) | 2u));

    neg = (int16_t)(0 - size);
    deepest = -1;
    for (i = 0; i < 4; i++) {
        PE_StoreU16(SCR + 0x20u, (uint16_t)(corner[i][0] < 0 ? neg : size));
        PE_StoreU16(SCR + 0x22u, 0u);
        PE_StoreU16(SCR + 0x24u, (uint16_t)(corner[i][1] < 0 ? neg : size));
        depth = func_80079244(SCR + 0x20u, SCR + 0x28u, SCR + 0x2Cu, SCR + 0x30u);
        xy = PE_LoadU32(SCR + 0x28u);
        PE_StoreU32(packet + 8u + i * 8u, xy);
        if (i == 0) { if (depth >= 0) deepest = depth; }
        else if (deepest < depth) deepest = depth;
    }
    for (i = 0; i < 0x40u; i++) PE_StoreU8(SCR + i, saved[i]);
    if ((uint32_t)deepest < 0x1000u) {
        pe_addr_t ot = PE_LoadU32(0x800B0E38u + PE_LoadU32(0x8009CDDCu) * 4u) + (uint32_t)deepest * 4u;
        PE_StoreU32(packet, (PE_LoadU32(packet) & 0xFF000000u) | (PE_LoadU32(ot) & 0xFFFFFFu));
        PE_StoreU32(ot, (PE_LoadU32(ot) & 0xFF000000u) | (packet & 0xFFFFFFu));
    }
    return 0;
}
