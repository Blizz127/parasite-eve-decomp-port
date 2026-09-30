#include <stdio.h>
#include <stdlib.h>
/*
 * PE-CH2 — func_80066800: opcode 0x82 52-byte view-record apply.
 *
 * Complete retail body (99 words / 0x18C, exe 0x80066800–0x8006698C,
 * file offset 0x57000).  Callers are opcode handler func_80018E58
 * (jal 0x80018E6C) and func_800677FC (jal 0x80067834).
 *
 * Record = *(D_800B1624) + offset_at_+0x1C + index*52.
 * Retail publishes H through *D_800BCFA8 and SetGeomScreen, copies the
 * nine rotation halfwords and three translation words through
 * *D_800BCFA4, records the byte index, and sets bit 0x80 in D_800BCF88.
 * Bytes at record+0x20 and beyond are not read.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "game_port.h"

#define GA_D_800B1624 0x800B1624u
#define GA_D_800BCF88 0x800BCF88u
#define GA_D_800BCFA4 0x800BCFA4u
#define GA_D_800BCFA8 0x800BCFA8u
#define GA_D_800BCFFD 0x800BCFFDu

int func_80066800(unsigned int index)
{
    pe_addr_t container = PE_LoadU32(GA_D_800B1624);
    pe_addr_t record = container + PE_LoadU32(container + 0x1Cu)
                     + index * 52u;
    pe_addr_t h_dest = PE_LoadU32(GA_D_800BCFA8);
    uint16_t h;
    unsigned int i;

    h = PE_LoadU16(record);
    PE_StoreU32(h_dest, h);
    func_80079024((int)h);

    for (i = 0; i < 9u; i++) {
        pe_addr_t matrix = PE_LoadU32(GA_D_800BCFA4);
        PE_StoreU16(matrix + i * 2u, PE_LoadU16(record + 2u + i * 2u));
    }
    PE_StoreU32(PE_LoadU32(GA_D_800BCFA4) + 0x14u,
                PE_LoadU32(record + 0x14u));
    PE_StoreU32(PE_LoadU32(GA_D_800BCFA4) + 0x18u,
                PE_LoadU32(record + 0x18u));
    PE_StoreU32(PE_LoadU32(GA_D_800BCFA4) + 0x1Cu,
                PE_LoadU32(record + 0x1Cu));

    PE_StoreU8(GA_D_800BCFFD, (uint8_t)index);
    PE_StoreU32(GA_D_800BCF88, PE_LoadU32(GA_D_800BCF88) | 0x80u);
    return 0;
}

/* 65E48..661A4: camera follows the projected actor within the current
 * view's bounds. Native arguments replace the caller's stack VECTOR.
 * A view change seeds the smoothing history, then clamps independently
 * on each axis. This is essential when a 512-high view becomes 224-high. */
int func_80065E48_position(int32_t x, int32_t y, int32_t z)
{
    uint32_t flags=PE_LoadU32(GA_D_800BCF88), xy, depth;
    pe_addr_t container, view, matrix, screen;
    int sx,sy,cx,cy,half_w,half_h,lo,hi;
    if (flags&7u) return -23;
    if (!(flags&0x40u)) return -24;
    container=PE_LoadU32(GA_D_800B1624);
    matrix=PE_LoadU32(GA_D_800BCFA4);
    screen=PE_LoadU32(GA_D_800BCFA8);
    if (!container || !matrix || !screen) return -24;
    view=container+PE_LoadU32(container+0x1Cu)+PE_LoadU8(GA_D_800BCFFD)*52u;
    PE_GTE_LoadRT(matrix);
    g_pe_gte.ofx=160*65536; g_pe_gte.ofy=112*65536;
    g_pe_gte.h=(uint16_t)PE_LoadU32(screen);
    PE_GTE_SetV0((int16_t)(x>>16),(int16_t)(y>>16),(int16_t)(z>>16));
    PE_GTE_RTPS_coordinates(&xy,&depth);
    sx=(int16_t)xy; sy=(int16_t)(xy>>16);
    if (flags&0x80u) {
        PE_StoreU16(0x800BCFB4u,(uint16_t)sx);
        PE_StoreU16(0x800BCFB6u,(uint16_t)sy);
    }
    sx=(sx+(int16_t)PE_LoadU16(0x800BCFB4u))/2;
    sy=(sy+(int16_t)PE_LoadU16(0x800BCFB6u))/2;
    PE_StoreU16(0x800BCFB4u,(uint16_t)sx);
    PE_StoreU16(0x800BCFB6u,(uint16_t)sy);
    half_w=(int16_t)PE_LoadU16(view+0x28u)/2;
    half_h=(int16_t)PE_LoadU16(view+0x2Au)/2;
    cx=half_w-160+sx; cy=half_h-112+sy;
    lo=(int16_t)PE_LoadU16(view+0x2Cu); hi=(int16_t)PE_LoadU16(view+0x2Eu);
    if (cx<lo) cx=lo; else if (cx>hi) cx=hi;
    lo=(int16_t)PE_LoadU16(view+0x30u); hi=(int16_t)PE_LoadU16(view+0x32u);
    if (cy<lo) cy=lo; else if (cy>hi) cy=hi;
    PE_StoreU16(0x800BCF94u,(uint16_t)(160+half_w-cx));
    PE_StoreU16(0x800BCF96u,(uint16_t)(112+half_h-cy));
    PE_StoreU16(0x800BCF8Cu,(uint16_t)cx);
    PE_StoreU16(0x800BCF8Eu,(uint16_t)cy);
    return 0;
}

/* func_800661EC: ported from the matching decomp -- generated TU pc_port/game/decomp/func_800661EC_port.c (src/func_800661EC.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* 66268: advance an explicit camera pan before the actor-follow update.
 * Stage dialogue issues 661EC even when Aya is absent. Both background
 * scroll and geometry offset must advance together. */
/* func_80066268: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80066268_port.c (src/func_80066268.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md, round 6). */

#define GA_RSIN_589C  0x8009589Cu
#define GA_RSIN_509C  0x8009509Cu
#define GA_RSIN_489C  0x8009489Cu
#define GA_RSIN_409C  0x8009409Cu
#define GA_D_800BD000 0x800BD000u
#define GA_D_800BD020 0x800BD020u
#define GA_D_800BD022 0x800BD022u
#define GA_D_800BD028 0x800BD028u
#define GA_D_800BE9A0 0x800BE9A0u
#define GA_D_8009D254 0x8009D254u
#define GA_D_8009D2E8 0x8009D2E8u
#define GA_D_800BCF8C 0x800BCF8Cu
#define GA_D_800BCF8E 0x800BCF8Eu
#define GA_D_800BCF90 0x800BCF90u
#define GA_D_800BCF92 0x800BCF92u

static int32_t pe_rsin_77d30(uint32_t a0)
{
    if (a0 < 0x401u)
        return (int16_t)PE_LoadU16(GA_RSIN_589C + a0 * 2u);
    if (a0 < 0x801u)
        return (int16_t)PE_LoadU16(GA_RSIN_589C + (0x800u - a0) * 2u);
    if (a0 < 0xC01u)
        return -(int16_t)PE_LoadU16(GA_RSIN_489C + a0 * 2u);
    return -(int16_t)PE_LoadU16(GA_RSIN_589C + (0x1000u - a0) * 2u);
}

/* func_80077CF4: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80077CF4_port.c (src/func_80077CF4.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/* func_80077DC4: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80077DC4_port.c (src/func_80077DC4.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

/*
 * PE-BTL69 — func_80066CE8 walk heading matrix.
 *
 * 158 words 0x80066CE8..0x80066F60. Sole TEXT jal from 68CE0
 * @ 68CE8; 68CE0 is 3F3C4 @ 3F560 (same gate as 37870).
 * Digital yaw is BD020; analog BD022 is not this cut.
 * rsin/rcos build Ry; GTE column-extract writes BD000 and
 * zeros BD014/18/1C. 67A78/67B74/67D18 are not this cut.
 */
void func_80066CE8(void)
{
    uint16_t yaw;
    int32_t s;
    int32_t c;
    pe_addr_t rec;

    if ((PE_LoadU16(GA_D_800BE9A0) & 0xF000u) == 0x7000u)
        yaw = PE_LoadU16(GA_D_800BD022);
    else
        yaw = PE_LoadU16(GA_D_800BD020);
    rec = PE_LoadU32(GA_D_8009D254);
    if (rec != 0u && (PE_LoadU32(GA_D_8009D2E8) & 0x10u) != 0u) {
        rec = PE_LoadU32(rec);
        yaw = (uint16_t)(yaw + 0x800u);
        if (rec != 0u)
            yaw = (uint16_t)(yaw
                             + ((PE_LoadU32(rec + 0x4Cu) >> 7) & 0xC00u));
    }
    yaw &= 0xFFFu;
    c = func_80077DC4((int32_t)yaw);
    s = func_80077CF4((int32_t)yaw);
    PE_StoreU16(GA_D_800BD000, (uint16_t)c);
    PE_StoreU16(GA_D_800BD000 + 2u, 0u);
    PE_StoreU16(GA_D_800BD000 + 4u, (uint16_t)s);
    PE_StoreU16(GA_D_800BD000 + 6u, 0u);
    PE_StoreU16(GA_D_800BD000 + 8u, 0x1000u);
    PE_StoreU16(GA_D_800BD000 + 10u, 0u);
    PE_StoreU16(GA_D_800BD000 + 12u, (uint16_t)-s);
    PE_StoreU16(GA_D_800BD000 + 14u, 0u);
    PE_StoreU16(GA_D_800BD000 + 16u, (uint16_t)c);
    PE_StoreU32(GA_D_800BD000 + 0x14u, 0u);
    PE_StoreU32(GA_D_800BD000 + 0x18u, 0u);
    PE_StoreU32(GA_D_800BD000 + 0x1Cu, 0u);
}



/* STG2: background frame controllers, 655D4..6590C in 55C00.s.
 * Each 16-byte controller selects a layer from (index, duration) pairs.
 * Initialization activates its first layer; the tick changes visible layers
 * and advances the signed 8.8 cursor. Unpublished fixture containers skip. */
int func_800655D4(void)
{
    pe_addr_t container = PE_LoadU32(GA_D_800B1624);
    pe_addr_t controller, layers;
    unsigned int i, count;
    if (!container)
        return 0;
    controller = container + PE_LoadU32(container + 0x10u);
    layers = container + PE_LoadU32(container + 0x14u);
    count = PE_LoadU16(container + 4u);
    for (i = 0; i < count; i++, controller += 16u) {
        pe_addr_t frames = controller + PE_LoadU32(controller + 12u);
        pe_addr_t layer = layers + PE_LoadU8(frames) * 56u;
        uint8_t view = PE_LoadU8(controller + 4u);
        PE_StoreU16(controller + 8u, 0x100u);
        PE_StoreU16(controller + 10u, 0u);
        PE_StoreU8(controller, 1u);
        PE_StoreU32(controller + 4u, view);
        PE_StoreU8(layer, (uint8_t)(PE_LoadU8(layer) | 2u));
    }
    return 0;
}

static int32_t pe_mips_div_hi(int32_t num, int32_t den);

void func_80065674(void)
{
    pe_addr_t container, controller, layers;
    unsigned int i, count;
    if ((D_8009D1A0 & 0x104u) != 0u ||
        (PE_LoadU32(0x800B0CD8u) & 0xC00000u) == 0x800000u)
        return;
    container = PE_LoadU32(GA_D_800B1624);
    if (!container)
        return;
    controller = container + PE_LoadU32(container + 0x10u);
    layers = container + PE_LoadU32(container + 0x14u);
    count = PE_LoadU16(container + 4u);
    for (i = 0; i < count; i++, controller += 16u) {
        uint8_t flags = PE_LoadU8(controller);
        uint32_t packed = PE_LoadU32(controller + 4u);
        int32_t cursor = (int32_t)packed >> 8;
        pe_addr_t frames, frame, layer;
        unsigned int j, frame_count;
        uint16_t elapsed;
        int32_t duration, index, limit;
        if (!(flags & 2u) || !(flags & 0x14u) ||
            (uint8_t)packed != PE_LoadU8(GA_D_800BCFFD))
            continue;
        frames = controller + PE_LoadU32(controller + 12u);
        frame_count = PE_LoadU32(controller) >> 8;
        for (j = 0; j < frame_count; j++) {
            layer = layers + PE_LoadU8(frames + j * 2u) * 56u;
            PE_StoreU8(layer, (uint8_t)(PE_LoadU8(layer) & 0xFDu));
        }
        frame = frames + (uint32_t)(cursor >> 8) * 2u;
        layer = layers + PE_LoadU8(frame) * 56u;
        PE_StoreU8(layer, (uint8_t)(PE_LoadU8(layer) | 2u));
        duration = (int8_t)PE_LoadU8(frame + 1u);
        if (duration < 0) {
            PE_StoreU8(frame + 1u, 0u);
            PE_StoreU16(controller + 10u, 0u);
            PE_StoreU8(controller, (uint8_t)(flags & 0xFBu));
            return; /* Retail stops the entire controller walk here. */
        }
        elapsed = (uint16_t)(PE_LoadU16(controller + 10u) + 1u);
        PE_StoreU16(controller + 10u, elapsed);
        if ((int32_t)elapsed < duration)
            continue;
        PE_StoreU16(controller + 10u, 0u);
        packed = (packed & 0xFFu) |
            (((uint32_t)cursor + (uint32_t)(int32_t)(int16_t)
                PE_LoadU16(controller + 8u)) << 8);
        PE_StoreU32(controller + 4u, packed);
        cursor = (int32_t)packed >> 8;
        index = (int32_t)packed >> 16;
        limit = (int32_t)(frame_count << 8);
        if (index >= (int32_t)frame_count) {
            packed &= 0xFFu;
            if (flags & 0x20u)
                packed |= (uint32_t)pe_mips_div_hi(cursor, limit) << 8;
        } else if (index < 0) {
            packed &= 0xFFu;
            if (flags & 0x20u)
                packed |= (uint32_t)(limit - pe_mips_div_hi(-cursor, limit)) << 8;
            else
                packed |= (frame_count - 1u) << 16;
        } else {
            continue;
        }
        PE_StoreU32(controller + 4u, packed);
        PE_StoreU8(controller, (uint8_t)(flags & 0xFBu));
    }
}

/*
 * PE-BTL75 — func_80067E1C camera-slot interpolate.
 *
 * 126 words 0x80067E1C..0x80068014, SHA-256
 * 99f90f7f78d13d6ac4ff441b07eaf674192eee1c5a94f7d743107aae93c83d6f.
 * Zero jal. Sole TEXT jal from 68CE0 @ 68CF8.
 * Runs when (D1A0&0x104)==0 (live 0x4000 passes).
 * Walks B1624 +0x14 records, stride 56, count at +6.
 * Bit 4: 8.8 step + remainder into +0xC/+0x20.
 * Bit 8: pull toward BCF8C vs BD028.
 * If BCF88&0x80: clear that bit and copy BCF8C/8E to BCF90/92.
 * Unpublished B1624==0 skips the walk (host; retail 6B4F8
 * publishes before 68CE0).
 */
static int32_t pe_mips_div_hi(int32_t num, int32_t den)
{
    if (den == 0)
        return 0;
    if (den == -1 && num == (int32_t)0x80000000)
        return 0;
    return num % den;
}

void func_80067E1C(void)
{
    pe_addr_t container;
    pe_addr_t rec;
    uint16_t count;
    uint16_t i;
    uint32_t flags;

    if ((D_8009D1A0 & 0x104u) != 0u)
        return;
    container = PE_LoadU32(GA_D_800B1624);
    if (container == 0u)
        goto snapshot;
    count = PE_LoadU16(container + 6u);
    rec = container + PE_LoadU32(container + 0x14u);
    for (i = 0; i < count; i++, rec += 56u) {
        if ((PE_LoadU8(rec) & 4u) != 0u) {
            int32_t acc;
            int32_t den;

            acc = (((int32_t)(int16_t)PE_LoadU16(rec + 0xCu) << 8)
                   | (int32_t)PE_LoadU8(rec + 0x20u))
                  + (int32_t)(int16_t)PE_LoadU16(rec + 0x1Cu);
            den = (int32_t)PE_LoadU16(rec + 4u);
            PE_StoreU16(rec + 0x20u, (uint16_t)(acc & 0xFF));
            PE_StoreU16(rec + 0xCu,
                        (uint16_t)pe_mips_div_hi(acc >> 8, den));
            acc = (((int32_t)(int16_t)PE_LoadU16(rec + 0xEu) << 8)
                   | (int32_t)PE_LoadU8(rec + 0x22u))
                  + (int32_t)(int16_t)PE_LoadU16(rec + 0x1Eu);
            den = (int32_t)PE_LoadU16(rec + 6u);
            PE_StoreU16(rec + 0x22u, (uint16_t)(acc & 0xFF));
            PE_StoreU16(rec + 0xEu,
                        (uint16_t)pe_mips_div_hi(acc >> 8, den));
        }
        if ((PE_LoadU8(rec) & 8u) != 0u) {
            int32_t acc;
            int32_t delta;

            delta = (int32_t)(int16_t)PE_LoadU16(GA_D_800BCF8C)
                    - (int32_t)(int16_t)PE_LoadU16(GA_D_800BD028);
            acc = ((int32_t)(int16_t)PE_LoadU16(rec + 8u) << 8)
                  + delta * (int32_t)(int16_t)PE_LoadU16(rec + 0x1Cu);
            PE_StoreU16(rec + 0xCu, (uint16_t)(acc >> 8));
            PE_StoreU16(rec + 0x20u, (uint16_t)(acc & 0xFF));
            delta = (int32_t)(int16_t)PE_LoadU16(GA_D_800BCF8E)
                    - (int32_t)(int16_t)PE_LoadU16(GA_D_800BD028 + 2u);
            acc = ((int32_t)(int16_t)PE_LoadU16(rec + 0xAu) << 8)
                  + delta * (int32_t)(int16_t)PE_LoadU16(rec + 0x1Eu);
            PE_StoreU16(rec + 0xEu, (uint16_t)(acc >> 8));
            PE_StoreU16(rec + 0x22u, (uint16_t)(acc & 0xFF));
        }
    }
snapshot:
    flags = PE_LoadU32(GA_D_800BCF88);
    if ((flags & 0x80u) != 0u) {
        PE_StoreU32(GA_D_800BCF88, flags & ~0x80u);
        PE_StoreU16(GA_D_800BCF90, PE_LoadU16(GA_D_800BCF8C));
        PE_StoreU16(GA_D_800BCF92, PE_LoadU16(GA_D_800BCF8E));
    }
}

#define GA_D_8009CDDC 0x8009CDDCu
#define GA_D_800B0E38 0x800B0E38u
#define OT_LO 0x00FFFFFFu
#define OT_HI 0xFF000000u

/*
 * PE-BTL76 — 67A78 / 67294 / 67B74 / 67D18 68CE0 tails.
 *
 * 67A78 — 50 words 0x80067A78..0x80067B40, SHA-256
 * cc38093d…2759. Sole jal 67294. Writes container+0x38/+0x3A
 * from BCF8C-160 / BCF8E-112. Walks stride-56 records; jal
 * 67294 when rec bit 1 and +0x24==BCFFD. Live m0005i rec0
 * is flags=2 +24=0, so the jal is live once BCFFD is 0.
 *
 * 67294 — 249 words 0x80067294..0x80067678, SHA-256
 * bb61228a…bdc6. Zero jal. Sole TEXT jal from 67B08.
 * Builds screen +0x18/+0x1A. OT splice uses rec+0x30/+0x34.
 * Those lists are published by 3F074→68B94→677FC→66F60,
 * which this 3F3C4 cut does not jal. Unpublished +0x30==0
 * skips the OT walk (host).
 *
 * 67B74 — 82 words. Live BCF88&0x400==0 early-out.
 * 67D18 — 65 words. Live BCF88&0x1000==0 early-out.
 * Bodies are not this cut.
 */
static void pe_ot_splice(pe_addr_t dest, pe_addr_t ot_slot)
{
    uint32_t d;
    uint32_t o;

    d = PE_LoadU32(dest);
    o = PE_LoadU32(ot_slot);
    PE_StoreU32(dest, (d & OT_HI) | (o & OT_LO));
    o = PE_LoadU32(ot_slot);
    PE_StoreU32(ot_slot, (o & OT_HI) | (dest & OT_LO));
}

int func_80067294(pe_addr_t rec)
{
    uint32_t cddc;
    pe_addr_t prim;
    pe_addr_t prim2;
    pe_addr_t container;
    pe_addr_t ot;
    pe_addr_t verts;
    uint16_t n;
    int32_t a2;
    int32_t t8;
    int32_t s0;
    uint16_t i;

    cddc = PE_LoadU32(GA_D_8009CDDC);
    prim = PE_LoadU32(rec + 0x30u);
    n = PE_LoadU16(rec + 0x26u);
    ot = PE_LoadU32(GA_D_800B0E38 + cddc * 4u);
    if (cddc != 0u)
        prim += (uint32_t)n * 16u;
    prim2 = PE_LoadU32(rec + 0x34u);
    if (cddc != 0u)
        prim2 += (uint32_t)n * 8u;
    container = PE_LoadU32(GA_D_800B1624);
    a2 = (int32_t)PE_LoadU16(rec + 0xCu)
         + (int32_t)PE_LoadU16(container + 0x38u);
    t8 = (int32_t)PE_LoadU16(rec + 0xEu)
         + (int32_t)PE_LoadU16(container + 0x3Au);
    s0 = (int32_t)PE_LoadU16(container + 0x26u)
         + (int32_t)((PE_LoadU32(rec) >> 8) & 0xFFFu);
    verts = rec + PE_LoadU32(rec + 0x28u);
    if ((PE_LoadU8(rec) & 4u) != 0u) {
        int32_t dx;
        int32_t dy;
        uint16_t w;
        uint16_t h;

        w = PE_LoadU16(rec + 4u);
        h = PE_LoadU16(rec + 6u);
        dx = (int32_t)(int16_t)a2 - 320 + (int32_t)w;
        a2 = pe_mips_div_hi(dx, (int32_t)w);
        dy = (int32_t)(int16_t)t8 - 224 + (int32_t)h;
        t8 = pe_mips_div_hi(dy, (int32_t)h);
        a2 -= (int32_t)w - 320;
        t8 -= (int32_t)h - 224;
    }
    if (n != 0u && prim != 0u && PE_AddressIsRam(prim) && PE_AddressIsRam(ot)) {
        for (i = 0; i < n; i++, prim += 16u, prim2 += 8u) {
            uint32_t word;
            int32_t x;
            int32_t y;
            int32_t z;
            pe_addr_t slot;

            word = PE_LoadU32(verts + (uint32_t)i * 4u);
            x = a2 + (int32_t)(word >> 22);
            y = t8 + (int32_t)((word >> 12) & 0x3FFu);
            z = s0 + (int32_t)(word & 0xFFFu);
            /* Retail wrap arm compares signed low halfwords, then
             * clips modulo 16 bits in both arms (67408..674A4). */
            if ((PE_LoadU8(rec) & 4u) != 0u) {
                if ((int16_t)x >= 320)
                    x -= PE_LoadU16(rec + 4u);
                else if ((int16_t)x < -15)
                    x += PE_LoadU16(rec + 4u);
                if ((int16_t)y >= 224)
                    y -= PE_LoadU16(rec + 6u);
                else if ((int16_t)y < -15)
                    y += PE_LoadU16(rec + 6u);
            }
            if ((uint16_t)(x + 15) >= 335u)
                continue;
            if ((uint16_t)(y + 15) >= 239u)
                continue;
            if ((uint16_t)(z - 8) >= 4081u)
                continue;
            slot = ot + (uint32_t)((int32_t)(int16_t)z * 4);
            PE_StoreU16(prim + 8u, (uint16_t)x);
            PE_StoreU16(prim + 10u, (uint16_t)y);
            pe_ot_splice(prim, slot);
            pe_ot_splice(prim2, slot);
        }
    }
    PE_StoreU16(rec + 0x18u, (uint16_t)a2);
    PE_StoreU16(rec + 0x1Au, (uint16_t)t8);
    return 0;
}

/* func_80067A78: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80067A78_port.c (src/func_80067A78.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md, round 6). */

/* Shared colour walk of src/func_80067B74.c and src/func_80067D18.c: for
 * each of the container's n 0x38-byte records, write `col` to bytes +6/+5/+4
 * of the c 16-byte entries at *(rec + 0x30) (offset by c << 4 when
 * D_8009CDDC != 0).  The entry pointer is a 32-bit guest address, so it is
 * read with PE_LoadU32 rather than through a host `unsigned char **`. */
static void pe_67b74_colour_walk(uint8_t col)
{
    pe_addr_t base = PE_LoadU32(0x800B1624u);
    uint32_t n = PE_LoadU16(base + 6u), i;
    pe_addr_t e = base + PE_LoadU32(base + 0x14u);

    for (i = 0; i < n; i++, e += 0x38u) {
        pe_addr_t q = PE_LoadU32(e + 0x30u);
        uint32_t c = PE_LoadU16(e + 0x26u), k;
        if (PE_LoadU32(0x8009CDDCu) != 0u)
            q += c << 4;
        for (k = 0; k < c; k++, q += 0x10u) {
            PE_StoreU8(q + 6u, col);
            PE_StoreU8(q + 5u, col);
            PE_StoreU8(q + 4u, col);
        }
    }
}

/* src/func_80067B74.c (fade step).  Was an early-return stub that skipped
 * the whole body when bit 0x400 was set (portverify). */
int func_80067B74(void)
{
    int32_t na, nb, d;
    uint32_t t;

    if ((PE_LoadU32(GA_D_800BCF88) & 0x400u) == 0u)
        return 0;
    na = (int32_t)PE_LoadU8(0x800BCFFBu) << 7;
    nb = (int32_t)PE_LoadU8(0x800BCFFAu) - 1;
    if (nb == 0) {
        /* Retail 0x80067BA8: `bnez v0` guards `break 0x7`. */
        fprintf(stderr, "FATAL: func_80067B74: retail break 7 "
                        "(D_800BCFFA == 1 divides by zero)\n");
        abort();
    }
    d = na / nb;
    pe_67b74_colour_walk((uint8_t)(0x80 - d));
    t = PE_LoadU8(0x800BCFFBu) + 1u;
    PE_StoreU8(0x800BCFFBu, (uint8_t)t);
    if ((t & 0xFFu) >= PE_LoadU8(0x800BCFFAu)) {
        pe_addr_t h = PE_LoadU32(0x800B1624u);
        PE_StoreU32(GA_D_800BCF88,
                    (PE_LoadU32(GA_D_800BCF88) & ~0xC00u) | 0x800u);
        PE_StoreU16(h + 0x26u, 0x1FF0u);
    }
    return 0;
}

/* src/func_80067D18.c (colour hold + phase advance).  Was an early-return
 * stub (portverify). */
int func_80067D18(void)
{
    uint32_t f1 = PE_LoadU32(GA_D_800BCF88), f, t;

    if ((f1 & 0x1000u) == 0u)
        return 0;
    pe_67b74_colour_walk((f1 & 0x2000u) != 0u ? PE_LoadU8(0x800BCFFCu) : 0x80u);
    f = PE_LoadU32(GA_D_800BCF88);
    t = f & 0xFFFF3FFFu;
    PE_StoreU32(GA_D_800BCF88, t);
    switch (f & 0xC000u) {
    case 0x4000u:
        PE_StoreU32(GA_D_800BCF88, t | 0x8000u);
        break;
    case 0x8000u:
        PE_StoreU32(GA_D_800BCF88, t & ~0x1000u);
        break;
    }
    return 0;
}

/* func_80068CE0: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80068CE0_port.c (src/func_80068CE0.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

#define GA_D_800BD024 0x800BD024u
#define GA_D_800BD028 0x800BD028u
#define GA_D_800BCF8C 0x800BCF8Cu
#define GA_D_800BCF8E 0x800BCF8Eu
#define GA_D_800BCFAC 0x800BCFACu

/*
 * PE-BG1 — func_80066F60 tile SPRT_16 / DR_MODE builder.
 *
 * 154 words 0x80066F60..0x800671C8, sole jal from 677FC @ 67940.
 * a0 = 56-byte layer record, a1 = prim base, a2 = guest cursor out
 * (port: host pe_addr_t *; stack temporary, no persistent guest word).
 * Two CDDC banks: SPRT_16 (code 0x7C, stride 16) then DR_MODE
 * (code 0xE1, stride 8). rec+0x30/+0x34 publish bank 0. RGB is
 * 0x80/0x80/0x80; tile+4 bit 28 sets semi-trans (code 0x7E).
 */
int func_80066F60(pe_addr_t rec, pe_addr_t prim_base, pe_addr_t *out)
{
    uint8_t watermark;
    uint16_t n;
    pe_addr_t verts;
    pe_addr_t tiles;
    pe_addr_t sprt;
    pe_addr_t mode;
    pe_addr_t container;
    uint16_t sx;
    uint16_t sy;
    uint32_t bank;
    uint32_t max_y;
    uint16_t i;

    if (out == NULL || rec < 0x80000000u ||
        !PE_RangeIsRam(rec, 56u) || !PE_RangeIsRam(prim_base, 16u))
        return 0;

    watermark = PE_LoadU8(GA_D_800BD024);
    n = PE_LoadU16(rec + 0x26u);
    verts = rec + PE_LoadU32(rec + 0x28u);
    tiles = rec + PE_LoadU32(rec + 0x2Cu);
    /* Host: do not emit into the layer's own vert/tile payload. */
    {
        pe_addr_t span = prim_base + ((uint32_t)n * 48u);
        if ((prim_base < tiles + (uint32_t)n * 8u && span > tiles) ||
            (prim_base < verts + (uint32_t)n * 4u && span > verts))
            return 0;
    }
    PE_StoreU32(rec + 0x30u, prim_base);
    mode = prim_base + ((uint32_t)n << 5);
    PE_StoreU32(rec + 0x34u, mode);
    container = PE_LoadU32(GA_D_800B1624);
    sx = (uint16_t)(PE_LoadU16(rec + 0xCu) +
                    (container >= 0x80000000u ?
                     PE_LoadU16(container + 0x38u) : 0u));
    sy = (uint16_t)(PE_LoadU16(rec + 0xEu) +
                    (container >= 0x80000000u ?
                     PE_LoadU16(container + 0x3Au) : 0u));
    PE_StoreU16(rec + 0x18u, sx);
    PE_StoreU16(rec + 0x1Au, sy);
    max_y = (uint32_t)watermark + 0x1DFu;
    sprt = prim_base;

    for (bank = 0u; bank < 2u; bank++) {
        pe_addr_t tile = tiles;
        pe_addr_t vert = verts;
        pe_addr_t dr = mode;

        for (i = 0u; i < n; i++) {
            uint32_t packed;
            uint32_t word;
            uint16_t x;
            uint16_t y;
            uint8_t code;
            uint32_t clut_y;

            if (!PE_RangeIsRam(sprt, 16u) || !PE_RangeIsRam(dr, 8u) ||
                !PE_RangeIsRam(vert, 4u) || !PE_RangeIsRam(tile, 8u))
                break;
            PE_StoreU8(sprt + 3u, 3u);
            PE_StoreU8(sprt + 7u, 0x7Cu);
            PE_StoreU8(sprt + 4u, 0x80u);
            PE_StoreU8(sprt + 5u, 0x80u);
            PE_StoreU8(sprt + 6u, 0x80u);
            packed = PE_LoadU32(vert);
            x = (uint16_t)(sx + (uint16_t)(packed >> 22));
            y = (uint16_t)(sy + (uint16_t)((packed >> 12) & 0x3FFu));
            PE_StoreU16(sprt + 8u, x);
            PE_StoreU16(sprt + 10u, y);
            PE_StoreU8(sprt + 0xCu, PE_LoadU8(tile + 4u));
            PE_StoreU8(sprt + 0xDu, PE_LoadU8(tile + 3u));
            word = PE_LoadU32(tile + 4u);
            code = PE_LoadU8(sprt + 7u);
            if ((word & 0x10000000u) != 0u)
                code = (uint8_t)(code | 2u);
            else
                code = (uint8_t)(code & 0xFDu);
            PE_StoreU8(sprt + 7u, code);
            PE_StoreU16(sprt + 0xEu,
                        (uint16_t)func_80077AA4(
                            (int32_t)((PE_LoadU32(tile) >> 5) & 0x3F0u),
                            PE_LoadU32(tile) & 0x1FFu));
            clut_y = PE_LoadU32(tile) & 0x1FFu;
            if (max_y < clut_y)
                max_y = clut_y;
            PE_StoreU8(dr + 3u, 1u);
            word = PE_LoadU32(tile);
            PE_StoreU32(dr + 4u,
                        0xE1000600u |
                        (func_80077A64(1u,
                                       (word >> 22) & 3u,
                                       (word >> 10) & 0x3C0u,
                                       (word >> 7) & 0x100u) & 0x9FFu));
            tile += 8u;
            vert += 4u;
            sprt += 16u;
            dr += 8u;
        }
        mode += (uint32_t)n << 3;
    }

    if (out != NULL)
        *out = mode;
    PE_StoreU8(GA_D_800BD024, (uint8_t)(max_y + 0x21u));
    return 0;
}

/*
 * PE-BG1 — func_800677FC view apply + per-layer 66F60.
 *
 * 114 words 0x800677FC..0x800679C4. jal 66800(BCFFD), pan from the
 * 52-byte view record into BCF8C/8E and container+0x38/+0x3A, then
 * 66F60 on each stride-56 layer. a1 is the prim cursor (host).
 */
int func_800677FC(pe_addr_t prim_base, pe_addr_t *cursor)
{
    pe_addr_t container;
    pe_addr_t view;
    pe_addr_t rec;
    uint8_t index;
    uint16_t count;
    uint16_t i;
    int32_t pan_x;
    int32_t pan_y;
    int rc;

    if (cursor == NULL)
        return -18;
    *cursor = prim_base;
    index = PE_LoadU8(GA_D_800BCFFD);
    container = PE_LoadU32(GA_D_800B1624);
    if (container < 0x80000000u || !PE_RangeIsRam(container, 0x40u))
        return 0;
    view = container + PE_LoadU32(container + 0x1Cu) +
           (uint32_t)index * 52u;
    if (!PE_RangeIsRam(view, 52u))
        return 0;
    {
        pe_addr_t mat = PE_LoadU32(GA_D_800BCFA4);
        pe_addr_t hdest = PE_LoadU32(GA_D_800BCFA8);

        /* 66800 stores through these pointers. Unpublished (BSS
         * zero) on the skip-movie first load; tiles do not need
         * the GTE matrix. Host guard, not a retail skip. */
        if (PE_RangeIsRam(mat, 0x20u) && PE_RangeIsRam(hdest, 4u))
            (void)func_80066800(index);
    }
    /* Retail: add sign-bit (srl 31), then sra for X / srl for Y. */
    pan_x = (int32_t)(int16_t)PE_LoadU16(view + 0x2Cu) +
            (int32_t)(int16_t)PE_LoadU16(view + 0x2Eu);
    pan_x = (pan_x + (pan_x < 0 ? 1 : 0)) >> 1;
    PE_StoreU16(GA_D_800BD028, (uint16_t)pan_x);
    PE_StoreU16(GA_D_800BCF8C, (uint16_t)pan_x);
    pan_y = (int32_t)(int16_t)PE_LoadU16(view + 0x30u) +
            (int32_t)(int16_t)PE_LoadU16(view + 0x32u);
    {
        uint32_t uy = (uint32_t)pan_y;

        uy = (uy + (uy >> 31)) >> 1;
        pan_y = (int32_t)uy;
    }
    PE_StoreU16(GA_D_800BD028 + 2u, (uint16_t)pan_y);
    PE_StoreU16(GA_D_800BCF8E, (uint16_t)pan_y);
    PE_StoreU16(GA_D_800BCFAC, PE_LoadU16(view + 0x2Cu));
    PE_StoreU16(GA_D_800BCFAC + 2u, PE_LoadU16(view + 0x2Eu));
    PE_StoreU16(GA_D_800BCFAC + 4u, PE_LoadU16(view + 0x30u));
    PE_StoreU16(GA_D_800BCFAC + 6u, PE_LoadU16(view + 0x32u));
    PE_StoreU16(container + 0x38u,
                (uint16_t)(PE_LoadU16(container + 0x2Cu) -
                           (uint16_t)(pan_x - 160)));
    PE_StoreU16(container + 0x3Au,
                (uint16_t)(PE_LoadU16(container + 0x2Eu) -
                           (uint16_t)(PE_LoadU16(GA_D_800BCF8E) - 112)));
    rec = container + PE_LoadU32(container + 0x14u);
    count = PE_LoadU16(container + 6u);
    for (i = 0u; i < count; i++, rec += 56u) {
        if (!PE_RangeIsRam(rec, 56u))
            break;
        rc = func_80066F60(rec, *cursor, cursor);
        if (rc != 0)
            return -18;
        PE_StoreU16(rec + 0x10u, 0x8000u);
        PE_StoreU16(rec + 0x12u, 0x7FFFu);
        PE_StoreU16(rec + 0x14u, 0x8000u);
        PE_StoreU16(rec + 0x16u, 0x7FFFu);
    }
    PE_StoreU8(0x800BCFFBu, 0u);
    PE_StoreU8(0x800BCFFAu, 0u);
    PE_StoreU32(GA_D_800BCF88, PE_LoadU32(GA_D_800BCF88) & ~0xC00u);
    return 0;
}

/*
 * PE-BG1 — func_80068B94 tile-publish cut.
 *
 * 83 words 0x80068B94..0x80068CE0. Retail: 65B70, 677FC(B0E40),
 * 79024, 655D4, then RTPS of the player integer XYZ into BCFB4/B6.
 * Initializes camera and background controllers, then projects the
 * player position (src/func_80068B94.c).
 */
/*
 * 6E9A0 publishes dest2 at B0E64 (= dest2+0) and B0E40 at dest2+8.
 * Title (skipped) republishes the FLD1 dests so 677FC can emit at
 * B0E40. Skip-movie leaves the arena overlap; writing SPRTs there
 * destroys B161C/B1624. Host: if the prim cursor sits inside dest2,
 * emit after the loaded chunk (M0431I sec2 = 18 sectors) / layer
 * payload instead. Does not rewrite B0E40.
 */
static pe_addr_t pe_bg1_prim_away_from_map(pe_addr_t prim)
{
    pe_addr_t dest2 = PE_LoadU32(0x800B0CD8u + 0x18Cu);
    pe_addr_t container = PE_LoadU32(GA_D_800B1624);
    pe_addr_t end;
    uint16_t count;
    uint16_t i;

    if (!PE_RangeIsRam(prim, 16u) || !PE_RangeIsRam(dest2, 16u))
        return prim;
    if (prim + 0x4000u <= dest2 || prim >= dest2 + 0x10000u)
        return prim;

    end = dest2 + 0x9000u; /* 18 PE.IMG sectors; M0431I packed 0x01200911 */
    if (PE_RangeIsRam(container, 0x40u)) {
        pe_addr_t rec = container + PE_LoadU32(container + 0x14u);

        if (container + 0x40u > end)
            end = container + 0x40u;
        count = PE_LoadU16(container + 6u);
        for (i = 0u; i < count && i < 32u; i++, rec += 56u) {
            uint16_t n;
            pe_addr_t verts;
            pe_addr_t tiles;

            if (!PE_RangeIsRam(rec, 56u))
                break;
            n = PE_LoadU16(rec + 0x26u);
            verts = rec + PE_LoadU32(rec + 0x28u);
            tiles = rec + PE_LoadU32(rec + 0x2Cu);
            if (PE_RangeIsRam(verts, 4u) &&
                verts + (uint32_t)n * 4u > end)
                end = verts + (uint32_t)n * 4u;
            if (PE_RangeIsRam(tiles, 8u) &&
                tiles + (uint32_t)n * 8u > end)
                end = tiles + (uint32_t)n * 8u;
        }
    }
    end = (end + 0xFu) & ~0xFu;
    if (PE_RangeIsRam(end, 0x4000u))
        return end;
    return prim;
}

/* Retail 65B70..65C38: reset field camera state and publish outputs. */
static int pe_camera_65b70(pe_addr_t matrix, pe_addr_t screen)
{
    unsigned int i;
    PE_StoreU32(0x800BCF88u, 0x70u);
    PE_StoreU8(0x800BCFFCu, 0x60u);
    PE_StoreU16(0x800BCFFEu, 0x180u);
    PE_StoreU8(0x800BD027u, 0xFFu);
    PE_StoreU8(0x800BD026u, 0xFFu);
    PE_StoreU8(0x800BD025u, 0xFFu);
    for (i = 0; i < 6u; i++) PE_StoreU32(0x800BCF8Cu + i * 4u, 0u);
    PE_StoreU32(0x800BCFA4u, matrix);
    PE_StoreU32(0x800BCFA8u, screen);
    PE_StoreU32(0x800BCFACu, 0u);
    PE_StoreU32(0x800BCFB0u, 0u);
    PE_StoreU32(0x800BCFB4u, 0u);
    PE_StoreU8(0x800BCFFDu, 0u);
    PE_StoreU16(0x800BD022u, 0u);
    PE_StoreU16(0x800BD020u, 0u);
    PE_StoreU8(0x800BD024u, 0u);
    PE_StoreU32(0x800BD028u, 0u);
    return 0;
}

int func_80068B94(void)
{
    pe_addr_t cursor;
    pe_addr_t prim;
    int rc;

    (void)pe_camera_65b70(0x800B89F8u, 0x800B8A18u);
    prim = pe_bg1_prim_away_from_map(PE_LoadU32(0x800B0E40u));
    if (!PE_RangeIsRam(prim, 16u))
        return -2;
    cursor = prim;
    rc = func_800677FC(prim, &cursor);
    if (rc != 0)
        return -2;
    /* src/func_80068B94.c: SetGeomScreen(state[8]) between 677FC and
     * 655D4, then RTPS of the player's integer position (zeros without a
     * player) with OFX/OFY = 160/112 and the camera matrix *D_800BCFA4;
     * SXY2 -> D_800BCFB4 / D_800BCFB6. */
    func_80079024((int)PE_LoadU32(0x800B8A18u));
    func_800655D4();
    {
        pe_addr_t pl = PE_LoadU32(GA_D_8009D254);
        int16_t v[3] = { 0, 0, 0 };
        uint32_t xy, z;

        if (pl != 0u) {
            v[0] = (int16_t)PE_LoadU16(pl + 0x2Au);
            v[1] = (int16_t)PE_LoadU16(pl + 0x2Eu);
            v[2] = (int16_t)PE_LoadU16(pl + 0x32u);
        }
        func_80079004(160, 112);
        PE_GTE_LoadRT(PE_LoadU32(0x800BCFA4u));
        PE_GTE_SetV0(v[0], v[1], v[2]);
        PE_GTE_RTPS_coordinates(&xy, &z);
        PE_StoreU16(0x800BCFB4u, (uint16_t)(xy & 0xFFFFu));
        PE_StoreU16(0x800BCFB6u, (uint16_t)(xy >> 16));
    }
    return 0;
}
