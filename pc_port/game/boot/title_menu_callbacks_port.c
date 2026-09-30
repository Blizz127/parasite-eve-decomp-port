/*
 * Title-screen object callbacks (PE.IMG overlay 0x03D2, load VA
 * 0x8018EFF0), translated by the fmv lane from the overlay bytes so the
 * post-FMV title can reach its menu:
 *
 *   func_8018F958  [0x8018F958,0x8018FBC0)  type-6 wipe compositor (+0x10)
 *   func_8018FD04  [0x8018FD04,0x8018FE1C)  alloc type-4 node
 *   func_8018FE1C  [0x8018FE1C,0x80190064)  build the NEW GAME/CONTINUE menu
 *   func_80192F98  [0x80192F98,0x80192FE8)  type-5 arrival callback (+0x14)
 *   func_80193084  [0x80193084,0x801930D8)  alpha fade draw (+0xC)
 *   func_801930D8  [0x801930D8,0x8019316C)  type-6 cursor draw (+0xC)
 *   func_8019316C  [0x8019316C,0x8019319C)  type-6 fade-out draw (+0xC)
 *   func_801931BC  [0x801931BC,0x80193200)  Start callback: arm type 2
 *   func_80193200  [0x80193200,0x80193254)  "Press Start" fade-out draw
 *
 * Node layout (52 bytes): +0 next, +4/+6/+8/+A x/y/w/h (int16),
 * +C draw, +10 update(node,dst,skip_words,row_words), +14 callback,
 * +18 sprite (TIM record; pixels at +0x14), +1C alpha, +20/+24/+28
 * params, +2C type, +30 done.
 */
#include "psx_compat.h"
#include "game_port.h"
#include "pe_guestcode.h"

#define GA_FREELIST   0x801D136Cu
#define GA_LIST       0x801D1370u
#define GA_LIVE_HEAD  0x801D1378u
#define GA_LIVE_TAIL  0x801D137Cu
#define GA_DESC_TABLE 0x801D0D5Cu
#define GA_ANCHOR     0x80193254u
#define GA_TIM_TABLE  0x80193258u

extern pe_addr_t func_8018FBC0(int type);
extern void func_8018F7F0(pe_addr_t node, pe_addr_t dst, int32_t skip_words,
                          int32_t row_words);
extern void func_80192FE8(pe_addr_t node);
extern void func_8019319C(pe_addr_t node);

void func_8018F958(pe_addr_t node, pe_addr_t dst, int32_t skip_words,
                   int32_t row_words);
void func_8018FD04(void);
void func_8018FE1C(void);
void func_80192F98(pe_addr_t node);
void func_80193084(pe_addr_t node);
void func_801930D8(pe_addr_t node);
void func_8019316C(pe_addr_t node);
void func_801931BC(pe_addr_t node);
void func_80193200(pe_addr_t node);

/* Blend one byte: dst = max(dst, src * alpha >> 8) (signed mult/sra). */
static void Blend(pe_addr_t dst, uint32_t src_byte, uint32_t alpha)
{
    int32_t v = (int32_t)((int32_t)src_byte * (int32_t)alpha) >> 8;
    int32_t d = (int32_t)PE_LoadU8(dst);

    PE_StoreU8(dst, (uint8_t)(v < d ? d : v));
}

/* The inline allocation shared by 8018FD04 / 8018FE1C (same shape as
 * func_8018FBC0 but without its draw/update/+0x20/+0x24 seeds). */
static pe_addr_t AllocInline(uint32_t type, pe_addr_t draw, pe_addr_t update)
{
    pe_addr_t node = PE_LoadU32(GA_FREELIST);
    pe_addr_t tail = PE_LoadU32(GA_LIVE_TAIL);
    pe_addr_t desc = GA_DESC_TABLE + type * 12u;
    pe_addr_t tim;
    int32_t w2;
    unsigned i;

    PE_StoreU32(GA_FREELIST, PE_LoadU32(node));
    PE_StoreU32(node, 0u);
    if (tail != 0u)
        PE_StoreU32(tail, node);
    else
        PE_StoreU32(GA_LIVE_HEAD, node);
    PE_StoreU32(GA_LIVE_TAIL, node);
    for (i = 0x0Cu; i <= 0x30u; i += 4u)
        PE_StoreU32(node + i, 0u);
    PE_StoreU16(node + 4u, 0u);
    PE_StoreU16(node + 6u, 0u);
    PE_StoreU16(node + 8u, 0u);
    PE_StoreU16(node + 10u, 0u);
    PE_StoreU32(node + 0x2Cu, type);
    tim = PE_LoadU32(GA_TIM_TABLE + type * 4u) + GA_ANCHOR;
    PE_StoreU32(node + 0x18u, tim);
    PE_StoreU16(node + 4u, (uint16_t)PE_LoadU32(desc));
    PE_StoreU16(node + 6u, (uint16_t)PE_LoadU32(desc + 4u));
    w2 = (int32_t)(int16_t)PE_LoadU16(tim + 0x10u) << 1;
    PE_StoreU16(node + 8u,
                (uint16_t)((int32_t)(((int64_t)w2 * 0x55555556ll) >> 32) -
                           (w2 >> 31)));
    PE_StoreU16(node + 10u, PE_LoadU16(tim + 0x12u));
    PE_StoreU32(node + 0x0Cu, draw);
    PE_StoreU32(node + 0x10u, update);
    PE_StoreU32(node + 0x28u, PE_LoadU32(desc + 8u));
    return node;
}

static pe_addr_t FindType(uint32_t type)
{
    pe_addr_t n = PE_LoadU32(GA_LIST);

    while (n != 0u && PE_LoadU32(n + 0x2Cu) != type)
        n = PE_LoadU32(n);
    return n;
}

/* 0x8018F958: 84x24 sprite revealed as a diagonal wipe (part 1), then the
 * same sprite read backwards/transposed for the trailing edge (part 2);
 * both max-blend with alpha +0x1C.  Parameter p = +0x20 (0..84). */
void func_8018F958(pe_addr_t node, pe_addr_t dst, int32_t skip_words,
                   int32_t row_words)
{
    int32_t p = (int32_t)PE_LoadU32(node + 0x20u);
    pe_addr_t src = PE_LoadU32(node + 0x18u) + 20u;
    uint32_t alpha = PE_LoadU32(node + 0x1Cu);
    pe_addr_t out = dst;
    int32_t t7 = p + 24;
    int32_t row;

    if (60 - p >= 0)
        out += (uint32_t)(row_words * (60 - p)) << 2;
    for (row = 0; row < 24; row++, t7--) {
        int32_t n = t7 < 85 ? t7 : 84;
        int32_t k;

        for (k = 0; k < n; k++) {
            unsigned c;

            for (c = 0u; c < 3u; c++) {
                Blend(out, PE_LoadU8(src), alpha);
                src++;
                out++;
            }
        }
        src += (uint32_t)((84 - n) * 3);
        out += (uint32_t)(skip_words * 4 + (84 - n) * 3);
    }

    {
        pe_addr_t t3 = PE_LoadU32(node + 0x18u) + 6065u;
        pe_addr_t t1 = dst + (uint32_t)(p * 3);
        int32_t rows = 84 - p;
        int32_t t2 = 84;

        for (row = 0; row < rows; row++, t2--) {
            pe_addr_t s = t3;
            pe_addr_t d = t1;
            int32_t lim = t2 - p;
            int32_t k;

            if (lim >= 25)
                lim = 24;
            for (k = 0; k < lim; k++) {
                Blend(d, PE_LoadU8(s), alpha);
                Blend(d + 1u, PE_LoadU8(s + 1u), alpha);
                Blend(d + 2u, PE_LoadU8(s + 2u), alpha);
                d += 3u;
                s -= 252u;
            }
            t3 -= 3u;
            t1 += (uint32_t)row_words << 2;
        }
    }
}

void func_8018FD04(void)
{
    (void)AllocInline(4u, 0x80193084u, 0x8018F7F0u);
}

void func_8018FE1C(void)
{
    pe_addr_t five;

    (void)AllocInline(7u, 0u, 0x8018F7F0u);
    (void)func_8018FBC0(3);
    func_8018FD04();
    five = func_8018FBC0(5);
    PE_StoreU32(five + 0x14u, 0x80192F98u);
    (void)AllocInline(6u, 0x801930D8u, 0x8018F958u);
}

void func_80192F98(pe_addr_t node)
{
    pe_addr_t seven;

    PE_StoreU16(node + 6u, 180u);
    PE_StoreU32(node + 0x14u, 0u);
    PE_StoreU32(node + 0x0Cu, 0u);
    PE_StoreU32(node + 0x20u, 0u);
    seven = FindType(7u);
    if (seven != 0u) /* retail stores through the walk result unguarded */
        PE_StoreU32(seven + 0x1Cu, 256u);
}

void func_80193084(pe_addr_t node)
{
    int32_t a = (int32_t)PE_LoadU32(node + 0x1Cu);
    int32_t speed = (int32_t)PE_LoadU32(node + 0x20u);

    if (a <= 0) {
        if (speed < 0)
            PE_StoreU32(node + 0x20u, 0u);
    } else if (a >= 256 && speed > 0) {
        PE_StoreU32(node + 0x20u, 0u);
    }
    PE_StoreU32(node + 0x1Cu,
                PE_LoadU32(node + 0x1Cu) + PE_LoadU32(node + 0x20u));
}

void func_801930D8(pe_addr_t node)
{
    int32_t p = (int32_t)PE_LoadU32(node + 0x20u);
    int32_t a = (int32_t)PE_LoadU32(node + 0x1Cu);
    int32_t h;

    p += p != 0 ? (p < 84) : 0;
    PE_StoreU32(node + 0x20u, (uint32_t)p);
    if (a < 256)
        a += (int32_t)PE_LoadU32(node + 0x24u) << 3;
    PE_StoreU32(node + 0x1Cu, (uint32_t)a);
    if (a == 0 || a == 256)
        PE_StoreU32(node + 0x24u, 0u);
    p = (int32_t)PE_LoadU32(node + 0x20u);
    PE_StoreU16(node + 6u, (uint16_t)(60 - p >= 0 ? p + 80 : 140));
    h = 84 - (int32_t)PE_LoadU32(node + 0x20u);
    PE_StoreU16(node + 10u, (uint16_t)(h < 24 ? 24 : h));
}

void func_8019316C(pe_addr_t node)
{
    int32_t a = (int32_t)PE_LoadU32(node + 0x1Cu) - 16;

    if (a < 0)
        a = 0;
    PE_StoreU32(node + 0x1Cu, (uint32_t)a);
    if (a == 0) {
        PE_StoreU32(node + 0x24u, 0u);
        PE_StoreU32(node + 0x20u, 0u);
    }
}

void func_801931BC(pe_addr_t node)
{
    pe_addr_t two = FindType(2u);

    (void)node;
    if (two != 0u) /* retail stores through the walk result unguarded */
        PE_StoreU32(two + 0x0Cu, 0x80193200u);
}

void func_80193200(pe_addr_t node)
{
    PE_StoreU32(node + 0x1Cu, PE_LoadU32(node + 0x1Cu) - 16u);
    if (PE_LoadU32(node + 0x1Cu) == 128u)
        func_8018FE1C();
    if (PE_LoadU32(node + 0x1Cu) == 0u)
        PE_StoreU32(node + 0x30u, 1u);
}

/* Title node callback dispatch (draw +0xC / callback +0x14 take the node;
 * update +0x10 takes node,dst,skip,row).  Returns 0 for an unknown
 * address after reporting it through PE_GuestCall. */
int PE_TitleNodeCall(const char *site, pe_addr_t fn, pe_addr_t node,
                     pe_addr_t a1, int32_t a2, int32_t a3)
{
    switch (fn) {
    case 0x80192FE8u: func_80192FE8(node); return 1;
    case 0x8019319Cu: func_8019319C(node); return 1;
    case 0x80192F98u: func_80192F98(node); return 1;
    case 0x80193084u: func_80193084(node); return 1;
    case 0x801930D8u: func_801930D8(node); return 1;
    case 0x8019316Cu: func_8019316C(node); return 1;
    case 0x801931BCu: func_801931BC(node); return 1;
    case 0x80193200u: func_80193200(node); return 1;
    case 0x8018F7F0u: func_8018F7F0(node, a1, a2, a3); return 1;
    case 0x8018F958u: func_8018F958(node, a1, a2, a3); return 1;
    default:
        (void)PE_GuestCall(site, fn, 4u, node, a1, (uintptr_t)(uint32_t)a2,
                           (uintptr_t)(uint32_t)a3);
        return 0;
    }
}
