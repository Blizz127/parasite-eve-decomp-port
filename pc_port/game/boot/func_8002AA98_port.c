/*
 * PE-BTL100 — 2A7F8 mode-3: 2AA98 / 2B29C case 0.
 *
 * Authority: build/disc1.candidate.exe SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b. No matching src/ C.
 *
 * 2A7F8 is a compare chain, not a jump table:
 *   mode 1 → 25EE8 (not this cut)
 *   mode 2 → 2B0E8 (victory phases → mode 9)
 *   mode 3 → record+0x4C bit 0x800 ? 2AA98
 *            : 53E6C(18) ? 2AA98 (+ 5409C if 2AA98!=0)
 *            : 2B29C
 *
 * 2AA98: 8-way jtbl 0x800108F0 on D_8009CE74 (gp+0x104).
 * 2B29C: 6-way jtbl 0x80010910 on the same byte.
 * Both case 0 wait for Aya +0x0E==19 and +0x0F==+0x16
 * (death clip from 1F078 / 1A680(19)). They DIVERGE:
 *   2AA98 complete: +0x98|=0x100, CE70=16, CE74++
 *   2B29C complete: walk D20C (not Aya), then 293F4(0),
 *                   CE70=70, CE74++, Aya+0x98|=0x100
 *   2B29C wait: Aya+0x98 &= ~0x100
 *
 * 2AA98 is the bit-0x800 / category-18 presentation.
 * 2B29C is the zero-fixture remaining-actor cleanup.
 * Cases 1-4 drain CE70 / +0x252. Case 5: 295E4 tail,
 * mode=-1, 6A25C dest 0xA9400048. Overlay / 21D4C /
 * 51510 / sound jals stay deferred.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "game_port.h"

#define GA_D_8009D254 0x8009D254u
#define GA_D_8009D278 0x8009D278u
#define GA_D_8009D20C 0x8009D20Cu
#define GA_D_8009D280 0x8009D280u
#define GA_D_8009D28C 0x8009D28Cu
#define GA_D_8009D1A0 0x8009D1A0u
#define GA_D_8009D2E8 0x8009D2E8u
#define GA_D_8009D2A0 0x8009D2A0u /* gp+0x530 */
#define GA_D_8009D2A4 0x8009D2A4u /* gp+0x534 */
#define GA_D_8009D2EC 0x8009D2ECu /* gp+0x57C */
#define GA_D_8009CE70 0x8009CE70u /* gp+0x100 timer */
#define GA_D_8009CE74 0x8009CE74u /* gp+0x104 phase */
#define GA_D_800A77F4 0x800A77F4u
#define GA_D_800B0CD8 0x800B0CD8u
#define GA_D_800B0CE6 0x800B0CE6u
#define GA_D_800B0D8A 0x800B0D8Au
#define GA_D_800915E0 0x800915E0u
#define DEST_GAMEOVER 0xA9400048u

static int pe_aya_death_clip_ready(pe_addr_t aya)
{
    if (aya == 0u)
        return 0;
    if (PE_LoadU8(aya + 0x0Eu) != 19u)
        func_8001A680_command_cut(aya, 19u);
    aya = PE_LoadU32(GA_D_8009D254);
    if (aya == 0u)
        return 0;
    return PE_LoadU8(aya + 0x0Fu) == (uint8_t)PE_LoadU16(aya + 0x16u);
}

int func_8002AA98(void)
{
    pe_addr_t aya;
    uint8_t phase;
    uint32_t flags;

    phase = PE_LoadU8(GA_D_8009CE74);
    if (phase >= 8u)
        return 0;
    if (phase != 0u)
        return 0;

    aya = PE_LoadU32(GA_D_8009D254);
    if (!pe_aya_death_clip_ready(aya)) {
        aya = PE_LoadU32(GA_D_8009D254);
        if (aya != 0u) {
            flags = PE_LoadU32(aya + 0x98u);
            PE_StoreU32(aya + 0x98u, flags & ~0x100u);
        }
        return 0;
    }
    aya = PE_LoadU32(GA_D_8009D254);
    PE_StoreU8(GA_D_8009CE70, 16u);
    flags = PE_LoadU32(aya + 0x98u);
    PE_StoreU32(aya + 0x98u, flags | 0x100u);
    PE_StoreU8(GA_D_8009CE74, (uint8_t)(phase + 1u));
    return 0;
}

__attribute__((unused)) static void func_8002B29C_case0(void)
{
    pe_addr_t aya;
    pe_addr_t actor;
    uint32_t flags;

    aya = PE_LoadU32(GA_D_8009D254);
    if (!pe_aya_death_clip_ready(aya)) {
        aya = PE_LoadU32(GA_D_8009D254);
        if (aya != 0u) {
            flags = PE_LoadU32(aya + 0x98u);
            PE_StoreU32(aya + 0x98u, flags & ~0x100u);
        }
        return;
    }

    actor = PE_LoadU32(GA_D_8009D20C);
    aya = PE_LoadU32(GA_D_8009D254);
    while (actor != 0u) {
        pe_addr_t body;
        pe_addr_t next;
        int8_t kind;

        next = PE_LoadU32(actor + 4u);
        if (actor != aya) {
            body = PE_LoadU32(actor);
            kind = 0;
            if (body != 0u)
                kind = (int8_t)PE_LoadU8(body + 5u);
            if (kind != 1) {
                if (body != 0u ||
                    (PE_LoadU32(actor + 0x98u) & 0x40u) == 0u) {
                    uint16_t half;

                    half = PE_LoadU16(actor + 0x250u);
                    PE_StoreU16(actor + 0x250u, (uint16_t)(half | 2u));
                    flags = PE_LoadU32(actor + 0x98u) | 0x1000u;
                    PE_StoreU32(actor + 0x98u, flags);
                    if (body != 0u) {
                        unsigned int cmd;

                        cmd = (unsigned int)(uint16_t)(int16_t)
                              (int8_t)PE_LoadU8(body + 6u);
                        func_8001A680_command_cut(actor, cmd);
                        flags = PE_LoadU32(actor + 0x98u);
                        if ((flags & 0x40000000u) != 0u &&
                            PE_LoadU8(body + 0xAFu) == 0u) {
                            PE_StoreU32(actor + 0x98u, flags | 0x10u);
                            PE_StoreU32(actor, 0u);
                        }
                    }
                    PE_StoreU32(actor + 0x68u, 0u);
                    PE_StoreU32(actor + 0x6Cu, 0u);
                    PE_StoreU32(actor + 0x70u, 0u);
                }
            }
        }
        actor = next;
    }

    func_800293F4_hp_cut();
    aya = PE_LoadU32(GA_D_8009D254);
    PE_StoreU8(GA_D_8009CE70, 70u);
    PE_StoreU8(GA_D_8009CE74, (uint8_t)(PE_LoadU8(GA_D_8009CE74) + 1u));
    if (aya != 0u) {
        flags = PE_LoadU32(aya + 0x98u);
        PE_StoreU32(aya + 0x98u, flags | 0x100u);
    }
}

/* 2B29C[1] CE70!=60 walk: remaining actors get +0x98 bit 0x10. */
__attribute__((unused)) static void func_8002B29C_flag10_walk(void)
{
    pe_addr_t aya;
    pe_addr_t actor;

    aya = PE_LoadU32(GA_D_8009D254);
    actor = PE_LoadU32(GA_D_8009D20C);
    while (actor != 0u) {
        pe_addr_t next;
        pe_addr_t body;

        next = PE_LoadU32(actor + 4u);
        if (actor != aya) {
            body = PE_LoadU32(actor);
            if (body != 0u ||
                (PE_LoadU32(actor + 0x98u) & 0x40u) == 0u) {
                if (PE_LoadU8(actor + 0x252u) == 0u) {
                    uint32_t flags = PE_LoadU32(actor + 0x98u);
                    PE_StoreU32(actor + 0x98u, flags | 0x10u);
                }
            }
        }
        actor = next;
    }
}

/*
 * 295E4 tail (21D4C / 51510 / HUD sb storm deferred).
 * sb 0 gp+0x57C / +0x530, Aya+0x194 = D_800915E0,
 * D1A0 &= ~2, D2E8 &= ~0x10, B0CE6 |= 2.
 */
__attribute__((unused)) static void func_800295E4_tail_cut(void)
{
    pe_addr_t aya;
    uint32_t v;

    PE_StoreU8(GA_D_8009D2EC, 0u);
    PE_StoreU8(GA_D_8009D2A0, 0u);
    aya = PE_LoadU32(GA_D_8009D254);
    if (aya != 0u)
        PE_StoreU32(aya + 0x194u, PE_LoadU32(GA_D_800915E0));
    D_8009D1A0 &= ~2u;
    PE_StoreU32(GA_D_8009D1A0, PE_LoadU32(GA_D_8009D1A0) & ~2u);
    v = PE_LoadU32(GA_D_8009D2E8);
    PE_StoreU32(GA_D_8009D2E8, v & ~0x10u);
    PE_StoreU8(GA_D_800B0CE6, (uint8_t)(PE_LoadU8(GA_D_800B0CE6) | 2u));
}

/*
 * 6A25C Sys_Shutdown / game-over destination (src/func_8006A25C.c).
 * day1 09-28: the sound/CD calls were dropped ("fail-closed"), so the audio
 * was never stopped/reset on game over. Now all calls run, in retail order.
 */
void func_80081268(void);
void func_80086F34(int);
void func_80086FF8(void);
void func_80087024(void);
void func_80085744(void);
int func_80038D0C(void);
int func_80039970(void);
void func_8006A25C(void)
{
    uint32_t old;

    func_80081268();
    func_80086F34(0);
    func_80086FF8();
    func_80087024();
    func_80085744();
    if (func_80038D0C() & 0xFF)
        func_80039970();

    /* D280 is host-owned for 1220C/3F3C4; guest word is the
     * PE_Load view. Retail has one location — write both. */
    old = D_8009D280;
    if (old == 0u)
        old = PE_LoadU32(GA_D_8009D280);
    PE_StoreU32(GA_D_800A77F4, old);
    D_8009D280 = DEST_GAMEOVER;
    PE_StoreU32(GA_D_8009D280, DEST_GAMEOVER);
    PE_StoreU32(GA_D_800B0CD8, PE_LoadU32(GA_D_800B0CD8) | 0x100u);
}

__attribute__((unused)) static void func_8002B29C_case1(void)
{
    uint8_t timer;

    timer = PE_LoadU8(GA_D_8009CE70);
    if (timer == 60u) {
        pe_addr_t aya = PE_LoadU32(GA_D_8009D254);
        pe_addr_t actor = PE_LoadU32(GA_D_8009D20C);

        while (actor != 0u) {
            pe_addr_t next = PE_LoadU32(actor + 4u);
            pe_addr_t body = PE_LoadU32(actor);

            if (actor != aya) {
                int8_t kind = 0;
                if (body != 0u)
                    kind = (int8_t)PE_LoadU8(body + 5u);
                if (kind != 1 &&
                    (body != 0u ||
                     (PE_LoadU32(actor + 0x98u) & 0x40u) == 0u))
                    func_8003C5D8(actor + 0x1B4u, 60);
            }
            actor = next;
        }
    } else {
        func_8002B29C_flag10_walk();
    }
    timer = PE_LoadU8(GA_D_8009CE70);
    if (timer != 0u) {
        PE_StoreU8(GA_D_8009CE70, (uint8_t)(timer - 1u));
        return;
    }
    PE_StoreU8(GA_D_8009CE70, 30u);
    PE_StoreU8(GA_D_8009CE74,
               (uint8_t)(PE_LoadU8(GA_D_8009CE74) + 1u));
}

__attribute__((unused)) static void func_8002B29C_timer_advance(uint8_t next_timer)
{
    uint8_t timer;

    timer = PE_LoadU8(GA_D_8009CE70);
    if (timer != 0u) {
        PE_StoreU8(GA_D_8009CE70, (uint8_t)(timer - 1u));
        return;
    }
    PE_StoreU8(GA_D_8009CE70, next_timer);
    PE_StoreU8(GA_D_8009CE74,
               (uint8_t)(PE_LoadU8(GA_D_8009CE74) + 1u));
}

int func_80067B40(unsigned int a0);
void func_8001A680(pe_addr_t body, unsigned int id);   /* decomp_hand/absent_lo_actor_port.c */
void func_8002B29C(void)
{
    /* Line-for-line from src/func_8002B29C.c (Game Over presentation:
     * remaining-actor cleanup, sound fade, the D_800BE9F0 caption quad's
     * open / rise / hold / fade phases, then 295E4 + mode -1 + 6A25C).
     * Ent: f0 +0, next +4, fE +0xE, fF +0xF, f68/6C/70, f98 +0x98,
     * sub +0x1B4, f250 +0x250, f252 +0x252.  Rec: f5/f6 signed, fAF +0xAF.
     * PolyFT4 is 40 bytes: r0 +4, g0 +5, b0 +6, x0/y0 +8, x1/y1 +0x10,
     * x2/y2 +0x18, x3/y3 +0x20.  Retail reads o->f0->f5 before testing
     * f0 for null (a KUSEG read of low RAM); PE_Load maps it the same. */
#define E_F98(o) ((o) + 0x98u)
#define OR32(a, v) PE_StoreU32((a), PE_LoadU32(a) | (v))
#define OR16(a, v) PE_StoreU16((a), (uint16_t)(PE_LoadU16(a) | (v)))
#define QUAD() (0x800BE9F0u + (pe_addr_t)PE_LoadU32(0x8009CDDCu) * 40u)
#define SETXY(p, x0, y0, x1, y1, x2, y2, x3, y3) do { \
        PE_StoreU16((p) + 0x08u, (uint16_t)(x0)); PE_StoreU16((p) + 0x0Au, (uint16_t)(y0)); \
        PE_StoreU16((p) + 0x10u, (uint16_t)(x1)); PE_StoreU16((p) + 0x12u, (uint16_t)(y1)); \
        PE_StoreU16((p) + 0x18u, (uint16_t)(x2)); PE_StoreU16((p) + 0x1Au, (uint16_t)(y2)); \
        PE_StoreU16((p) + 0x20u, (uint16_t)(x3)); PE_StoreU16((p) + 0x22u, (uint16_t)(y3)); } while (0)
#define SETRGB(p, c) do { PE_StoreU8((p) + 4u, (uint8_t)(c)); PE_StoreU8((p) + 5u, (uint8_t)(c)); \
        PE_StoreU8((p) + 6u, (uint8_t)(c)); } while (0)
    pe_addr_t e, o, p;
    uint8_t t;
    unsigned int h;

    switch (PE_LoadU8(GA_D_8009CE74)) {
    case 0:
        e = PE_LoadU32(GA_D_8009D254);
        if (PE_LoadU8(e + 0x0Eu) != 0x13u) {
            func_8001A680(e, 0x13u);
            e = PE_LoadU32(GA_D_8009D254);
        }
        if (PE_LoadU8(e + 0x0Fu) != PE_LoadU16(e + 0x16u)) {
            PE_StoreU32(E_F98(e), PE_LoadU32(E_F98(e)) & ~0x100u);
            break;
        }
        for (o = PE_LoadU32(GA_D_8009D20C); o != 0u; o = PE_LoadU32(o + 4u)) {
            pe_addr_t r = PE_LoadU32(o);
            if (o == PE_LoadU32(GA_D_8009D254))
                continue;
            if ((int8_t)PE_LoadU8(r + 5u) == 1)
                continue;
            if (r == 0u && (PE_LoadU32(E_F98(o)) & 0x40u))
                continue;
            OR16(o + 0x250u, 2u);
            OR32(E_F98(o), 0x1000u);
            if (r != 0u) {
                func_8001A680(o, (uint16_t)(int16_t)(int8_t)PE_LoadU8(r + 6u));
                r = PE_LoadU32(o);
                if ((PE_LoadU32(E_F98(o)) & 0x40000000u) && PE_LoadU8(r + 0xAFu) == 0u) {
                    OR32(E_F98(o), 0x10u);
                    PE_StoreU32(o, 0u);
                }
            }
            PE_StoreU32(o + 0x68u, 0u);
            PE_StoreU32(o + 0x6Cu, 0u);
            PE_StoreU32(o + 0x70u, 0u);
        }
        func_800293F4(0);
        e = PE_LoadU32(GA_D_8009D254);
        PE_StoreU8(GA_D_8009CE70, 0x46u);
        PE_StoreU8(GA_D_8009CE74, (uint8_t)(PE_LoadU8(GA_D_8009CE74) + 1u));
        OR32(E_F98(e), 0x100u);
        break;
    case 1:
        if (PE_LoadU8(GA_D_8009CE70) == 0x3Cu) {
            (void)func_80067B40(0x3Cu);
            func_80086C5C(0, 0x3C, 0);
            for (o = PE_LoadU32(GA_D_8009D20C); o != 0u; o = PE_LoadU32(o + 4u)) {
                pe_addr_t r = PE_LoadU32(o);
                if (o == PE_LoadU32(GA_D_8009D254))
                    continue;
                if ((int8_t)PE_LoadU8(r + 5u) == 1)
                    continue;
                if (r == 0u && (PE_LoadU32(E_F98(o)) & 0x40u))
                    continue;
                func_8003C5D8(o + 0x1B4u, 0x3C);
            }
        } else {
            for (o = PE_LoadU32(GA_D_8009D20C); o != 0u; o = PE_LoadU32(o + 4u)) {
                if (o == PE_LoadU32(GA_D_8009D254))
                    continue;
                if (PE_LoadU32(o) == 0u && (PE_LoadU32(E_F98(o)) & 0x40u))
                    continue;
                if (PE_LoadU8(o + 0x252u) != 0u)
                    continue;
                OR32(E_F98(o), 0x10u);
            }
        }
        if (PE_LoadU8(GA_D_8009CE70) != 0u) {
            PE_StoreU8(GA_D_8009CE70, (uint8_t)(PE_LoadU8(GA_D_8009CE70) - 1u));
            break;
        }
        func_800703F4();
        func_800866A4(0u, 0xFFu);
        PE_StoreU8(GA_D_8009CE70, 0x1Eu);
        PE_StoreU8(GA_D_8009CE74, (uint8_t)(PE_LoadU8(GA_D_8009CE74) + 1u));
        break;
    case 2: {
        int c;
        t = PE_LoadU8(GA_D_8009CE70);
        c = 0x78 - t * 4;
        p = QUAD();
        SETRGB(p, c);
        SETXY(p, 0x64 - t * 4, 0x7A, (0x64 - t * 4) + (0x78 + t * 8), 0x7A,
              0x64 - t * 4, 0x7C, (0x64 - t * 4) + (0x78 + t * 8), 0x7C);
        if (t != 0u) {
            PE_StoreU8(GA_D_8009CE70, (uint8_t)(t - 1u));
            break;
        }
        PE_StoreU8(GA_D_8009CE70, 0x50u);
        PE_StoreU8(GA_D_8009CE74, (uint8_t)(PE_LoadU8(GA_D_8009CE74) + 1u));
        break; }
    case 3:
        t = PE_LoadU8(GA_D_8009CE70);
        p = QUAD();
        if (t >= 0x1Au) {
            int y;
            h = ((unsigned int)func_80077DC4((t - 0x10) << 4) * 11u) >> 11;
            h &= 0xFFu;
            y = 0x7A - (int)h;
            SETXY(p, 0x64, y, 0x64 + 0x78, y, 0x64, 0x7C, 0x64 + 0x78, 0x7C);
        } else {
            SETXY(p, 0x64, 0x64, 0x64 + 0x78, 0x64, 0x64, 0x7C, 0x64 + 0x78, 0x7C);
        }
        if (PE_LoadU8(GA_D_8009CE70) != 0u) {
            PE_StoreU8(GA_D_8009CE70, (uint8_t)(PE_LoadU8(GA_D_8009CE70) - 1u));
            break;
        }
        func_8003C5D8(PE_LoadU32(GA_D_8009D254) + 0x1B4u, 0x3C);
        OR16(PE_LoadU32(GA_D_8009D254) + 0x250u, 2u);
        func_8003C5D8(0x800B0B38u + 0x1B4u, 0x3C);
        PE_StoreU8(GA_D_8009CE70, 0x3Cu);
        OR16(0x800B0B38u + 0x250u, 2u);
        PE_StoreU8(GA_D_8009CE74, (uint8_t)(PE_LoadU8(GA_D_8009CE74) + 1u));
        break;
    case 4:
        if (PE_LoadU8(PE_LoadU32(GA_D_8009D254) + 0x252u) == 0u &&
            PE_LoadU8(0x800B0B38u + 0x252u) == 0u) {
            PE_StoreU8(GA_D_8009CE74, (uint8_t)(PE_LoadU8(GA_D_8009CE74) + 1u));
            break;
        }
        {
            int c5 = PE_LoadU8(GA_D_8009CE70) * 2;
            p = QUAD();
            SETXY(p, 0x64, 0x64, 0x64 + 0x78, 0x64, 0x64, 0x64 + 0x18, 0x64 + 0x78, 0x64 + 0x18);
            SETRGB(p, c5);
        }
        PE_StoreU8(GA_D_8009CE70, (uint8_t)(PE_LoadU8(GA_D_8009CE70) - 1u));
        break;
    case 5:
        func_800295E4();
        PE_StoreU32(GA_D_8009D28C, 0xFFFFFFFFu);
        func_8006A25C();
        break;
    }
    func_80077AC4(PE_LoadU32(0x800B0E38u + (pe_addr_t)PE_LoadU32(0x8009CDDCu) * 4u) + 0x10u, QUAD());
#undef E_F98
#undef OR32
#undef OR16
#undef QUAD
#undef SETXY
#undef SETRGB
}

/*
 * 2B0E8 — mode 2 (encounter-end / victory), not player death.
 * Phase on D_8009CE74:
 *   0: wait Aya+0x16==10 or D1A0&0x800; +0x98|=0x100;
 *      703F4 cleanup; 4B70C rewards; 67CBC music flag; phase++
 *   1: wait gp+0x534==1000; phase++; +0x98&=~0x100
 *   2: wait +0x0F==+0x1A; 1A680(0x15) or 0x18 if D1A0&0x1800;
 *      clear 0x1800; phase++
 *   3: 6D60C(0); F2==0 starts 45→50→64. +0xE8==-1
 *      (6A674) skips 6CDA4; bit4 clear returns 0 →
 *      295E4, mode=9, B0CD8&=~0x8000. Do not plant 0x41.
 */
/* Full 2F300 victory initialization, retaining the historical entry name. */
static void victory_rgb(pe_addr_t at,uint8_t r,uint8_t g,uint8_t b)
{ PE_StoreU8(at,r); PE_StoreU8(at+1u,g); PE_StoreU8(at+2u,b); }

void func_8002F300_mode2_cut(void)
{
    static const pe_addr_t hp[]={0x800B00ECu,0x800B00FCu,0x800B0110u,0x800B0120u};
    static const pe_addr_t at[]={0x800B0134u,0x800B0144u,0x800B017Cu,0x800B018Cu};
    static const pe_addr_t pe[]={0x800B0158u,0x800B0168u,0x800B01A0u,0x800B01B0u};
    pe_addr_t aya;
    unsigned i;
    func_800293F4(0u);
    for (i=0;i<4;i++) {
        victory_rgb(hp[i],0u,70u,130u); victory_rgb(hp[i]+8u,159u,255u,249u);
        victory_rgb(at[i],0u,130u,54u); victory_rgb(at[i]+8u,74u,255u,59u);
        victory_rgb(pe[i],255u,61u,129u); victory_rgb(pe[i]+8u,131u,19u,1u);
    }
    victory_rgb(0x800B692Cu,159u,255u,249u); victory_rgb(0x800B6948u,159u,255u,249u);
    PE_StoreU32(GA_D_8009D28C, 2u);
    PE_StoreU32(GA_D_8009D2E8,PE_LoadU32(GA_D_8009D2E8)|1u);
    aya=PE_LoadU32(GA_D_8009D254);
    PE_StoreU32(aya+0x68u,0u); PE_StoreU32(aya+0x6Cu,0u); PE_StoreU32(aya+0x70u,0u);
    PE_StoreU32(aya+0x98u,PE_LoadU32(aya+0x98u)&~0x100u);
    PE_StoreU32(GA_D_800B0CD8,PE_LoadU32(GA_D_800B0CD8)|0x8000u);
    if (!(D_8009D1A0&0x1800u)) {
        func_8001A680_command_cut(aya,20u);
        func_8006DE80(0x45B,0,(int16_t)PE_LoadU16(aya+0x2Au),
            (int16_t)PE_LoadU16(aya+0x2Eu),(int16_t)PE_LoadU16(aya+0x32u));
    }
}

/*
 * 292EC remaining-enemy tail (inside 28E94).
 * D2A0!=0 → out. Walk D20C: any non-Aya with body keeps
 * combat. If none remain and record+0x0C>0 → 2F300.
 * Aya HP<=0 does not start victory (player death is mode 3).
 */
void func_800292EC_victory_ready_cut(void)
{
    pe_addr_t actor;
    pe_addr_t aya;
    pe_addr_t rec;
    int remain;

    if ((int8_t)PE_LoadU8(GA_D_8009D2A0) != 0)
        return;
    actor = PE_LoadU32(GA_D_8009D20C);
    remain = 1;
    aya = PE_LoadU32(GA_D_8009D254);
    while (actor != 0u) {
        if (actor != aya && PE_LoadU32(actor) != 0u)
            remain = 0;
        actor = PE_LoadU32(actor + 4u);
    }
    if (remain == 0)
        return;
    rec = PE_LoadU32(GA_D_8009D278);
    if (rec == 0u)
        return;
    if ((int16_t)PE_LoadU16(rec + 0x0Cu) <= 0)
        return;
    func_8002F300_mode2_cut();
}

/* func_8002B0E8: ported from the matching decomp -- generated TU pc_port/game/decomp/func_8002B0E8_port.c (src/func_8002B0E8.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md, round 6). */

void func_8002A7F8_mode3_cut(void)
{
    pe_addr_t rec;
    uint32_t flags;
    int occ;

    rec = PE_LoadU32(GA_D_8009D278);
    if (rec == 0u)
        return;
    flags = PE_LoadU32(rec + 0x4Cu);
    if (flags & 0x800u) {
        if (func_8002AA98() != 0)
            PE_StoreU32(rec + 0x4Cu, flags & ~0x800u);
        return;
    }
    occ = func_80053E6C(18);
    if (occ != 0) {
        (void)func_8002AA98();
        return;
    }
    func_8002B29C();
}

/* Original 2B94C..2BC90 (19DE4.s): mode4 cleanup and mode10 publication. */
void func_8002B94C(void)
{
    pe_addr_t actor,aya,record;
    uint32_t flags;
    uint8_t phase=PE_LoadU8(GA_D_8009CE74);
    if (phase==0u) {
        PE_StoreU8(0x8009D244u,0);
        (void)func_800701B4();
        if (PE_Port_ShouldStop()) return;
        actor=PE_LoadU32(0x8009D20Cu);
        while (actor) {
            record=PE_LoadU32(actor);
            if (record) {
                if (actor==PE_LoadU32(GA_D_8009D254)) {
                    func_8003C5D8(0x800B0CECu,30);
                    PE_StoreU16(0x800B0D88u,PE_LoadU16(0x800B0D88u)|2u);
                    aya=PE_LoadU32(GA_D_8009D254);
                    PE_StoreU16(aya+0x250u,PE_LoadU16(aya+0x250u)|2u);
                } else {
                    func_8001A680_command_cut(actor,(uint16_t)(int16_t)(int8_t)PE_LoadU8(record+6u));
                    if (PE_Port_ShouldStop()) return;
                    PE_StoreU32(actor+0x68u,0);PE_StoreU32(actor+0x6Cu,0);PE_StoreU32(actor+0x70u,0);
                    PE_StoreU32(actor+0x98u,PE_LoadU32(actor+0x98u)|0x1000u);
                    flags=PE_LoadU32(actor+0x98u);
                    if ((flags&0x40000000u) && !PE_LoadU8(PE_LoadU32(actor)+0xAFu)) {
                        PE_StoreU32(actor+0x98u,flags|0x10u);PE_StoreU32(actor,0);
                    }
                }
                func_8003C5D8(actor+0x1B4u,30);
            } else if (PE_LoadU32(actor+0x18Cu)) func_8003C5D8(actor+0x1B4u,30);
            if (PE_Port_ShouldStop()) return;
            actor=PE_LoadU32(actor+4u);
        }
        PE_StoreU8(GA_D_8009CE74,(uint8_t)(PE_LoadU8(GA_D_8009CE74)+1u));
    } else if (phase==1u) {
        aya=PE_LoadU32(GA_D_8009D254);
        if (PE_LoadU8(aya+0x252u) || PE_LoadU8(0x800B0D8Au)) return;
        func_800293F4(0);
        if (PE_Port_ShouldStop()) return;
        actor=PE_LoadU32(0x8009D20Cu);
        PE_StoreU8(GA_D_8009CE74,(uint8_t)(PE_LoadU8(GA_D_8009CE74)+1u));
        aya=PE_LoadU32(GA_D_8009D254);
        while (actor) {
            if (actor!=aya && (PE_LoadU32(actor) || PE_LoadU32(actor+0x18Cu)))
                PE_StoreU16(actor+0x250u,PE_LoadU16(actor+0x250u)|2u);
            actor=PE_LoadU32(actor+4u);
        }
    } else if (phase==2u) {
        unsigned ready=1;
        actor=PE_LoadU32(0x8009D20Cu);aya=PE_LoadU32(GA_D_8009D254);
        while (actor) {
            if (actor!=aya && (PE_LoadU32(actor) || PE_LoadU32(actor+0x18Cu))) {
                flags=PE_LoadU32(actor+0x98u);
                if (!PE_LoadU8(actor+0x252u) || (flags&0x40u)) {
                    PE_StoreU32(actor,0);PE_StoreU32(actor+0x98u,flags|0x10u);
                } else ready=0;
            }
            actor=PE_LoadU32(actor+4u);
        }
        if (ready) {
            func_800866A4(0,255);
            if (PE_Port_ShouldStop()) return;
            func_800703F4();
            if (PE_Port_ShouldStop()) return;
            PE_StoreU8(GA_D_8009CE74,(uint8_t)(PE_LoadU8(GA_D_8009CE74)+1u));
        }
    } else if (phase==3u) {
        if (func_8006D60C(0)==1 || PE_Port_ShouldStop()) return;
        func_8002F9CC();
        if (PE_Port_ShouldStop()) return;
        func_8001A680_command_cut(PE_LoadU32(GA_D_8009D254),21);
        if (PE_Port_ShouldStop()) return;
        func_800295E4();
        if (PE_Port_ShouldStop()) return;
        PE_StoreU32(GA_D_8009D28C,10u);
    }
}

/* Original 2F0B0..2F300 (19DE4.s): mode5 cleanup and mode11 publication. */
void func_8002F0B0(void)
{
    pe_addr_t actor,aya,record;
    uint32_t flags;
    uint8_t phase=PE_LoadU8(GA_D_8009CE74);
    if (phase==0u) {
        actor=PE_LoadU32(GA_D_8009D20C);
        PE_StoreU8(0x8009D244u,0);
        while (actor) {
            if (actor!=PE_LoadU32(GA_D_8009D254)) {
                record=PE_LoadU32(actor);
                if (record || PE_LoadU32(actor+0x18Cu)) {
                    if (record) {
                        func_8001A680_command_cut(actor,(uint16_t)(int16_t)(int8_t)PE_LoadU8(record+6u));
                        if (PE_Port_ShouldStop()) return;
                        PE_StoreU32(actor+0x68u,0);PE_StoreU32(actor+0x6Cu,0);PE_StoreU32(actor+0x70u,0);
                        PE_StoreU32(actor+0x98u,PE_LoadU32(actor+0x98u)|0x1000u);
                        flags=PE_LoadU32(actor+0x98u);
                        if ((flags&0x40000000u) && !PE_LoadU8(PE_LoadU32(actor)+0xAFu)) {
                            PE_StoreU32(actor+0x98u,flags|0x10u);PE_StoreU32(actor,0);
                        }
                    }
                    func_8003C5D8(actor+0x1B4u,30);
                    if (PE_Port_ShouldStop()) return;
                    PE_StoreU16(actor+0x250u,PE_LoadU16(actor+0x250u)|2u);
                }
            }
            actor=PE_LoadU32(actor+4u);
        }
        PE_StoreU8(GA_D_8009CE74,(uint8_t)(PE_LoadU8(GA_D_8009CE74)+1u));
    } else if (phase==1u) {
        unsigned ready=1;
        actor=PE_LoadU32(GA_D_8009D20C);aya=PE_LoadU32(GA_D_8009D254);
        while (actor) {
            if (actor!=aya && (PE_LoadU32(actor) || PE_LoadU32(actor+0x18Cu))) {
                flags=PE_LoadU32(actor+0x98u);
                if (!PE_LoadU8(actor+0x252u) || (flags&0x40u)) {
                    PE_StoreU32(actor,0);PE_StoreU32(actor+0x98u,flags|0x10u);
                } else ready=0;
            }
            actor=PE_LoadU32(actor+4u);
        }
        if (ready) {
            func_800703F4();
            if (PE_Port_ShouldStop()) return;
            PE_StoreU8(GA_D_8009CE74,(uint8_t)(PE_LoadU8(GA_D_8009CE74)+1u));
        }
    } else if (phase==2u) {
        if (func_8006D60C(0)==1 || PE_Port_ShouldStop()) return;
        func_8002F9CC();
        if (PE_Port_ShouldStop()) return;
        func_8001A680_command_cut(PE_LoadU32(GA_D_8009D254),21);
        if (PE_Port_ShouldStop()) return;
        func_800295E4();
        if (PE_Port_ShouldStop()) return;
        PE_StoreU32(GA_D_8009D28C,11u);
        func_800293F4(0);
    }
}
