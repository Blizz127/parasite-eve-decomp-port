/*
 * PE-BTL105 — 27D14 enemy tick and 28E94 death phases.
 *
 * Authority: build/disc1.candidate.exe SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b. No matching src/ C.
 *
 * 27D14: 534 words 0x80027D14..0x80028570. a0 = actor, *actor = body.
 * After 1D340, 299CC @ 0x8002A53C walks non-Aya bodies here.
 * body&0x6000==0x2000 jals 28574 (Attack HP subtract).
 * body+0x10<=0 and !(D1A0&0x100) and kind not 1/3 → zero
 * +0x68/6C/70, jal 28E94(actor).
 *
 * 28E94 is complete: death clip/fade, linked actors, rewards, drops
 * and the remaining-enemy gate into 2F300's victory initialization.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define GA_D_8009D254 0x8009D254u
#define GA_D_8009D20C 0x8009D20Cu
#define GA_D_8009D1A0 0x8009D1A0u
#define GA_D_8009D2A0 0x8009D2A0u
#define GA_D_8009D278 0x8009D278u
#define GA_D_8009D294 0x8009D294u
#define GA_D_8009D1D4 0x8009D1D4u
#define GA_D_8009CE54 0x8009CE54u
#define GA_D_8009CE55 0x8009CE55u
#define GA_D_8009CE48 0x8009CE48u
#define GA_D_8009CE4C 0x8009CE4Cu
#define GA_D_8009CE3C 0x8009CE3Cu
#define GA_D_8009CE38 0x8009CE38u
#define GA_D_8009CE39 0x8009CE39u
#define GA_D_8009D25C 0x8009D25Cu
#define GA_D_8009D258 0x8009D258u
#define GA_OVERLAY    0x800B0CD8u
#define GA_T_800BE830 0x800BE830u
#define GA_D_800A5D58 0x800A5D58u
#define SLOT_STRIDE   220u
#define BODY_HIT      0x2000u
#define BODY_REACT    0x4000u
#define BODY_HITMASK  0x6000u



void func_8002F970(pe_addr_t p)
{
    pe_addr_t want;
    unsigned int i;

    if (p == 0u)
        return;
    want = PE_LoadU32(p);
    for (i = 0u; i < 7u; i++) {
        pe_addr_t rec = GA_D_800A5D58 + i * SLOT_STRIDE;

        if (rec + 4u == want)
            PE_StoreU32(rec, 0u);
    }
    PE_StoreU32(p, 0u);
}

static uint32_t pe_d1a0(void)
{
    return D_8009D1A0 | PE_LoadU32(GA_D_8009D1A0);
}

void func_80028E94(pe_addr_t actor)
{
    pe_addr_t body=PE_LoadU32(actor),walk;
    uint8_t phase=PE_LoadU8(body+0xACu);
    unsigned i;
    if (phase == 0u) {
        PE_StoreU8(GA_D_8009D2A0,(uint8_t)(PE_LoadU8(GA_D_8009D2A0)-1u));
        phase=1u; PE_StoreU8(body+0xACu,1u);
    }
    if (phase == 1u) {
        if (!PE_LoadU8(body+0xAFu)) {
            func_800866A4(0u,PE_LoadU32(body+8u));
            PE_StoreU8(body+0xACu,3u); return;
        }
        if (PE_LoadU16(actor+0x16u)<PE_LoadU8(body+0x91u) &&
            !PE_LoadU8(actor+14u) && (int8_t)PE_LoadU8(body+5u)<2) return;
        PE_StoreU32(actor+0x98u,PE_LoadU32(actor+0x98u)|0x100u);
        func_8003C5D8(actor+0x1B4u,22);
        PE_StoreU8(body+0xACu,2u); PE_StoreU8(body+0xADu,0u);
        PE_StoreU32(actor+0x68u,0u); PE_StoreU32(actor+0x6Cu,0u); PE_StoreU32(actor+0x70u,0u);
        PE_StoreU32(actor+0x98u,PE_LoadU32(actor+0x98u)|0x1000u);
        func_800866A4(0u,PE_LoadU32(body+8u));
        for (walk=PE_LoadU32(GA_D_8009D20C);walk;walk=PE_LoadU32(walk+4u))
            if (!PE_LoadU32(walk) && PE_LoadU32(walk+0x18Cu)==actor)
                func_8003C5D8(walk+0x1B4u,22);
        return;
    }
    if (phase == 2u) {
        uint8_t color=(uint8_t)(-128-(int)PE_LoadU8(body+0xADu)*16);
        func_8003CAEC(actor+0x1B4u,color,128u,color);
        if (!color) {
            PE_StoreU16(actor+0x250u,PE_LoadU16(actor+0x250u)|2u);
            PE_StoreU8(body+0xACu,3u); PE_StoreU8(body+0xADu,0u);
        } else {
            if (!PE_LoadU8(body+0xADu) && PE_LoadU8(body+0xAEu))
                func_8006DED4(PE_LoadU32(0x800B0E64u),PE_LoadU16(body+0xB4u),0,
                    (int16_t)PE_LoadU16(actor+0x268u),(int16_t)PE_LoadU16(actor+0x26Au),
                    (int16_t)PE_LoadU16(actor+0x26Cu));
            PE_StoreU8(body+0xADu,(uint8_t)(PE_LoadU8(body+0xADu)+1u));
        }
        for (walk=PE_LoadU32(GA_D_8009D20C);walk;walk=PE_LoadU32(walk+4u)) {
            if (PE_LoadU32(walk) || PE_LoadU32(walk+0x18Cu)!=actor) continue;
            func_8003CAEC(walk+0x1B4u,color,128u,color);
            if (!color) PE_StoreU16(walk+0x250u,PE_LoadU16(walk+0x250u)|2u);
        }
        return;
    }
    if (phase!=3u || (PE_LoadU8(actor+0x252u) && PE_LoadU8(body+0xAFu))) return;
    PE_StoreU32(actor+0x98u,PE_LoadU32(actor+0x98u)|0x410u);
    func_8002F970(actor);
    for (walk=PE_LoadU32(GA_D_8009D20C);walk;walk=PE_LoadU32(walk+4u))
        if (!PE_LoadU32(walk) && PE_LoadU32(walk+0x18Cu)==actor)
            PE_StoreU32(walk+0x98u,PE_LoadU32(walk+0x98u)|0x10u);
    PE_StoreU32(0x8009D304u,PE_LoadU32(0x8009D304u)+PE_LoadU16(body+14u));
    {
        int32_t reward=(int32_t)((uint32_t)PE_LoadU16(body+0x98u)-
            (uint32_t)PE_LoadU16(body+0x9Au)*PE_LoadU16(body+0x9Cu));
        if (reward>0) PE_StoreU16(0x8009D21Cu,(uint16_t)(PE_LoadU16(0x8009D21Cu)+(uint32_t)reward));
    }
    for (i=0;i<10;i++) if (!(int16_t)PE_LoadU16(0x800A7FF0u+i*4u)) {
        PE_StoreU16(0x800A7FF0u+i*4u,PE_LoadU8(body+0x9Eu)); break;
    }
    if ((int16_t)PE_LoadU16(body+0xA0u))
        for (i=0;i<10;i++) if (!(int16_t)PE_LoadU16(0x800A7FF0u+i*4u)) {
            PE_StoreU16(0x800A7FF0u+i*4u,PE_LoadU16(body+0xA0u));
            if ((int16_t)PE_LoadU16(body+0xA2u)>=0)
                PE_StoreU16(0x800A7FF2u+i*4u,PE_LoadU16(body+0xA2u));
            break;
        }
    func_800292EC_victory_ready_cut();
}

/*
 * PE-BTL114 — 24A3C is the 17-way at 0x80010824. Entry is
 * 24A3C (lbu D25C), not 24A40. Sole TEXT jal is 22394 @
 * 229D8. Case 9 is the only CE54 writer: wait Aya
 * +0x0F==+0x1A, 1A680((int8)CE48*2+9), CE55=2, CE54=1,
 * body |= 0x2000. Cases 0-8 increment D25C. 6C1CC / 6FC18 /
 * 6F39C / 6DE80 stay deferred. Case 6 waits Aya+0x252==0.
 * That byte is dest+0x9E on the embedded Aya+0x1B4 dest.
 * 35558 jals 3AF14 → 3C818, which sb $0 at dest+0x9E when
 * +0x8C==1. Do not plant the clear.
 */
void func_8001A680(pe_addr_t body, unsigned int id);   /* decomp_hand/absent_lo_actor_port.c */
int func_80024A3C(void)
{
    /* Line-for-line from src/func_80024A3C.c (audit item 3): all 17 phases,
     * including phase 4's func_8006FC18 + func_8006F39C(0x6D), phase 6's
     * reposition/facing + SFX 0x4B6 and phases 10..16.  Case 3 falls
     * through into case 4 exactly as the matched C does. */
    const pe_addr_t tbl = GA_T_800BE830;               /* D_800BE830: {p, id} x 8 bytes */
    const pe_addr_t obj = 0x800B0CECu;                 /* D_800B0CEC (f9C +0x9C, f9E +0x9E) */
    int r = 0;
    pe_addr_t p, e, q;
    int32_t s, t2;
    uint32_t k;
#define AYA        PE_LoadU32(GA_D_8009D254)
#define PH()       PE_LoadU8(GA_D_8009D25C)
#define PH_INC()   PE_StoreU8(GA_D_8009D25C, (uint8_t)(PH() + 1u))
#define TIMER()    ((int16_t)PE_LoadU16(GA_D_8009CE4C))
#define TIMER_DEC() PE_StoreU16(GA_D_8009CE4C, (uint16_t)(TIMER() - 1))
#define CE48()     ((int8_t)PE_LoadU8(GA_D_8009CE48))
#define OR16(a, v) PE_StoreU16((a), (uint16_t)(PE_LoadU16(a) | (v)))
#define ENT(i)     PE_LoadU32(tbl + (uint32_t)(i) * 8u)
#define D1D4()     PE_LoadU8(GA_D_8009D1D4)
#define FRAME_DONE(pp, off) (PE_LoadU8((pp) + 0xFu) == PE_LoadU16((pp) + (off)))

    switch (PH()) {
    case 0:
        (void)func_8006C1CC(1);
        p = AYA;
        PE_StoreU32(0x8009E054u, PE_LoadU32(p + 0x28u));
        PE_StoreU32(0x8009E058u, PE_LoadU32(p + 0x2Cu));
        PE_StoreU32(0x8009E05Cu, PE_LoadU32(p + 0x30u));
        PE_StoreU16(0x8009CE58u, PE_LoadU16(p + 0x38u));
        PE_StoreU16(0x8009CE5Au, PE_LoadU16(p + 0x3Au));
        PE_StoreU8(GA_D_8009CE48, 0u);
        PE_StoreU16(0x8009CE5Cu, PE_LoadU16(p + 0x3Cu));
        PE_StoreU32(0x8009D2E8u, PE_LoadU32(0x8009D2E8u) & ~4u);
        PE_StoreU32(p + 0x98u, PE_LoadU32(p + 0x98u) | 0x80u);
        func_8003C5D8(p + 0x1B4u, 0x1E);
        OR16(AYA + 0x250u, 2u);
        func_8003C5D8(obj, 0x1E);
        OR16(obj + 0x9Cu, 2u);
        (void)func_800702DC();
        PE_StoreU32(GA_D_8009D258, (uint32_t)func_8006F39C(0x6Bu, AYA));
        PH_INC();
        break;
    case 1:
        (void)func_8006C1CC(1);
        if (PE_LoadU8(AYA + 0x252u) != 0u) break;
        if (PE_LoadU8(obj + 0x9Eu) != 0u) break;
        PH_INC();
        break;
    case 2:
        if (func_8006C1CC(1) != 0) break;
        func_8001A680(AYA, 5u);
        p = AYA;
        PE_StoreU8(p + 0x252u, 1u);
        PE_StoreU32(p + 0x98u, PE_LoadU32(p + 0x98u) | 0x100u);
        func_8003C5D8(AYA + 0x1B4u, 0x1E);
        OR16(AYA + 0x250u, 4u);
        (void)func_8006F39C(0x6Cu, AYA);
        PE_StoreU16(GA_D_8009CE4C, 0x1Eu);
        PH_INC();
        break;
    case 3:
        if (TIMER() == 0) {
            PH_INC();
            PE_StoreU32(AYA + 0x98u, PE_LoadU32(AYA + 0x98u) & ~0x100u);
        } else {
            TIMER_DEC();
        }
        /* fall through (matched C has no break) */
    case 4:
        p = AYA;
        if (!FRAME_DONE(p, 0x16u)) break;
        for (q = PE_LoadU32(GA_D_8009D20C); q != 0u; q = PE_LoadU32(q + 4u)) {
            if (q != p && PE_LoadU32(q) != 0u) {
                PE_StoreU32(q + 0x68u, 0u); PE_StoreU32(q + 0x6Cu, 0u); PE_StoreU32(q + 0x70u, 0u);
            }
        }
        (void)func_8006FC18(PE_LoadU32(GA_D_8009D258), AYA, 0u);
        (void)func_8006F39C(0x6Du, AYA);
        PH_INC();
        break;
    case 5:
        func_8001A680(AYA, 6u);
        func_8003C5D8(AYA + 0x1B4u, 0xF);
        PH_INC();
        OR16(AYA + 0x250u, 2u);
        break;
    case 6:
        p = AYA;
        if (PE_LoadU8(p + 0x252u) != 0u) break;
        e = ENT(D1D4());
        s = (int32_t)(int16_t)PE_LoadU16(e + 0x224u) *
            (int32_t)((PE_LoadU32(PE_LoadU32(e) + 0xCCu) >> 19) & 0x1Fu) / 10 +
            (int32_t)(int16_t)PE_LoadU16(p + 0x224u);
        t2 = func_80030584(e + 0x1B4u, 0x8009E054u);
        PE_StoreU16(AYA + 0x3Au, (uint16_t)t2);
        t2 = func_80077CF4((int16_t)t2);
        PE_StoreU32(AYA + 0x28u, ((uint32_t)(int32_t)(int16_t)PE_LoadU16(ENT(D1D4()) + 0x268u) << 16) +
                                 ((uint32_t)(s * t2) << 4));
        t2 = func_80077DC4((int16_t)PE_LoadU16(AYA + 0x3Au));
        q = AYA;
        PE_StoreU32(q + 0x30u, ((uint32_t)(int32_t)(int16_t)PE_LoadU16(ENT(D1D4()) + 0x26Cu) << 16) +
                               ((uint32_t)(s * t2) << 4));
        PE_StoreU32(q + 0x40u, PE_LoadU32(q + 0x28u));
        PE_StoreU32(q + 0x44u, PE_LoadU32(q + 0x2Cu));
        PE_StoreU32(q + 0x48u, PE_LoadU32(q + 0x30u));
        (void)func_8006DE80(0x4B6, 0, (short)PE_LoadU16(q + 0x2Au), (short)PE_LoadU16(q + 0x2Eu),
                            (short)PE_LoadU16(q + 0x32u));
        PE_StoreU16(GA_D_8009CE4C, 0x1Eu);
        PH_INC();
        break;
    case 7:
        p = AYA;
        if (FRAME_DONE(p, 0x1Au)) {
            func_8001A680(p, 7u);
            PE_StoreU32(AYA + 0x14u, (uint32_t)PE_LoadU8(AYA + 0xFu) << 15);
        }
        if (TIMER() != 0) { TIMER_DEC(); break; }
        PE_StoreU8(AYA + 0x252u, 1u);
        func_8003C5D8(AYA + 0x1B4u, 0xF);
        PH_INC();
        OR16(AYA + 0x250u, 4u);
        break;
    case 8:
        p = AYA;
        if (!FRAME_DONE(p, 0x1Au)) break;
        {
            uint16_t v = (uint16_t)(PE_LoadU16(p + 0x250u) | 0x20u);
            uint16_t a = (uint16_t)(CE48() * 2 + 8);
            PE_StoreU16(p + 0x250u, v);
            func_8001A680(p, a);
        }
        PH_INC();
        break;
    case 9:
        p = AYA;
        if (!FRAME_DONE(p, 0x1Au)) break;
        func_8001A680(p, (uint16_t)(CE48() * 2 + 9));
        k = D1D4();
        PE_StoreU8(GA_D_8009CE55, 2u);
        PE_StoreU8(GA_D_8009CE54, 1u);
        q = PE_LoadU32(ENT(k));                        /* W(D_800BE830[k].p, 0) */
        {
            uint32_t w = PE_LoadU32(q);
            w = (w & ~(3u << 13)) | (1u << 13);        /* Bits.a = 1 */
            PE_StoreU32(q, w);
            w = (PE_LoadU32(q) & 0xFFF3FFFFu) |
                (((PE_LoadU32(PE_LoadU32(PE_LoadU32(0x8009D278u) + 0x68u) + 0xCu) >> 20) & 3u) << 18);
            PE_StoreU32(q, w);
            w = (PE_LoadU32(q) & ~(7u << 15)) | (((uint32_t)(int8_t)PE_LoadU8(GA_D_8009CE55) & 7u) << 15);
            PE_StoreU32(q, w);                         /* Bits.c = D_8009CE55 */
        }
        if (CE48() == 2) (void)func_8006F39C(0x6Fu, ENT(k));
        else if (CE48() == 5) (void)func_8006F39C(0x71u, ENT(k));
        else if (CE48() == 6) (void)func_8006F39C(0x70u, ENT(k));
        else (void)func_8006F39C(0x6Eu, ENT(k));
        PE_StoreU8(GA_D_8009CE48, (uint8_t)(CE48() + 1));
        if (CE48() < 7 && ENT(D1D4()) == ENT(D1D4() + 1u)) {
            PE_StoreU8(GA_D_8009D25C, 8u);
            PE_StoreU8(GA_D_8009D1D4, (uint8_t)(D1D4() + 1u));
            break;
        }
        PH_INC();
        break;
    case 10:
        p = AYA;
        if (!FRAME_DONE(p, 0x1Au)) break;
        func_8001A680(p, 7u);
        if (CE48() < 7) {
            PE_StoreU8(GA_D_8009D25C, 5u);
            PE_StoreU8(GA_D_8009D1D4, (uint8_t)(D1D4() + 1u));
            break;
        }
        func_8003C5D8(AYA + 0x1B4u, 0xF);
        PH_INC();
        OR16(AYA + 0x250u, 2u);
        break;
    case 11:
        q = AYA;
        if (PE_LoadU8(q + 0x252u) != 0u) break;
        PE_StoreU32(q + 0x28u, PE_LoadU32(0x8009E054u));
        PE_StoreU32(q + 0x2Cu, PE_LoadU32(0x8009E058u));
        PE_StoreU32(q + 0x30u, PE_LoadU32(0x8009E05Cu));
        PE_StoreU32(q + 0x40u, PE_LoadU32(0x8009E054u));
        PE_StoreU32(q + 0x44u, PE_LoadU32(0x8009E058u));
        PE_StoreU32(q + 0x48u, PE_LoadU32(0x8009E05Cu));
        PE_StoreU16(q + 0x38u, PE_LoadU16(0x8009CE58u));
        PE_StoreU16(q + 0x3Au, PE_LoadU16(0x8009CE5Au));
        PE_StoreU16(q + 0x3Cu, PE_LoadU16(0x8009CE5Cu));
        (void)func_8006DE80(0x4B6, 0, (short)PE_LoadU16(q + 0x2Au), (short)PE_LoadU16(q + 0x2Eu),
                            (short)PE_LoadU16(q + 0x32u));
        PE_StoreU16(GA_D_8009CE4C, 0x1Eu);
        PH_INC();
        break;
    case 12:
        if (TIMER() != 0) { TIMER_DEC(); break; }
        func_8001A680(AYA, 7u);
        PE_StoreU8(AYA + 0x252u, 1u);
        func_8003C5D8(AYA + 0x1B4u, 0x1E);
        PE_StoreU16(0x801F1F38u, 1u);
        PE_StoreU16(GA_D_8009CE4C, 0xFu);
        PH_INC();
        OR16(AYA + 0x250u, 4u);
        break;
    case 13:
        if (TIMER() != 0) { TIMER_DEC(); break; }
        p = AYA;
        if (!FRAME_DONE(p, 0x1Au)) break;
        func_8001A680(p, 4u);
        PH_INC();
        break;
    case 14:
        p = AYA;
        if (!FRAME_DONE(p, 0x16u)) break;
        PE_StoreU32(p + 0x98u, PE_LoadU32(p + 0x98u) | 0x100u);
        func_8003C5D8(p + 0x1B4u, 0xF);
        OR16(AYA + 0x250u, 2u);
        (void)func_8006F39C(0x72u, AYA);
        PH_INC();
        break;
    case 15:
        if (PE_LoadU8(AYA + 0x252u) != 0u) break;
        if (func_8006C1CC(0) != 0) break;
        func_8001A680(AYA, PE_LoadU8(PE_LoadU32(0x8009D278u) + 0x12u));
        PE_StoreU32(0x8009D2E8u, PE_LoadU32(0x8009D2E8u) | 4u);
        p = AYA;
        PE_StoreU8(p + 0x252u, 1u);
        PE_StoreU32(p + 0x98u, PE_LoadU32(p + 0x98u) & ~0x100u);
        PE_StoreU32(p + 0x98u, PE_LoadU32(p + 0x98u) & ~0x80u);
        func_8003C5D8(AYA + 0x1B4u, 0x1E);
        OR16(AYA + 0x250u, 4u);
        PE_StoreU8(obj + 0x9Eu, 1u);
        func_8003C5D8(obj, 0x1E);
        PE_StoreU16(GA_D_8009CE4C, 0xFu);
        OR16(obj + 0x9Cu, 4u);
        PH_INC();
        break;
    case 16:
        if (TIMER() == 0) {
            D_8009D1A0 &= ~0x100u;
            PE_StoreU8(GA_D_8009D1D4, (uint8_t)(D1D4() + 1u));
            r = 1;
        } else {
            TIMER_DEC();
        }
        break;
    default:
        break;
    }
#undef AYA
#undef PH
#undef PH_INC
#undef TIMER
#undef TIMER_DEC
#undef CE48
#undef OR16
#undef ENT
#undef D1D4
#undef FRAME_DONE
    return (int)(int8_t)r;
}

/* Original target pulse reset. Command completion is in26824_port.c. */
#define GA_D_8009D2B0 0x8009D2B0u

/* func_80026FD0: ported from the matching decomp -- generated TU pc_port/game/decomp/func_80026FD0_port.c (src/func_80026FD0.c); hand port retired after differential verification (docs/evidence/portverify/REPORT.md). */

static void pe_27d14_atb(pe_addr_t actor, pe_addr_t body)
{
    uint16_t cur;
    uint16_t rate;

    /* s1 stays *actor. ATB is body+0x0C / +0x8E. */
    cur = PE_LoadU16(body + 0x0Cu);
    rate = PE_LoadU16(body + 0x8Eu);
    if (cur < 0x2328u) {
        uint16_t sum = (uint16_t)(cur + rate);

        if ((PE_LoadU32(body) & 1u) != 0u)
            sum = (uint16_t)((int)sum - ((int)rate * 2) / 5);
        PE_StoreU16(body + 0x0Cu, sum);
        return;
    }
    PE_StoreU16(body + 0x0Cu, 0x2328u);
    if ((PE_LoadU32(actor + 0x98u) & 0x1000u) != 0u)
        PE_StoreU16(body + 0x0Cu, 0u);
}

static void pe_27d14_dot(pe_addr_t body)
{
    uint32_t word;
    unsigned int shift;
    int32_t hp;
    int16_t tick;

    word = PE_LoadU32(body);
    if ((word & 0x10u) == 0u)
        return;
    shift = (word >> 5) & 0x1Fu;
    if (shift < 0x1Eu) {
        shift = (shift + 1u) & 0x1Fu;
        word = (word & 0xFFFFFC1Fu) | (shift << 5);
        PE_StoreU32(body, word);
        return;
    }
    tick = (int16_t)PE_LoadU16(body + 0x96u);
    hp = (int32_t)(PE_LoadU32(body + 0x10u) - (uint32_t)(int32_t)tick);
    PE_StoreU32(body + 0x10u, (uint32_t)hp);
    PE_StoreU32(body, word & 0xFFFFFC1Fu);
}

/* 36254 restores each of an actor's three script lists from saved PCs. */
void func_80036254(pe_addr_t actor)
{
    unsigned i;
    for (i=0;i<3;i++) {
        pe_addr_t task=PE_LoadU32(actor+0xA0u+i*4u);
        for (;task;task=PE_LoadU32(task+0x24u)) {
            pe_addr_t pc=PE_LoadU32(task+4u);
            if (!pc) continue;
            PE_StoreU32(task,pc);PE_StoreU32(task+0x10u,1u);
            PE_StoreU16(task+8u,PE_LoadU16(task+8u)&0xFF9Fu);
        }
    }
}

static void pe_27d14_stop(pe_addr_t actor)
{
    PE_StoreU32(actor+0x68u,0u);PE_StoreU32(actor+0x6Cu,0u);PE_StoreU32(actor+0x70u,0u);
}

static void pe_27d14_release_status(pe_addr_t actor, pe_addr_t body)
{
    pe_addr_t action;
    PE_StoreU32(actor+0x98u,PE_LoadU32(actor+0x98u)&~0x1000u);
    func_80036254(actor);
    action=PE_LoadU32(body+0x18u);
    if (action) {
        PE_StoreU8(action,4u);
        PE_StoreU32(body,PE_LoadU32(body)&0xC0FFFFFFu);
    }
}

static void pe_27d14_reaction(pe_addr_t actor, pe_addr_t body)
{
    int8_t kind=(int8_t)PE_LoadU8(body+5u);
    uint32_t flags=PE_LoadU32(body);
    if (!(flags&14u)) {
        if (kind) return;
        if (PE_LoadU8(actor+15u)==PE_LoadU16(actor+26u)) {
            if (PE_LoadU8(body+0xBCu)==2u && !(flags&0x1800u)) {
                PE_StoreU8(body+0xBCu,1u);
                func_8001A680_command_cut(actor,PE_LoadU8(body+0xBDu));
                if (PE_LoadU8(body+0xBEu)) PE_StoreU32(actor+0x98u,PE_LoadU32(actor+0x98u)|0x200u);
                PE_StoreU32(actor+0x14u,PE_LoadU32(body+0xC0u));
                PE_StoreU32(actor+0x18u,PE_LoadU32(body+0xC4u));
                PE_StoreU32(actor+0x1Cu,PE_LoadU32(body+0xC8u));
            } else func_8001A680_command_cut(actor,(uint16_t)(int16_t)(int8_t)PE_LoadU8(body+6u));
            PE_StoreU32(body,PE_LoadU32(body)&~BODY_HITMASK);
            if (!(PE_LoadU32(PE_LoadU32(GA_D_8009D278)+0x4Cu)&0x80000u) &&
                !(PE_LoadU32(body)&0x1800u)) {
                PE_StoreU32(actor+0x98u,PE_LoadU32(actor+0x98u)&~0x1000u);
                func_80036254(actor);
            }
        } else if (PE_LoadU8(body+0xA4u)>=2u && PE_LoadU8(body+0xA5u)) {
            int32_t angle=(int16_t)PE_LoadU16(body+0xA8u);
            uint32_t distance=PE_LoadU16(body+0xA6u);
            PE_StoreU32(actor+40u,PE_LoadU32(actor+40u)+((distance*(uint32_t)func_80077CF4(angle))<<4u));
            PE_StoreU32(actor+48u,PE_LoadU32(actor+48u)+((distance*(uint32_t)func_80077DC4(angle))<<4u));
            PE_StoreU8(body+0xA5u,(uint8_t)(PE_LoadU8(body+0xA5u)-1u));
        }
        return;
    }
    PE_StoreU32(body,PE_LoadU32(body)&~BODY_HITMASK);
    if (!kind || (int32_t)PE_LoadU32(body+16u)<=0) return;
    if (kind==1) actor=PE_LoadU32(actor+0x18Cu);
    else if (kind==4) {
        pe_addr_t walk;
        for (walk=PE_LoadU32(GA_D_8009D20C);walk;walk=PE_LoadU32(walk+4u)) {
            pe_addr_t other=PE_LoadU32(walk);
            if (walk!=PE_LoadU32(GA_D_8009D254) && other && (int8_t)PE_LoadU8(other+5u)==4)
                PE_StoreU16(walk+0x250u,PE_LoadU16(walk+0x250u)|0x20u);
        }
        return;
    }
    PE_StoreU16(actor+0x250u,PE_LoadU16(actor+0x250u)|0x20u);
}

/* Full 27D14, including timed status release, hit flashes, damage text,
 * linked-actor defeat and movement guards. Authority: 120D8.s. */
void func_80027D14(pe_addr_t actor)
{
    pe_addr_t body=PE_LoadU32(actor),walk;
    uint32_t flags,bits;
    int32_t delta;
    int8_t kind;
    if ((int32_t)PE_LoadU32(body+0x88u)<(int32_t)PE_LoadU32(body+16u))
        PE_StoreU32(body+16u,PE_LoadU32(body+0x88u));
    if (!(pe_d1a0()&0x100u)) {
        if (!PE_LoadU16(body+12u)) {
            flags=PE_LoadU32(body);
            if (flags&14u) {
                flags=(flags&~14u)|((((flags>>1u)-1u)&7u)<<1u);PE_StoreU32(body,flags);
                if (!(flags&0x180Eu)) pe_27d14_release_status(actor,body);
            }
            flags=PE_LoadU32(body);
            if (flags&0x1800u) {
                flags=(flags&~0x1800u)|((((flags>>11u)-1u)&3u)<<11u);PE_StoreU32(body,flags);
                if (!(flags&0x180Eu)) pe_27d14_release_status(actor,body);
            }
        }
        pe_27d14_atb(actor,body);pe_27d14_dot(body);
    }
    if (PE_LoadU32(body)&BODY_HITMASK) {
        (void)func_80027A08(actor);
        bits=PE_LoadU32(body)&BODY_HITMASK;
        if (bits==BODY_HIT) {
            func_80028574(actor);
            func_8006DCE4(PE_LoadU16(body+0xB0u),0u,(int16_t)PE_LoadU16(actor+0x268u),
                (int16_t)PE_LoadU16(actor+0x26Au),(int16_t)PE_LoadU16(actor+0x26Cu));
        } else if (bits==BODY_REACT) pe_27d14_reaction(actor,body);
        if ((int8_t)PE_LoadU8(body+5u)!=1) pe_27d14_stop(actor);
    }
    if ((PE_LoadU32(body)&14u) && (int32_t)PE_LoadU32(body+16u)>0) {
        pe_27d14_stop(actor);
        if (!(pe_d1a0()&0x100u))
            PE_StoreU16(actor+0x3Au,(uint16_t)(((int16_t)PE_LoadU16(actor+0x3Au)+128)%4096));
    }
    if (PE_LoadU32(body)&0x1800u) pe_27d14_stop(actor);
    if (PE_LoadU32(body)&0x400u) PE_StoreU32(body+16u,0xFFFFFFFFu);
    delta=(int32_t)(PE_LoadU32(body+20u)-PE_LoadU32(body+16u));
    if (delta) {
        PE_StoreU16(body+0xD0u,(uint16_t)(delta<0?0u-(uint32_t)delta:(uint32_t)delta));
        PE_StoreU8(body+0xD7u,delta<0?1u:(PE_LoadU32(body)&0x38000u)==0x18000u?2u:0u);
        PE_StoreU16(body+0xD2u,PE_LoadU16(actor+0x218u));
        PE_StoreU16(body+0xD4u,(uint16_t)(PE_LoadU16(actor+0x21Au)-20u));
        PE_StoreU8(body+0xD6u,30u);
    }
    if (PE_LoadU8(body+0xD6u)) {
        func_80032B0C(1u,body+0xD0u);
        PE_StoreU8(body+0xD6u,(uint8_t)(PE_LoadU8(body+0xD6u)-1u));
    }
    if ((int32_t)PE_LoadU32(body+16u)<=0 && !(pe_d1a0()&0x100u)) {
        kind=(int8_t)PE_LoadU8(body+5u);
        if (kind==1 && !(PE_LoadU32(body)&BODY_HITMASK)) {
            PE_StoreU32(actor+0x98u,PE_LoadU32(actor+0x98u)|0x10u);func_8002F970(actor);
        }
        if (kind==4) {
            if (!(PE_LoadU32(body)&BODY_HITMASK)) { pe_27d14_stop(actor);func_80028E94(actor); }
        } else if (kind!=1 && kind!=3) { pe_27d14_stop(actor);func_80028E94(actor); }
        else {
            for (walk=PE_LoadU32(GA_D_8009D20C);walk;walk=PE_LoadU32(walk+4u)) {
                pe_addr_t other=PE_LoadU32(walk);
                int8_t k;
                if (!other || walk==PE_LoadU32(GA_D_8009D254)) continue;
                k=(int8_t)PE_LoadU8(other+5u);
                if (k && k!=2 && k!=4 && (int32_t)PE_LoadU32(other+16u)>0) break;
            }
            if (!walk) {
                if (kind==1) {
                    PE_StoreU32(actor+0x98u,PE_LoadU32(actor+0x98u)|0x10u);func_8002F970(actor);
                    actor=PE_LoadU32(actor+0x18Cu);
                }
                pe_27d14_stop(actor);func_80028E94(actor);
            }
        }
    }
    PE_StoreU32(body+20u,PE_LoadU32(body+16u));
}
