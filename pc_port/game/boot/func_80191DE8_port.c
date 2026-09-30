/*
 * DAY1-16 — M0000I overlay leaves called from 8019234C.
 * Translated from overlay LBA2805 SHA256
 * c51e36c27422e990d4683d73dc9fc2633e0924721dd0c242a8efc2e8520a4edb,
 * not matching src/.
 *
 * func_80191DE8 — 18 words 80191DE8..80191E30.
 * func_80191E30 — 51 words 80191E30..80191EFC. 6EC6C package walk,
 * Push/SetTrans/SetRot on 8019CC30, temporary BCFA4/BCFA8, 6DF50,
 * restore, PopMatrix.
 * func_8019BF8C — 14 words 8019BF8C..8019BFC4, store in jr delay slot.
 * func_80193AB0 — 43 words 80193AB0..80193B5C, no jal. Signed lh
 * counter at 8019C058; if >0 decrement and add 10 source records at
 * 801EA268 into dest records starting 8019CAA8 (words 0/1/2, stride 16).
 * func_801941A4 — 86 words 801941A4..801942FC. SetPolyG4 + SetSemiTrans
 * fullscreen quad, AddPrim, DR_MODE a3=64, second AddPrim. Packet cursor
 * *8019C9C0.
 * func_80194108 — 39 words 80194108..801941A4. Signed fade stepper
 * calling 941A4; returns the next s16 level.
 * func_80192740 — 48 words 80192740..80192800. If *8019C1F0==1, AddPrim
 * the 801EA598 (or +36) dest into *8019C9C0's OT word at +0x3FFC.
 * If *8019C02C!=0 the original jals 80193B5C.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "game_port.h"

int func_80193B5C(int level);

/* Original 91C94..91DE8: two package reads. A failed poll reissues the
 * current table span; only the first completion selects the second span. */
static int transition_read_package(pe_addr_t table, pe_addr_t dest)
{
    unsigned epoch=PE_Port_StopEpoch();
    for (;;) {
        uint32_t start=PE_LoadU16(table);
        uint32_t lba=PE_LoadU32(0x800B0DD8u)+start;
        int sectors=(int)PE_LoadU16(table+2u)-(int)start;
        int status=func_8006E6A8((int32_t)lba,dest,sectors);
        if (PE_Port_StopEpoch()!=epoch) return 0;
        if (status==-1) continue;
        do {
            status=func_8006E7E8();
            if (PE_Port_StopEpoch()!=epoch) return 0;
        } while (status!=0 && status!=-1);
        if (status==0) return 1;
    }
}

void func_80191C94(void)
{
    if (!transition_read_package(0x80093170u,0x801D0260u)) return;
    transition_read_package(PE_LoadU8(0x8019C1F0u)?0x80093172u:0x80093174u,
                            0x8019CE10u);
}

/* Original 95F6C..96498: double-buffered G4 backgrounds, fade strips,
 * and the three draw modes. SetPolyG4 deliberately preserves tag/RGB
 * padding bytes that the original does not initialize. */
static void transition_gradient(pe_addr_t packet, const uint8_t top[3],
                                const uint8_t bottom[3], uint16_t y0,
                                uint16_t y1, int semi)
{
    func_80077BC4(packet);
    if (semi) func_80077B04(packet, 1u);
    for (unsigned i=0;i<4u;i++) {
        const uint8_t *color=i<2u ? top : bottom;
        for (unsigned j=0;j<3u;j++) PE_StoreU8(packet+4u+i*8u+j,color[j]);
        PE_StoreU16(packet+8u+i*8u,(i&1u)?320u:0u);
        PE_StoreU16(packet+10u+i*8u,i<2u?y0:y1);
    }
}

void func_80195F6C(void)
{
    static const uint8_t top0[3]={20,68,100}, bottom0[3]={5,17,25};
    static const uint8_t top1[3]={10,24,40}, bottom1[3]={5,7,15};
    static const uint8_t black[3]={0,0,0}, white[3]={255,255,255};
    for (unsigned bank=0;bank<2u;bank++) {
        uint32_t off=bank*36u, mode_off=bank*8u;
        transition_gradient(0x8019CB60u+off,top0,bottom0,0u,240u,0);
        transition_gradient(0x801EA598u+off,top1,bottom1,0u,240u,0);
        transition_gradient(0x8019C9D8u+off,black,white,200u,240u,1);
        transition_gradient(0x8019CA20u+off,white,black,0u,40u,1);
        func_80077C84(0x8019CA70u+mode_off,0u,0u,0u);
        func_80077C84(0x8019CA80u+mode_off,0u,0u,32u);
        func_80077C84(0x8019CA98u+mode_off,0u,0u,64u);
    }
}

void func_80191DE8(int a0)
{
    uint32_t fade = 16u;
    PE_StoreU8(0x800B0DCEu, 16u);
    if (a0)
        fade = 64u;
    PE_StoreU8(0x800B0DCFu, (uint8_t)fade);
    PE_StoreU16(0x800B0DD0u, 0);
    PE_StoreU16(0x800B0DD2u, 0x0FFFu);
}

int32_t func_80191E30(uint32_t id)
{
    pe_addr_t package = func_8006EC6C(0x801D0260u, 3);
    pe_addr_t matrix = 0x8019CC30u;
    uint32_t saved_cam = PE_LoadU32(0x800BCFA4u);
    uint32_t saved_h = PE_LoadU32(0x800BCFA8u);
    uint32_t saved_slot = PE_LoadU32(0x8019C04Cu);
    int32_t result;

    func_80078A94();
    func_80078E94(matrix);
    func_80078E04(matrix);
    PE_StoreU32(0x8019C04Cu, 0x300u);
    PE_StoreU32(0x800BCFA4u, matrix);
    PE_StoreU32(0x800BCFA8u, 0x8019C04Cu);
    result = func_8006DF50(package, id, 0u, 0x80u, 1u);
    PE_StoreU32(0x800BCFA4u, saved_cam);
    PE_StoreU32(0x800BCFA8u, saved_h);
    PE_StoreU32(0x8019C04Cu, saved_slot);
    func_80078B38();
    return result;
}

/* Original 59 words, 80191EFC..80191FE8. Stack-local H/SVECTOR/results
 * use a saved scratchpad window so no host pointer escapes into guest RAM. */
void func_80191EFC(uint32_t handle, pe_addr_t position)
{
    const pe_addr_t scratch = 0x1F800200u;
    uint32_t saved[6];
    uint32_t saved_cam, saved_h;
    for (unsigned i = 0; i < 6u; i++) saved[i] = PE_LoadU32(scratch + i * 4u);
    PE_StoreU32(scratch, 0x300u);
    func_80078A94();
    func_80078E94(0x8019CC30u);
    func_80078E04(0x8019CC30u);
    saved_cam = PE_LoadU32(0x800BCFA4u);
    PE_StoreU32(0x800BCFA4u, 0x8019CC30u);
    saved_h = PE_LoadU32(0x800BCFA8u);
    PE_StoreU32(0x800BCFA8u, scratch);
    for (unsigned i = 0; i < 3u; i++)
        PE_StoreU16(scratch + 8u + i * 2u, (uint16_t)PE_LoadU32(position + i * 4u));
    func_8006DFA8(scratch + 8u, scratch + 16u, scratch + 20u);
    func_800868F0(handle, 0u, PE_LoadU32(scratch + 20u));
    func_80086A28(handle, 0u, PE_LoadU32(scratch + 16u));
    PE_StoreU32(0x800BCFA4u, saved_cam);
    PE_StoreU32(0x800BCFA8u, saved_h);
    func_80078B38();
    for (unsigned i = 0; i < 6u; i++) PE_StoreU32(scratch + i * 4u, saved[i]);
}

void func_8019BF8C(pe_addr_t dest)
{
    uint32_t value = PE_LoadU32(0x800B0E4Cu);
    if (dest != 0x8019C1F8u)
        value += 0x15F90u;
    PE_StoreU32(dest, value);
}

/* Original 9BD78..9BF50: transition double-buffer display setup. */
void func_8019BD78(void)
{
    RECT rect={0,0,320,240};
    unsigned epoch=PE_Port_StopEpoch();
    pe_addr_t first,second;
    func_80074D28(0);
    func_80078FC4(128u,128u,128u);
    func_80078FE4(0u,0u,0u);
    func_80077E64(6500u,11500u,768);
    func_80079004(160,120);func_80079024(768);
    func_8019BF8C(0x8019C1F8u);func_8019BF8C(0x8019C270u);
    first=PE_LoadU32(0x800B0E38u);second=PE_LoadU32(0x800B0E3Cu);
    PE_StoreU32(0x8019C1FCu,first);PE_StoreU32(0x8019C274u,second);
    func_800752AC(first,4096);
    if (PE_Port_StopEpoch()!=epoch) return;
    func_800752AC(PE_LoadU32(0x8019C274u),4096);
    if (PE_Port_StopEpoch()!=epoch) return;
    func_80074924(0x8019C200u,0,0,320,240);
    func_800749D8(0x8019C25Cu,0,240,320,240);
    func_80074924(0x8019C278u,0,240,320,240);
    func_800749D8(0x8019C2D4u,0,0,320,240);
    PE_StoreU8(0x8019C218u,0);PE_StoreU8(0x8019C290u,0);
    PE_StoreU8(0x8019C219u,0);PE_StoreU8(0x8019C21Au,0);PE_StoreU8(0x8019C21Bu,0);
    PE_StoreU8(0x8019C291u,0);PE_StoreU8(0x8019C292u,0);PE_StoreU8(0x8019C293u,0);
    func_80074F44(&rect,0,0,0);
    if (PE_Port_StopEpoch()!=epoch) return;
    rect.x=0;rect.y=240;rect.w=320;rect.h=240;
    func_80074F44(&rect,0,0,0);
    if (PE_Port_StopEpoch()!=epoch) return;
    func_80074DC0(0);
    if (PE_Port_StopEpoch()!=epoch) return;
    func_80074D28(1);
    PE_StoreU32(0x8019C9C0u,0x8019C1F8u);
}

/* Original 9BF50..9BF8C: clear current bank, then reset its packet cursor. */
void func_8019BF50(void)
{
    unsigned epoch=PE_Port_StopEpoch();
    func_800752AC(PE_LoadU32(PE_LoadU32(0x8019C9C0u)+4u),4096);
    if (PE_Port_StopEpoch()!=epoch) return;
    func_8019BF8C(PE_LoadU32(0x8019C9C0u));
}

void func_80193AB0(void)
{
    int16_t remaining = (int16_t)PE_LoadU16(0x8019C058u);
    unsigned i;

    if (remaining <= 0)
        return;
    PE_StoreU16(0x8019C058u, (uint16_t)(remaining - 1));
    for (i = 0; i < 10u; i++) {
        pe_addr_t dest = 0x8019CAA8u + i * 16u;
        pe_addr_t src = 0x801EA268u + i * 16u;
        PE_StoreU32(dest, PE_LoadU32(dest) + PE_LoadU32(src));
        PE_StoreU32(dest + 4u, PE_LoadU32(dest + 4u) + PE_LoadU32(src + 4u));
        PE_StoreU32(dest + 8u, PE_LoadU32(dest + 8u) + PE_LoadU32(src + 8u));
    }
}

static void m0000i_addprim(pe_addr_t ctx, pe_addr_t packet, unsigned ot_off, unsigned advance)
{
    pe_addr_t ot = PE_LoadU32(ctx + 4u) + ot_off;
    uint32_t tag = PE_LoadU32(packet);
    uint32_t ot_val = PE_LoadU32(ot);

    PE_StoreU32(packet, (tag & 0xFF000000u) | (ot_val & 0x00FFFFFFu));
    PE_StoreU32(ot, (ot_val & 0xFF000000u) | (packet & 0x00FFFFFFu));
    PE_StoreU32(ctx, packet + advance);
}

void func_801941A4(uint32_t intensity)
{
    pe_addr_t ctx = PE_LoadU32(0x8019C9C0u);
    pe_addr_t packet = PE_LoadU32(ctx);
    uint8_t rgb = (uint8_t)intensity;
    unsigned i;

    func_80077BC4(packet);
    func_80077B04(packet, 1u);
    for (i = 0; i < 4u; i++) {
        pe_addr_t color = packet + 4u + i * 8u;
        PE_StoreU8(color, rgb);
        PE_StoreU8(color + 1u, rgb);
        PE_StoreU8(color + 2u, rgb);
    }
    PE_StoreU16(packet + 8u, 0);
    PE_StoreU16(packet + 10u, 0);
    PE_StoreU16(packet + 16u, 320u);
    PE_StoreU16(packet + 18u, 0);
    PE_StoreU16(packet + 24u, 0);
    PE_StoreU16(packet + 26u, 240u);
    PE_StoreU16(packet + 32u, 320u);
    PE_StoreU16(packet + 34u, 240u);
    m0000i_addprim(ctx, packet, 0u, 36u);
    packet = PE_LoadU32(ctx);
    (void)func_80077C84(packet, 0u, 0u, 64u);
    m0000i_addprim(ctx, packet, 0u, 8u);
}

int func_80194108(int level)
{
    int next = level;
    int16_t value = (int16_t)level;

    if (value > 0) {
        if (value < 255)
            func_801941A4((uint32_t)next & 0xFFu);
        else
            func_801941A4(255u);
        if (value < 255)
            next = level + 16;
    }
    value = (int16_t)next;
    if (value < 0) {
        if (value < -254) {
            func_801941A4(0u);
            next = 0;
        } else {
            func_801941A4((uint32_t)(next - 1) & 0xFFu);
            next -= 16;
        }
    }
    return (int16_t)next;
}

void func_80192740(void)
{
    if (PE_LoadU8(0x8019C1F0u) == 1u) {
        pe_addr_t ctx = PE_LoadU32(0x8019C9C0u);
        pe_addr_t prim = 0x801EA598u;
        pe_addr_t ot;
        uint32_t dest;
        uint32_t src;

        if (ctx != 0x8019C1F8u)
            prim += 36u;
        ot = PE_LoadU32(ctx + 4u);
        dest = PE_LoadU32(prim);
        src = PE_LoadU32(ot + 16380u);
        PE_StoreU32(prim, (dest & 0xFF000000u) | (src & 0x00FFFFFFu));
        src = PE_LoadU32(ot + 16380u);
        PE_StoreU32(ot + 16380u, (src & 0xFF000000u) | (prim & 0x00FFFFFFu));
    }
    if (PE_LoadU16(0x8019C02Cu) != 0u)
        PE_StoreU16(0x8019C02Cu, (uint16_t)func_80193B5C((int)(int16_t)PE_LoadU16(0x8019C02Cu)));
}

int func_80193B5C(int level)
{
    int16_t fade = (int16_t)level;
    pe_addr_t ctx = PE_LoadU32(0x8019C9C0u);
    pe_addr_t dest = 0x8019CA80u;
    pe_addr_t ot;
    uint32_t tag;
    uint32_t ot_val;
    unsigned i;
    uint8_t half;
    uint8_t third;
    uint16_t select;

    if (fade > 0) {
        if (fade < 255)
            fade = (int16_t)(fade + 8);
        if (fade >= 256)
            fade = 255;
    } else if (fade < 0) {
        if (fade >= -254)
            fade = (int16_t)(fade - 8);
        if (fade < -255)
            fade = -255;
    }

    /* Positive clamp skips the `addiu t0, t3, 255` at 80193C24 (bgez
     * target is 80193C28). Negative fade adds 255 to the lhu, then
     * both paths do sll 16 / srl 17 and srl 19. */
    {
        uint16_t bright = (uint16_t)fade;
        if (fade < 0)
            bright = (uint16_t)((uint16_t)fade + 255u);
        half = (uint8_t)(bright >> 1);
        third = (uint8_t)(bright >> 3);
    }
    if (ctx == 0x8019C1F8u)
        dest += 8u;
    ot = PE_LoadU32(ctx + 4u);
    tag = PE_LoadU32(dest);
    ot_val = PE_LoadU32(ot + 40u);
    PE_StoreU32(dest, (tag & 0xFF000000u) | (ot_val & 0x00FFFFFFu));
    PE_StoreU32(ot + 40u, (ot_val & 0xFF000000u) | (dest & 0x00FFFFFFu));

    select = PE_LoadU16(0x8019CC52u);
    for (i = 0; i < 10u; i++) {
        pe_addr_t rec = 0x801EA370u + i * 52u;
        pe_addr_t cam = 0x8019CAAAu + i * 16u;
        pe_addr_t packet;
        pe_addr_t scratch = 0x1F800080u;
        uint32_t xy0;
        uint16_t sx, sy;

        if (PE_LoadU8(0x801EA394u + i * 52u) != 1u || fade == 0)
            continue;
        PE_StoreU16(scratch, PE_LoadU16(rec));
        PE_StoreU16(scratch + 2u, PE_LoadU16(rec + 2u));
        PE_StoreU16(scratch + 4u, PE_LoadU16(rec + 4u));
        PE_StoreU16(scratch + 8u, PE_LoadU16(cam));
        PE_StoreU16(scratch + 10u, PE_LoadU16(cam + 4u));
        PE_StoreU16(scratch + 12u, PE_LoadU16(cam + 8u));
        (void)func_80079274(scratch, scratch + 8u, scratch + 16u,
                            scratch + 24u, scratch + 32u, scratch + 40u,
                            scratch + 48u, scratch + 56u);
        ctx = PE_LoadU32(0x8019C9C0u);
        packet = PE_LoadU32(ctx);
        PE_StoreU8(packet + 3u, 3u);
        PE_StoreU8(packet + 7u, 64u);
        func_80077B04(packet, 1u);
        PE_StoreU32(packet + 8u, PE_LoadU32(scratch + 24u));
        PE_StoreU32(packet + 12u, PE_LoadU32(scratch + 32u));
        if (select == (uint16_t)i) {
            PE_StoreU8(packet + 4u, half);
            PE_StoreU8(packet + 5u, half);
            PE_StoreU8(packet + 6u, half);
        } else {
            PE_StoreU8(packet + 4u, third);
            PE_StoreU8(packet + 5u, third);
            PE_StoreU8(packet + 6u, third);
        }
        m0000i_addprim(ctx, packet, 40u, 16u);
        packet = PE_LoadU32(ctx);
        func_80077BE4(packet);
        func_80077B04(packet, 1u);
        xy0 = PE_LoadU32(scratch + 32u);
        sx = (uint16_t)xy0;
        sy = (uint16_t)(xy0 >> 16);
        {
            unsigned k;
            for (k = 0; k < 4u; k++) {
                pe_addr_t color = packet + 4u + k * 12u;
                PE_StoreU8(color, half);
                PE_StoreU8(color + 1u, half);
                PE_StoreU8(color + 2u, half);
            }
        }
        PE_StoreU8(packet + 12u, 0);
        PE_StoreU8(packet + 13u, (uint8_t)(i * 13u));
        PE_StoreU8(packet + 25u, (uint8_t)(i * 13u));
        PE_StoreU8(packet + 36u, 0);
        PE_StoreU8(packet + 37u, (uint8_t)(12u + i * 13u));
        PE_StoreU8(packet + 49u, (uint8_t)(12u + i * 13u));
        PE_StoreU16(packet + 34u, (uint16_t)(sy + 12u));
        PE_StoreU8(packet + 24u, 127u);
        PE_StoreU8(packet + 48u, 127u);
        PE_StoreU16(packet + 8u, sx);
        PE_StoreU16(packet + 10u, sy);
        PE_StoreU16(packet + 20u, (uint16_t)(sx + 127u));
        PE_StoreU16(packet + 22u, sy);
        PE_StoreU16(packet + 32u, sx);
        PE_StoreU16(packet + 44u, (uint16_t)(sx + 127u));
        PE_StoreU16(packet + 46u, (uint16_t)(sy + 13u));
        PE_StoreU16(packet + 14u, (uint16_t)func_80077AA4(960, select == (uint16_t)i ? 144u : 143u));
        PE_StoreU16(packet + 26u, (uint16_t)func_80077A64(0u, 1u, 960u, 0u));
        m0000i_addprim(ctx, packet, 36u, 52u);
        packet = PE_LoadU32(ctx);
        func_80077BE4(packet);
        func_80077B04(packet, 1u);
        {
            unsigned k;
            for (k = 0; k < 4u; k++) {
                pe_addr_t color = packet + 4u + k * 12u;
                PE_StoreU8(color, 128u);
                PE_StoreU8(color + 1u, 128u);
                PE_StoreU8(color + 2u, 128u);
            }
        }
        PE_StoreU16(packet + 34u, (uint16_t)(sy + 12u));
        PE_StoreU16(packet + 46u, (uint16_t)(sy + 13u));
        PE_StoreU16(packet + 8u, sx);
        PE_StoreU16(packet + 10u, sy);
        PE_StoreU16(packet + 20u, (uint16_t)(sx + 127u));
        PE_StoreU16(packet + 22u, sy);
        PE_StoreU16(packet + 32u, sx);
        PE_StoreU16(packet + 44u, (uint16_t)(sx + 127u));
        PE_StoreU8(packet + 12u, 128u);
        PE_StoreU8(packet + 13u, (uint8_t)(i * 13u));
        PE_StoreU8(packet + 24u, 255u);
        PE_StoreU8(packet + 25u, (uint8_t)(i * 13u));
        PE_StoreU8(packet + 36u, 128u);
        PE_StoreU8(packet + 37u, (uint8_t)(12u + i * 13u));
        PE_StoreU8(packet + 48u, 255u);
        PE_StoreU8(packet + 49u, (uint8_t)(12u + i * 13u));
        PE_StoreU16(packet + 14u, (uint16_t)func_80077AA4(960, 145u));
        PE_StoreU16(packet + 26u, (uint16_t)func_80077A64(0u, 2u, 960u, 0u));
        m0000i_addprim(ctx, packet, 36u, 52u);
    }
    return fade;
}
