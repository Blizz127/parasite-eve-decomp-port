/*
 * PE-BTL11 — func_80035558 D20C walk cut, type!=0 vtable 35E04,
 * and 361F4 slot publish (translated retail, not matching src/).
 *
 * Authority: build/disc1.candidate.exe SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 *
 * func_80035558 — 459 words 0x80035558..0x80035C84, SHA-256
 * 7352fc04…ba83. Sole TEXT caller 3F3C4 @ 0x8003F4F0 (field
 * tick; 3F3C4 sole caller 0x800123D8). This cut is the prologue
 * only:
 *
 *   if (D_8009D1A0 & 4) return;   # ROM skips walk and continues
 *   for actor in D_8009D20C via +4:
 *       jalr actor+0x190 (a0=actor)
 *
 * Known jalr targets 0x80035E04 (types 1–9) and 0x80035C84
 * (type 0) are ported below. PE-BTL67 wires 35C84 jal 3999C
 * when D2E8 bit 0 is clear (0x3F). PE-BTL68 indexes
 * table[*codep] (not actor+0) and jalrs 710A4/7136C.
 * After the walk, this cut also takes the live D1A0&2 jal
 * 299CC @ 0x800355E8 (consume + after-consume idle gate) and
 * the 35B2C jal 69594 event pump. PE-BTL66 adds the post-69594
 * 1A4AC clip ticks @ 35B84 (D254 when D1A0&0x100) and 35BEC
 * (D20C walk when that bit is clear; skip +0x98 & 0x800040).
 * PE-BTL124 takes 35C2C jal 360B4's proven +0x98
 * `and 0xFF7FFFFF` (clear the 1A4AC-this-frame bit) and the
 * 35C34 walk `and 0xEFFFFFFF`. SEW18 adds the full 36448
 * actor-contact pass and 12774 task retirement. The full 360B4
 * actor retirement now releases effects and unlinks dead actors. 661CC @
 * 35B34 restores the projection center after effect updates. PE-BTL120 takes the dest tick at
 * 35AE0: jal 3AF14(actor+0x1B4) when +0x98 bit 0x40 is
 * clear. 3AC90 / 3A6A8 / 6698C / 68014 stay deferred. Not M2.
 *
 * func_80035E04 — 83 words 0x80035E04..0x80035F50. If D1A0 bit
 * 0x100, only 361F4. Else snapshot +0x28/+0x38 into +0x40/+0x50,
 * jal 361F4, then integrate +0x68/+0x78/+0x58 into pose.
 * +0x98 bit 1 only adds +0x88 first; it does not skip pose.
 *
 * func_800361F4 — 24 words 0x800361F4..0x80036254. sw actor →
 * gp+0x580 (D_8009D2F0), then three words at +0xA0 into
 * D_8009D300. Nonempty slots jal 17018. Live 35038 zeros
 * +0xA0/+0xA4 and stores the 12700 task at +0xA8.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include <stdio.h>
#include <stdlib.h>
#include "game_port.h"
#include "pe_sdk.h"
#include "pe_guestcode.h"
#include "pe_plat/enhance.h"
int32_t func_80068014(pe_addr_t actor);

#define GA_D_8009D20C 0x8009D20Cu
#define GA_D_8009D254 0x8009D254u
#define GA_D_8009D2F0 0x8009D2F0u
#define GA_D_8009D300 0x8009D300u
#define GA_D_8009D2E8 0x8009D2E8u
#define GA_D_8009D26C 0x8009D26Cu
#define GA_D_8009D1F4 0x8009D1F4u
#define GA_D_8009D1E4 0x8009D1E4u
#define GA_D_800B0CD8 0x800B0CD8u
#define GA_D_800943C0 0x800943C0u
#define GA_D_800BD000 0x800BD000u
#define GA_D_800BD020 0x800BD020u
#define GA_D_800BE9A0 0x800BE9A0u
#define GA_VT_35E04   0x80035E04u
#define GA_VT_35C84   0x80035C84u
#define GA_FN_7136C   0x8007136Cu
#define GA_FN_710A4   0x800710A4u
#define GA_WALK_VEL   0x801221A0u
#define CMD_RTIR_SF0  0x041E012u
#define CMD_RTIR_SF1  0x049E012u

extern void func_8001A4AC(pe_addr_t actor);

static void pe_actor_snapshot_pose(pe_addr_t actor)
{
    PE_StoreU32(actor + 0x40u, PE_LoadU32(actor + 0x28u));
    PE_StoreU32(actor + 0x44u, PE_LoadU32(actor + 0x2Cu));
    PE_StoreU32(actor + 0x48u, PE_LoadU32(actor + 0x30u));
    PE_StoreU16(actor + 0x50u, PE_LoadU16(actor + 0x38u));
    PE_StoreU16(actor + 0x52u, PE_LoadU16(actor + 0x3Au));
    PE_StoreU16(actor + 0x54u, PE_LoadU16(actor + 0x3Cu));
}

static void pe_actor_integrate_motion(pe_addr_t actor)
{
    /* ROM 35C84/35E04: bit 1 only adds +0x88/+0x8C/+0x90.
     * Pose += +0x68/+0x78/+0x58 always. BTL11 skip-all is REJECTED. */
    if ((PE_LoadU32(actor + 0x98u) & 2u) != 0u) {
        PE_StoreU32(actor + 0x68u,
                    PE_LoadU32(actor + 0x68u) + PE_LoadU32(actor + 0x88u));
        PE_StoreU32(actor + 0x6Cu,
                    PE_LoadU32(actor + 0x6Cu) + PE_LoadU32(actor + 0x8Cu));
        PE_StoreU32(actor + 0x70u,
                    PE_LoadU32(actor + 0x70u) + PE_LoadU32(actor + 0x90u));
    }
    PE_StoreU32(actor + 0x68u,
                PE_LoadU32(actor + 0x68u) + PE_LoadU32(actor + 0x78u));
    PE_StoreU32(actor + 0x6Cu,
                PE_LoadU32(actor + 0x6Cu) + PE_LoadU32(actor + 0x7Cu));
    PE_StoreU32(actor + 0x70u,
                PE_LoadU32(actor + 0x70u) + PE_LoadU32(actor + 0x80u));
    PE_StoreU32(actor + 0x28u,
                PE_LoadU32(actor + 0x28u) + PE_LoadU32(actor + 0x68u));
    PE_StoreU32(actor + 0x2Cu,
                PE_LoadU32(actor + 0x2Cu) + PE_LoadU32(actor + 0x6Cu));
    PE_StoreU32(actor + 0x30u,
                PE_LoadU32(actor + 0x30u) + PE_LoadU32(actor + 0x70u));
    PE_StoreU32(actor + 0x28u,
                PE_LoadU32(actor + 0x28u) + PE_LoadU32(actor + 0x58u));
    PE_StoreU32(actor + 0x2Cu,
                PE_LoadU32(actor + 0x2Cu) + PE_LoadU32(actor + 0x5Cu));
    PE_StoreU32(actor + 0x30u,
                PE_LoadU32(actor + 0x30u) + PE_LoadU32(actor + 0x60u));
}



void func_800361F4(pe_addr_t actor)
{
    unsigned int i;
    uint32_t slot;

    if (PE_Port_ShouldStop()) return;
    PE_StoreU32(GA_D_8009D2F0, actor);
    for (i = 0; i < 3u; i++) {
        slot = PE_LoadU32(actor + 0xA0u + i * 4u);
        PE_StoreU32(GA_D_8009D300, slot);
        if (slot != 0u) {
            func_80017018();
            if (PE_Port_ShouldStop()) return;
        }
    }
}

void func_80035E04(pe_addr_t actor)
{
    if (PE_Port_ShouldStop()) return;
    if (D_8009D1A0 & 0x100u) {
        func_800361F4(actor);
        return;
    }

    pe_actor_snapshot_pose(actor);
    func_800361F4(actor);
    if (PE_Port_ShouldStop()) return;
    pe_actor_integrate_motion(actor);
}

/* func_80078934: replaced by pe_plat_gte_apply_matrix_lv (pe_plat_gte_lv.c, clean-room) */

static void pe_walk_digital_apply(pe_addr_t actor, uint32_t speed)
{
    uint32_t bits;
    uint16_t yaw;
    uint16_t facing;
    uint32_t diag;
    uint32_t vx;
    uint32_t vy;
    uint32_t vz;
    pe_addr_t rec;

    if ((PE_LoadU16(GA_D_800BE9A0) & 0xF000u) == 0x7000u)
        return;

    diag = func_8003708C(speed, 0xB504u);
    bits = PE_LoadU32(GA_D_8009D26C);
    yaw = PE_LoadU16(GA_D_800BD020);
    facing = yaw;
    vx = 0u;
    vy = 0u;
    vz = 0u;
    if ((bits & 8u) != 0u) {
        if ((bits & 0x40u) != 0u) {
            facing = (uint16_t)(yaw + 0x600u);
            vx = (uint32_t)-(int32_t)diag;
            vz = diag;
        } else if ((bits & 0x10u) != 0u) {
            facing = (uint16_t)(yaw + 0xA00u);
            vx = diag;
            vz = diag;
        } else {
            facing = (uint16_t)(yaw + 0x800u);
            vz = speed;
        }
    } else if ((bits & 0x20u) != 0u) {
        if ((bits & 0x40u) != 0u) {
            facing = (uint16_t)(yaw + 0x200u);
            vx = (uint32_t)-(int32_t)diag;
            vz = (uint32_t)-(int32_t)diag;
        } else if ((bits & 0x10u) != 0u) {
            facing = (uint16_t)(yaw + 0xE00u);
            vx = diag;
            vz = (uint32_t)-(int32_t)diag;
        } else {
            facing = yaw;
            vz = (uint32_t)-(int32_t)speed;
        }
    } else if ((bits & 0x40u) != 0u) {
        facing = (uint16_t)(yaw + 0x400u);
        vx = (uint32_t)-(int32_t)speed;
    } else if ((bits & 0x10u) != 0u) {
        facing = (uint16_t)(yaw + 0xC00u);
        vx = speed;
    }

    rec = PE_LoadU32(GA_D_8009D254);
    if (rec != 0u && (PE_LoadU32(GA_D_8009D2E8) & 0x10u) != 0u) {
        rec = PE_LoadU32(rec);
        facing = (uint16_t)(facing + 0x800u);
        if (rec != 0u)
            facing = (uint16_t)(facing
                                + ((PE_LoadU32(rec + 0x4Cu) >> 7) & 0xC00u));
    }
    facing &= 0xFFFu;
    PE_StoreU32(GA_WALK_VEL, vx);
    PE_StoreU32(GA_WALK_VEL + 4u, vy);
    PE_StoreU32(GA_WALK_VEL + 8u, vz);
    PE_StoreU16(actor + 0x3Au, facing);
    func_80078934(GA_D_800BD000, GA_WALK_VEL, actor + 0x68u);
    if ((PE_LoadU32(GA_D_8009D2E8) & 0x10u) != 0u) {
        PE_StoreU16(actor + 0x3Au,
                    (uint16_t)((0x1000u - PE_LoadU16(actor + 0x3Au))
                               & 0xFFFu));
        PE_StoreU32(actor + 0x68u,
                    (uint32_t)-(int32_t)PE_LoadU32(actor + 0x68u));
    }
}

/*
 * PE-BTL68 — func_8007136C walk core.
 * 206 words 0x8007136C..0x800716A4, SHA-256 below in oracle.
 * Analog (BE9A0&0xF000==0x7000) is not this cut.
 */
int32_t func_80079FB4(int32_t a0, int32_t a1);   /* generated TU */
void func_8001A680(pe_addr_t body, unsigned int id);   /* decomp_hand/absent_lo_actor_port.c */
void func_8007136C(pe_addr_t actor, pe_addr_t codep)
{
    /* Audit item 18: line-for-line src/func_8007136C.c, including the
     * analog-stick path (pad type D_800BE9A0 & 0xF000 == 0x7000) and the
     * D_8009D2E8 & 0x10 heading adjust.  `v` (a stack VEC in retail) lives in
     * GA_WALK_VEL; like retail, a digital frame with no direction bits leaves
     * it as it was. */
    int32_t sp = 0x168000, d, ang, r, a;
    uint32_t f, g;
    pe_addr_t rec;

    if (D_8009D1A0 & 2u) {
        rec = PE_LoadU32(actor);
        g = PE_LoadU32(rec + 0x4Cu);
        if ((g & 0xC0u) == 0x40u) sp = 0xB4000;
        else if (g & 0x100u) sp = 0x21C000;
        if (PE_LoadU32(codep) != PE_LoadU8(PE_LoadU32(actor) + 0x13u)) {
            func_8001A680(actor, PE_LoadU8(PE_LoadU32(actor) + 0x13u));
            PE_StoreU32(codep, PE_LoadU8(PE_LoadU32(actor) + 0x13u));
        }
    } else if (PE_LoadU32(codep) != 0x17u) {
        func_8001A680(actor, 0x17u);
        PE_StoreU32(codep, 0x17u);
    }
    sp = (int32_t)func_8003708C((uint32_t)sp, PE_LoadU32(actor + 0x20u));
    sp = (int32_t)func_8003708C((uint32_t)sp, (uint32_t)PE_LoadU16(actor + 0x26u) << 4);
    /* Opt-in enhancement (pe_plat/enhance.h): Aya's field run speed.  At the
     * default 100 % this returns sp unchanged, so the retail path runs. */
    sp = pe_enh_field_speed(actor, sp);
    if ((PE_LoadU16(GA_D_800BE9A0) & 0xF000u) == 0x7000u) {
        ang = func_80079FB4((int32_t)PE_LoadU8(0x800BE9A7u) - 0x80, (int32_t)PE_LoadU8(0x800BE9A6u) - 0x80) - 0x400;
        if (ang < 0) ang += 0x1000;
        PE_StoreU16(actor + 0x3Au, (uint16_t)(PE_LoadU16(0x800BD022u) + (uint32_t)ang));
        d = func_80077CF4(ang);
        sp = -sp;
        PE_StoreU32(GA_WALK_VEL + 0u, func_8003708C((uint32_t)sp, (uint32_t)d << 4));
        PE_StoreU32(GA_WALK_VEL + 4u, 0u);
        PE_StoreU32(GA_WALK_VEL + 8u, func_8003708C((uint32_t)sp, (uint32_t)func_80077DC4(ang) << 4));
    } else {
        d = (int32_t)func_8003708C((uint32_t)sp, 0xB504u);
        f = PE_LoadU32(GA_D_8009D26C);
        a = PE_LoadU16(GA_D_800BD020);
        PE_StoreU16(actor + 0x3Au, (uint16_t)a);
#define WV(x, z) (PE_StoreU32(GA_WALK_VEL, (uint32_t)(x)), PE_StoreU32(GA_WALK_VEL + 4u, 0u), \
                  PE_StoreU32(GA_WALK_VEL + 8u, (uint32_t)(z)))
        if (f & 8u) {
            if (f & 0x40u)      { PE_StoreU16(actor + 0x3Au, (uint16_t)(a + 0x600)); WV(-d, d); }
            else if (f & 0x10u) { PE_StoreU16(actor + 0x3Au, (uint16_t)(a + 0xA00)); WV(d, d); }
            else                { PE_StoreU16(actor + 0x3Au, (uint16_t)(a + 0x800)); WV(0, sp); }
        } else if (f & 0x20u) {
            if (f & 0x40u)      { PE_StoreU16(actor + 0x3Au, (uint16_t)(a + 0x200)); WV(-d, -d); }
            else if (f & 0x10u) { PE_StoreU16(actor + 0x3Au, (uint16_t)(a + 0xE00)); WV(d, -d); }
            else                { PE_StoreU16(actor + 0x3Au, (uint16_t)a); WV(0, -sp); }
        } else if (f & 0x40u)   { PE_StoreU16(actor + 0x3Au, (uint16_t)(a + 0x400)); WV(-sp, 0); }
        else if (f & 0x10u)     { PE_StoreU16(actor + 0x3Au, (uint16_t)(a + 0xC00)); WV(sp, 0); }
#undef WV
    }
    r = (int16_t)PE_LoadU16(actor + 0x3Au);
    if (PE_LoadU32(GA_D_8009D254) != 0u && (PE_LoadU32(GA_D_8009D2E8) & 0x10u)) {
        r += 0x800;
        r += (int32_t)((PE_LoadU32(PE_LoadU32(PE_LoadU32(GA_D_8009D254)) + 0x4Cu) >> 7) & 0xC00u);
    }
    r &= 0xFFF;
    PE_StoreU16(actor + 0x3Au, (uint16_t)r);
    func_80078934(GA_D_800BD000, GA_WALK_VEL, actor + 0x68u);
    if (PE_LoadU32(GA_D_8009D2E8) & 0x10u) {
        PE_StoreU16(actor + 0x3Au, (uint16_t)((0x1000u - PE_LoadU16(actor + 0x3Au)) & 0xFFFu));
        PE_StoreU32(actor + 0x68u, (uint32_t)-(int32_t)PE_LoadU32(actor + 0x68u));
    }
}

/*
 * src/func_800710A4.c:45-57 + 109-120: the analog-pad field walk (pad ID 73h,
 * D_800BE9A0 & 0xF000 == 0x7000).  Heading = atan2(LY - 0x80, LX - 0x80)
 * - 0x400, wrapped to 0..0xFFF, plus the analog camera yaw D_800BD022;
 * velocity = (-speed * sin, 0, -speed * cos) of that heading; then the
 * shared D_8009D2E8 bit-4 heading adjust, func_80078934 into actor+0x68 and
 * the mirror.  Retail always takes this branch when the ID is 73h, so before
 * it was ported an analog reply left Aya's walk velocity unwritten.  Same
 * order as the func_8007136C analog block above.
 */
static void pe_walk_analog_apply(pe_addr_t actor, uint32_t speed)
{
    int32_t ang, d, sp, r;

    ang = func_80079FB4((int32_t)PE_LoadU8(0x800BE9A7u) - 0x80, (int32_t)PE_LoadU8(0x800BE9A6u) - 0x80) - 0x400;
    if (ang < 0) ang += 0x1000;
    PE_StoreU16(actor + 0x3Au, (uint16_t)(PE_LoadU16(0x800BD022u) + (uint32_t)ang));
    d = func_80077CF4(ang);
    sp = -(int32_t)speed;
    PE_StoreU32(GA_WALK_VEL + 0u, func_8003708C((uint32_t)sp, (uint32_t)d << 4));
    PE_StoreU32(GA_WALK_VEL + 4u, 0u);
    PE_StoreU32(GA_WALK_VEL + 8u, func_8003708C((uint32_t)sp, (uint32_t)func_80077DC4(ang) << 4));
    r = (int16_t)PE_LoadU16(actor + 0x3Au);
    if (PE_LoadU32(GA_D_8009D254) != 0u && (PE_LoadU32(GA_D_8009D2E8) & 0x10u)) {
        r += 0x800;
        r += (int32_t)((PE_LoadU32(PE_LoadU32(PE_LoadU32(GA_D_8009D254)) + 0x4Cu) >> 7) & 0xC00u);
    }
    r &= 0xFFF;
    PE_StoreU16(actor + 0x3Au, (uint16_t)r);
    func_80078934(GA_D_800BD000, GA_WALK_VEL, actor + 0x68u);
    if (PE_LoadU32(GA_D_8009D2E8) & 0x10u) {
        PE_StoreU16(actor + 0x3Au, (uint16_t)((0x1000u - PE_LoadU16(actor + 0x3Au)) & 0xFFFu));
        PE_StoreU32(actor + 0x68u, (uint32_t)-(int32_t)PE_LoadU32(actor + 0x68u));
    }
}

/*
 * PE-BTL68 — func_800710A4 field walk (no Circle): d-pad, or the stick with
 * the analog pad ID.  178 words 0x800710A4..0x8007136C. D1A0&2 jals 7136C.
 */
void func_800710A4(pe_addr_t actor, pe_addr_t codep)
{
    uint32_t scaled;

    if ((D_8009D1A0 & 2u) != 0u) {
        func_8007136C(actor, codep);
        return;
    }
    /* Opt-in "Always run" (pe_plat/enhance.h): the field d-pad walk takes the
     * Circle-held run path for Aya.  Off by default: no effect, retail path. */
    if (pe_enh_field_always_run(actor)) {
        func_8007136C(actor, codep);
        return;
    }
    if (PE_LoadU32(codep) != 0x16u) {
        func_8001A680_command_cut(actor, 0x16u);
        PE_StoreU32(codep, 0x16u);
    }
    scaled = func_8003708C(0x50000u, PE_LoadU32(actor + 0x20u));
    scaled = func_8003708C(scaled,
                           (uint32_t)PE_LoadU16(actor + 0x26u) << 4);
    scaled = (uint32_t)pe_enh_field_speed(actor, (int32_t)scaled);  /* opt-in; 100 % = retail */
    if ((PE_LoadU16(GA_D_800BE9A0) & 0xF000u) == 0x7000u) {
        pe_walk_analog_apply(actor, scaled);
        return;
    }
    pe_walk_digital_apply(actor, scaled);
}

/* 716A4..71754: field/battle idle share velocity clearing, while battle
 * selects the current equipment record's idle clip without restarting it. */
void func_800716A4(pe_addr_t actor, pe_addr_t codep)
{
    uint32_t idle = (D_8009D1A0 & 2u)
        ? PE_LoadU8(PE_LoadU32(actor) + 0x12u) : 0x15u;
    if (PE_LoadU32(codep) == idle) return;
    if (D_8009D1A0 & 2u) func_8001A784(actor, idle);
    else func_8001A680_command_cut(actor, idle);
    PE_StoreU32(actor + 0x68u, 0u);
    PE_StoreU32(actor + 0x6Cu, 0u);
    PE_StoreU32(actor + 0x70u, 0u);
    PE_StoreU32(codep, (D_8009D1A0 & 2u)
        ? PE_LoadU8(PE_LoadU32(actor) + 0x12u) : 0x15u);
}

static void pe_3999c_jalr(uint32_t fn, pe_addr_t actor, pe_addr_t codep)
{
    /* Field idle, 71034..710A4 in 617AC.s. Changing from walk to idle
     * clears velocity before the frame's integration; otherwise key-up
     * left Aya moving indefinitely because this default callback was absent. */
    if (fn == 0x80071034u || fn == 0x800716A4u)
        func_800716A4(actor, codep);
    else if (fn == GA_FN_7136C)
        func_8007136C(actor, codep);
    else if (fn == GA_FN_710A4)
        func_800710A4(actor, codep);
    else
        (void)PE_GuestCall("@8003999C:input", fn, 2u,
                           (uintptr_t)actor, (uintptr_t)codep, 0u, 0u);
}

/*
 * PE-BTL67/68 — func_8003999C pad-table dispatcher.
 *
 * 118 words 0x8003999C..0x80039B74, SHA-256 9a5267b0…3f6f.
 * Sole TEXT caller 35C84 @ 35D14. Indexes table[*a2], not actor+0
 * (399C0 is lw 0(s2); s2=a2). BTL67 host-guard on actor+0 is
 * REJECTED. Record+0xC jalr of 710A4/7136C is this cut;
 * STG6 also handles 71034/716A4 idle. Other handlers use native registry dispatch.
 */
void func_8003999C(pe_addr_t actor, pe_addr_t table, pe_addr_t codep)
{
    uint32_t idx;
    pe_addr_t list;
    pe_addr_t rec;
    uint32_t flags;
    uint32_t fn;
    uint32_t mask1;
    uint32_t mask2;
    uint32_t matched;
    int take;

    if (actor == 0u || table == 0u || codep == 0u)
        return;
    idx = PE_LoadU32(codep);
    if (idx >= 64u)
        return;
    list = PE_LoadU32(table + idx * 4u);
    rec = list;
    flags = (list != 0u) ? PE_LoadU32(list) : 0u;
    if (flags == 0u)
        goto end_default;

    matched = 0u;
    while (flags != 0u) {
        fn = PE_LoadU32(rec + 0xCu);
        if (fn == 0u)
            return;
        mask1 = PE_LoadU32(rec + 4u);
        mask2 = PE_LoadU32(rec + 8u);
        take = 0;
        if ((flags & 0x20u) != 0u) {
            take = 1;
        } else if ((flags & 1u) != 0u) {
            if ((PE_LoadU32(GA_D_8009D26C) & mask1) == mask1) {
                if ((flags & 4u) != 0u)
                    take = (PE_LoadU32(GA_D_8009D1F4) & mask2) != 0u;
                else if ((flags & 8u) != 0u)
                    take = (PE_LoadU32(GA_D_8009D1E4) & mask2) != 0u;
                else if ((flags & 2u) == 0u)
                    take = 1;
                else
                    take = (PE_LoadU32(GA_D_8009D26C) & mask2) != 0u;
            }
        } else if ((flags & 2u) != 0u) {
            if ((PE_LoadU32(GA_D_8009D26C) & mask2) != 0u) {
                if ((flags & 4u) != 0u)
                    take = (PE_LoadU32(GA_D_8009D1F4) & mask2) != 0u;
                else if ((flags & 8u) != 0u)
                    take = (PE_LoadU32(GA_D_8009D1E4) & mask2) != 0u;
                else
                    take = 1;
            }
        } else if ((flags & 4u) != 0u) {
            take = (PE_LoadU32(GA_D_8009D1F4) & mask2) != 0u;
        } else if ((flags & 8u) != 0u) {
            take = (PE_LoadU32(GA_D_8009D1E4) & mask2) != 0u;
        }
        if (take) {
            pe_3999c_jalr(fn, actor, codep);
            if (PE_Port_ShouldStop()) return;
            matched = 1u;
        }
        if ((flags & 0x10u) == 0u && matched != 0u)
            return;
        rec += 16u;
        flags = PE_LoadU32(rec);
    }

end_default:
    if (rec == 0u)
        return;
    fn = PE_LoadU32(rec + 0xCu);
    if (fn != 0u)
        pe_3999c_jalr(fn, actor, codep);
}

void func_80035C84(pe_addr_t actor)
{
    uint32_t code;
    pe_addr_t rec;

    pe_actor_snapshot_pose(actor);
    func_800361F4(actor);
    if (PE_Port_ShouldStop()) return;
    if ((PE_LoadU32(GA_D_8009D2E8) & 1u) == 0u) {
        code = PE_LoadU8(actor + 0x0Eu);
        rec = PE_LoadU32(GA_D_8009D254);
        if (rec != 0u) {
            rec = PE_LoadU32(rec);
            if (rec != 0u && (PE_LoadU32(rec + 0x4Cu) & 0xC0u) == 0x80u)
                code = 0x11u;
        }
        PE_StoreU32(0x80122190u, code);
        func_8003999C(actor, GA_D_800943C0, 0x80122190u);
        if (PE_Port_ShouldStop()) return;
    }
    pe_actor_integrate_motion(actor);
}

/* Retail 357B4..35988: publish the current actor world transform before
 * bone traversal. Translation uses signed integer halves of 16.16 pose. */
static void pe_actor_refresh_model_transform(pe_addr_t actor)
{
    pe_addr_t matrix = actor + 0x1E8u;
    unsigned int axis, row;
    int16_t scale = (int16_t)PE_LoadU16(actor + 0x26u);
    for (axis = 0; axis < 3u; axis++) {
        PE_StoreU32(matrix + 20u + axis * 4u,
            (uint32_t)(int32_t)(int16_t)PE_LoadU16(actor + 0x2Au + axis * 4u));
        PE_StoreU16(actor + 0x1E0u + axis * 2u, PE_LoadU16(actor + 0x38u + axis * 2u));
    }
    func_800794C4(actor + 0x1E0u, matrix);
    PE_GTE_LoadRT(matrix);
    for (axis = 0; axis < 3u; axis++) {
        int16_t v[3] = {0,0,0};
        v[axis] = scale;
        PE_GTE_SetIR(v[0],v[1],v[2]);
        PE_GTE_MVMVA(0x049E012u);
        for (row = 0; row < 3u; row++)
            PE_StoreU16(matrix + row * 6u + axis * 2u, (uint16_t)g_pe_gte.ir[row]);
    }
    PE_GTE_SetV0(0,0,0);
    PE_GTE_MVMVA(0x0480012u);
    for (axis = 0; axis < 3u; axis++)
        PE_StoreU32(matrix + 20u + axis * 4u, (uint32_t)g_pe_gte.ir[axis]);
}

void func_8001A9F8(void);

static void pe_35558_body(void)
{
    pe_addr_t actor;
    pe_addr_t fn;

    /* Menu pause skips actor callbacks, not battle input or rendering.
     * Retail 35574 branches to 355B4, before the call to 299CC. */
    actor = (D_8009D1A0 & 4u) ? 0u : PE_LoadU32(GA_D_8009D20C);
    while (actor != 0u) {
        fn = PE_LoadU32(actor + 0x190u);
        if (fn == GA_VT_35E04) {
            PE_M34StackActor(actor);
            func_80035E04(actor);
            PE_M34StackActor(0u);
        }
        else if (fn == GA_VT_35C84)
            func_80035C84(actor);
        else if (fn != 0u)
            (void)PE_GuestCall("@80035558:actor+190", fn, 1u,
                               (uintptr_t)actor, 0u, 0u, 0u);
        if (PE_Port_ShouldStop()) return;
        if (getenv("PE_CONTACT_TRACE") && PE_LoadU8(actor + 0xCu) == 2u && PE_LoadU8(actor + 0xDu) == 0u) {
            static uint32_t last_sig;
            uint32_t sig = PE_LoadU32(actor + 0x98u) ^ PE_LoadU32(actor + 0xA4u) ^ PE_LoadU32(actor + 0x228u)
                         ^ (PE_LoadU32(actor + 0x230u) << 3) ^ PE_LoadU32(actor + 0x1A0u);
            if (sig != last_sig) {
                last_sig = sig;
                fprintf(stderr, "CONTACT date actor=%08X tok=%08X f190=%08X f1A0=%08X f98=%08X fA4=%08X c228=%04X,%04X,%04X r230=%04X f26=%04X f1AC=%08X\n",
                        actor, PE_LoadU32(0x8009D280u), fn, PE_LoadU32(actor + 0x1A0u), PE_LoadU32(actor + 0x98u),
                        PE_LoadU32(actor + 0xA4u), PE_LoadU16(actor + 0x228u), PE_LoadU16(actor + 0x22Au),
                        PE_LoadU16(actor + 0x22Cu), PE_LoadU16(actor + 0x230u), PE_LoadU16(actor + 0x26u),
                        PE_LoadU32(actor + 0x1ACu));
            }
        }
        actor = PE_LoadU32(actor + 4u);
    }

    /* 355C8 skips the whole middle frame body while overlay bit 200 is
     * set. Actor callbacks have run; final contact/task cleanup still runs. */
    if (PE_LoadU32(GA_D_800B0CD8)&0x200u) goto final_cleanup;

    /* Retail 355E8 updates battle state after actor callbacks and before
     * floor resolution and model projection (damage can change poses). */
    if (D_8009D1A0 & 2u) {
        static int legacy = -1;   /* PE_BATTLE_TICK_LEGACY=1: old cuts, A/B diagnostics only */
        if (legacy < 0) legacy = getenv("PE_BATTLE_TICK_LEGACY") != NULL;
        if (!legacy) {
            func_800299CC();   /* complete battle tick (src/func_800299CC.c) */
        } else {
            func_800299CC_consume_cut();
            func_800299CC_after_consume_cut();
            if (PE_LoadU32(0x8009D28Cu) != 0u || PE_LoadU8(0x8009D244u) == 0u)
                func_800299CC_mode_switch_cut();
            else
                func_800299CC_damage_entry_cut();
        }
    } else {
        PE_FieldMenuFrame();
    }
    if (PE_Port_ShouldStop()) return;

    /* 356D0: small effect-pool movement precedes floor/contact resolution. */
    (void)func_80069660();
    if(PE_Port_ShouldStop())return;

    /* 356F0: constrain integrated positions before projecting models. */
    if (!(D_8009D1A0 & 4u)) func_8001A9F8();
    if (PE_Port_ShouldStop()) return;
    func_8003601C();
    (void)func_80066268();
    /* 35708..3579C: follow Aya, or the first resource-bearing actor
     * during the stage play, using this view's camera limits. */
    actor = PE_LoadU32(GA_D_8009D254);
    if (!actor) {
        actor = PE_LoadU32(GA_D_8009D20C);
        while (actor && !PE_LoadU32(actor + 0x1ACu))
            actor = PE_LoadU32(actor + 4u);
    }
    if (actor)
        (void)func_80065E48_position((int32_t)PE_LoadU32(actor + 0x28u),
            (int32_t)(PE_LoadU32(actor + 0x2Cu)
              - ((uint32_t)(int32_t)(int16_t)PE_LoadU16(0x800BCFFEu) << 16)),
            (int32_t)PE_LoadU32(actor + 0x30u));
    else
        (void)func_80065E48_position(0,0,0);
    /* Retail 35798..35998 freezes model rebuilds under the menu-pause bit. */
    if (!(D_8009D1A0 & 4u)) {
        func_800661A4();
        actor = PE_LoadU32(GA_D_8009D20C);
        while (actor != 0u) {
            if ((PE_LoadU32(actor + 0x98u) & 0x10040u) == 0u
                && PE_RangeIsRam(PE_LoadU32(actor + 0x238u), 32u))
                pe_actor_refresh_model_transform(actor);
            actor = PE_LoadU32(actor + 4u);
        }
    }
    func_800661A4();

    /* ROM 359B8: dest tick. Skip Aya when overlay&0x40000.
     * +0x98&0x40 and +0x98&0x20000000 skip 3AF14. */
    actor = PE_LoadU32(GA_D_8009D20C);
    while (actor != 0u) {
        uint32_t flags;

        if (!(actor == PE_LoadU32(GA_D_8009D254)
              && (PE_LoadU32(GA_D_800B0CD8) & 0x40000u) != 0u)) {
            /* Retail 359E0..35A7C: resource scale and hidden-actor poses. */
            if (PE_LoadU32(actor + 0x1ACu) != 0u)
                PE_StoreU16(PE_LoadU32(actor + 0x1B4u) + 0x16u,
                            PE_LoadU16(actor + 0x26u));
            flags = PE_LoadU32(actor + 0x98u);
            if (flags & 0x40u) {
                if (flags & 0x2000u) {
                    func_8003A6A8(actor + 0x1B4u, 0x800B89F8u);
                    PE_StoreU32(actor + 0x28u, (uint32_t)(int32_t)(int16_t)PE_LoadU16(actor + 0x254u) << 16);
                    PE_StoreU32(actor + 0x30u, (uint32_t)(int32_t)(int16_t)PE_LoadU16(actor + 0x258u) << 16);
                    PE_StoreU32(actor + 0x2Cu, (uint32_t)(int32_t)(int16_t)PE_LoadU16(actor + 0x256u) << 16);
                } else if (PE_LoadU32(actor + 0x1ACu) == 0u) {
                    PE_StoreU16(actor + 0x228u, PE_LoadU16(actor + 0x2Au));
                    PE_StoreU16(actor + 0x22Au, PE_LoadU16(actor + 0x2Eu));
                    PE_StoreU16(actor + 0x22Cu, PE_LoadU16(actor + 0x32u));
                }
            }
            /* End retail hidden-actor arm. */
            else {
                pe_addr_t instance = actor + 0x1B4u;
                /* Resource-bearing 35A80..35AE4 continuation. Empty
                 * instance fixtures remain outside model projection. */
                if (PE_LoadU32(instance) != 0u
                    && PE_RangeIsRam(PE_LoadU32(instance + 0x84u), 32u)) {
                    func_8006698C(instance);
                    if (PE_LoadU32(actor + 0x1B0u) != 0u)
                        func_80039B74(instance, PE_LoadU32(actor + 0x1B0u),
                                     (int16_t)PE_LoadU16(actor + 0x16u), 1);
                    if (PE_Port_ShouldStop()) return;
                    func_8003A088_mode0_walk_cut(instance);
                    if (PE_Port_ShouldStop()) return;
                    func_8003A6A8(instance, 0x800B89F8u);
                    if ((flags & 0x20000000u) == 0u)
                        func_8003AC90(instance, 0x800B89F8u);
                }
                if ((flags & 0x20000000u) == 0u) {
                    (void)func_8003AF14(instance, 0x800B89F8u);
                    if (PE_Port_ShouldStop()) return;
                    /* 35AE8..35B10: +0x98&0x8800 raises +0x250 bit 0, then
                     * the floor shadow func_80068014 (retail asm
                     * translation, decomp_hand/shadow_68014_port.c). */
                    if (PE_LoadU32(actor + 0x98u) & 0x8800u)
                        PE_StoreU16(actor + 0x250u, (uint16_t)(PE_LoadU16(actor + 0x250u) | 1u));
                    /* Needs the CD/GPU-DMA ordering in pe_libcd.c
                     * (func_8006E6D4): without it the shadow CLUT is
                     * clobbered (pink quads). */
                    if (PE_LoadU32(instance) != 0u)
                        (void)func_80068014(actor);
                }
            }
        }
        actor = PE_LoadU32(actor + 4u);
    }

    /* 35B24: keep the actor/weapon package loader moving between maps. */
    (void)func_8006C5BC();
    if (PE_Port_ShouldStop()) return;
    func_80069594();
    if (PE_Port_ShouldStop()) return;
    func_800661CC();

    /* ROM 0x80035B44: D1A0&4 skips 1A4AC, 36448, 12774, 360B4. */
    if (D_8009D1A0 & 4u)
        return;
    if (D_8009D1A0 & 0x100u) {
        if ((PE_LoadU32(GA_D_800B0CD8) & 0x40000u) == 0u) {
            actor = PE_LoadU32(GA_D_8009D254);
            if (actor != 0u)
                func_8001A4AC(actor);
        }
    } else {
        actor = PE_LoadU32(GA_D_8009D20C);
        while (actor != 0u) {
            if (!(actor == PE_LoadU32(GA_D_8009D254)
                  && (PE_LoadU32(GA_D_800B0CD8) & 0x40000u) != 0u)
                && (PE_LoadU32(actor + 0x98u) & 0x800040u) == 0u)
                func_8001A4AC(actor);
            actor = PE_LoadU32(actor + 4u);
        }
    }

final_cleanup:
    /* Original 35C10 rechecks the pause flag after animation callbacks. */
    if (D_8009D1A0 & 4u)
        return;
    /* Original 35C1C / 35C24: generate proximity tasks and reclaim finished ones. */
    func_80036448();
    if (PE_Port_ShouldStop()) return;
    func_80012774();

    /* 35C2C retires actors before 35C34 clears the per-frame flag. */
    func_800360B4();
    if (PE_Port_ShouldStop()) return;
    actor = PE_LoadU32(GA_D_8009D20C);
    while (actor != 0u) {
        uint32_t flags;

        flags = PE_LoadU32(actor + 0x98u);
        PE_StoreU32(actor + 0x98u, flags & ~0x10000000u);
        actor = PE_LoadU32(actor + 4u);
    }
}

void func_80035558_walk_cut(void)
{
    PE_M34StackField(1);
    pe_35558_body();
    PE_M34StackField(0);
}
