/*
 * Hand adapters — overlay-resident matched leaves (VRAM 0x800C0000..) that
 * were `absent` from pc_port (port_absent lane, 2026-09-23).
 *
 * Each function is the matched leaf src/func_XXXXXXXX.c with the host
 * adaptation spelled out: pointer parameters are guest addresses, guest
 * pointer globals are loaded with PE_LoadU32, `short *a1` element k lives at
 * a1 + 2k, and retail stack buffers handed to guest-address callees live in
 * PE_ABSENT_OVL_STACK (hand_absent_ovl_protos.h).  Code-pointer arguments
 * (particle callbacks) are the callee's retail VMA, exactly what the retail
 * lui/addiu pair loads.  Callees with no pc_port implementation are loud
 * boundaries (PE_Decomp_Boundary) carrying the guest argument registers.
 *
 * Common globals: D_800E27EC is the effect frame counter (int), D_800F33E0
 * the effect context whose +8 word is the particle pool, D_8009D254 the
 * player actor.
 */
#include "pe_guest_decomp.h"

#define OV_FRAME()   ((int)PE_LoadU32(0x800E27ECu))
#define OV_POOL()    PE_LoadU32(PE_LoadU32(0x800F33E0u) + 8u)
#define OV_ACTOR()   PE_LoadU32(0x8009D254u)
#define OV_RAND()    ((int)func_80071A54())
#define OV_S16(a)    ((int)(int16_t)PE_LoadU16(a))
#define OV_SET16(a, v) PE_StoreU16((a), (uint16_t)(v))

/* Shared "mode 2" sprite-state block of the particle emitters: the exact
 * store sequence
 *   D_800F3368 = 0x20; D_800F336A = 2; D_800F3376 = 0x20; D_800F3378 = 0x20;
 *   av = D_800E2850[ix]; D_800F336C = 1; D_800F3370 = av;
 *   func_800CEDA8(1 [, ix * 2]); D_800F336E = e; D_800F3372 = 0;
 *   D_800F3374 = bias;
 * with ix read from `ixsym` (D_800E11E6 / D_800E11F6).  Several leaves pass
 * a second argument `ix * 2` to func_800CEDA8; its matched C
 * (src/func_800CEDA8.c) takes one parameter, so the extra register is dead
 * and the host call passes the one argument. */
static void ov_sprite_state(pe_addr_t ixsym, unsigned e, unsigned bias)
{
    unsigned ix = PE_LoadU16(ixsym);
    unsigned av;

    OV_SET16(0x800F3368u, 0x20);
    OV_SET16(0x800F336Au, 2);
    OV_SET16(0x800F3376u, 0x20);
    OV_SET16(0x800F3378u, 0x20);
    av = PE_LoadU16(0x800E2850u + ix * 2u);
    OV_SET16(0x800F336Cu, 1);
    OV_SET16(0x800F3370u, av);
    func_800CEDA8(1);
    OV_SET16(0x800F336Eu, e);
    OV_SET16(0x800F3372u, 0);
    OV_SET16(0x800F3374u, bias);
}

/* *(u16 *)(*(u8 **)(D_800F32D0 + 8) + off): the target actor's position. */
static unsigned ov_target16(unsigned off)
{
    return PE_LoadU16(PE_LoadU32(PE_LoadU32(0x800F32D0u) + 8u) + off);
}

/* src/func_800C65E4.c: m = D_800C213C (16-byte copy); d = a1 - a0 per axis
 * (int, pad word untouched); func_80079178(&d, &m, &t1);
 * func_80079178(&d, &t1, &t2); func_80078120(&t1, a2 + 0x14);
 * func_80078120(&t2, a2 + 0x18); func_80078120(&d, a2 + 0x1C).  Neither
 * callee has a pc_port implementation (libgte matrix helpers), so all five
 * calls are loud boundaries with the retail guest argument addresses. */
void func_800C65E4(pe_addr_t a0, pe_addr_t a1, pe_addr_t a2)
{
    const pe_addr_t m = PE_ABSENT_OVL_C65E4_M, t1 = PE_ABSENT_OVL_C65E4_T1;
    const pe_addr_t t2 = PE_ABSENT_OVL_C65E4_T2, d = PE_ABSENT_OVL_C65E4_D;
    unsigned i;

    for (i = 0; i < 4u; i++)
        PE_StoreU32(m + i * 4u, PE_LoadU32(0x800C213Cu + i * 4u));
    for (i = 0; i < 3u; i++)
        PE_StoreU32(d + i * 4u, (uint32_t)(OV_S16(a1 + i * 2u) - OV_S16(a0 + i * 2u)));
    PE_Decomp_Boundary("func_80079178", 0x80079178u, 3u, d, m, t1, 0u);
    PE_Decomp_Boundary("func_80079178", 0x80079178u, 3u, d, t1, t2, 0u);
    PE_Decomp_Boundary("func_80078120", 0x80078120u, 2u, t1, a2 + 0x14u, 0u, 0u);
    PE_Decomp_Boundary("func_80078120", 0x80078120u, 2u, t2, a2 + 0x18u, 0u, 0u);
    PE_Decomp_Boundary("func_80078120", 0x80078120u, 2u, d, a2 + 0x1Cu, 0u, 0u);
}

/* src/func_800D5010.c: drifting sprite.  Mode 1 integrates velocity
 * a1[3..5] into a1[0..2], damps x/z by 31/32 (signed int division), flips
 * a1[4] when a1[1] > 0 then adds gravity 3, and ends once
 * D_800E27EC >= a1[6].  Mode 2 draws one func_800CEE20 sprite. */
int func_800D5010(int a0, pe_addr_t a1)
{
    const pe_addr_t pos = PE_ABSENT_OVL_POS, vec = PE_ABSENT_OVL_VEC;
    int s1v, s2v, q, w, idx, t, f;

    switch (a0) {
    case 1:
        OV_SET16(a1 + 0u, OV_S16(a1 + 0u) + OV_S16(a1 + 6u));
        OV_SET16(a1 + 2u, OV_S16(a1 + 2u) + OV_S16(a1 + 8u));
        OV_SET16(a1 + 4u, OV_S16(a1 + 4u) + OV_S16(a1 + 10u));
        OV_SET16(a1 + 6u, OV_S16(a1 + 6u) * 31 / 32);
        OV_SET16(a1 + 10u, OV_S16(a1 + 10u) * 31 / 32);
        if (OV_S16(a1 + 2u) > 0)
            OV_SET16(a1 + 8u, OV_S16(a1 + 8u) * -1);
        OV_SET16(a1 + 8u, OV_S16(a1 + 8u) + 3);
        if (OV_FRAME() < OV_S16(a1 + 12u))
            break;
        return 1;
    case 2:
        f = OV_FRAME();
        q = (f << 7) / OV_S16(a1 + 12u);
        OV_SET16(pos + 0u, OV_S16(a1 + 0u));
        OV_SET16(pos + 2u, OV_S16(a1 + 2u));
        OV_SET16(pos + 4u, OV_S16(a1 + 4u));
        OV_SET16(vec + 0u, 0);
        OV_SET16(vec + 2u, 0);
        OV_SET16(vec + 4u, OV_S16(a1 + 14u) + (f << 3));
        s2v = 0x80 - q;
        s1v = func_80077CF4((f << 10) / OV_S16(a1 + 12u)) + 0x1000;
        idx = (int)PE_LoadU16(0x800F336Cu);
        w = (int)PE_LoadU16(0x800E1204u + (uint32_t)idx * 2u);
        if (idx == 4 && PE_LoadU32(0x800F3428u) != 0u)
            w += 4;
        t = (int)func_80077AA4(0x40, (uint32_t)w);
        func_800CEE20(pos, vec, s1v, s1v,
                      OV_S16(0x800F336Au) * ((f << 3) / OV_S16(a1 + 12u)) + 0x80,
                      (uint32_t)(t & 0xFFFF), 1, s2v, 0u);
        break;
    default:
        return 0;
    }
    return 0;
}

/* src/func_800D6A1C.c: blk = D_800C22CC (4-byte copy).  Mode 0 seeds
 * *(int *)(a1 + 8 shorts) with rand, copies the target position into
 * a1[0..2] and a1[4..6] and raises a1[5] by 0x1B8.  Mode 1 ends at 0x20.
 * Mode 2 sets D_800F3374 = 0x64 around a func_800D004C burst, then below
 * frame 0x11 adds a func_800D0728 ring. */
int func_800D6A1C(int a0, pe_addr_t a1)
{
    const pe_addr_t vec = PE_ABSENT_OVL_VEC, blk = PE_ABSENT_OVL_BLK;
    int s1v, s2v, t, f;

    PE_StoreU32(blk, PE_LoadU32(0x800C22CCu));
    switch (a0) {
    case 0:
        t = OV_RAND();
        PE_StoreU32(a1 + 16u, (uint32_t)t);
        OV_SET16(a1 + 0u, ov_target16(0x268u));
        OV_SET16(a1 + 2u, ov_target16(0x26Au));
        OV_SET16(a1 + 4u, ov_target16(0x26Cu));
        OV_SET16(a1 + 8u, OV_S16(a1 + 0u));
        OV_SET16(a1 + 10u, OV_S16(a1 + 2u));
        OV_SET16(a1 + 12u, OV_S16(a1 + 4u));
        OV_SET16(a1 + 10u, OV_S16(a1 + 10u) - 0x1B8);
        return 0;
    case 1:
        if (OV_FRAME() < 0x20)
            break;
        return 1;
    case 2:
        f = OV_FRAME();
        OV_SET16(0x800F3374u, 0x64);
        s1v = func_80077DC4(f << 5) / 2 + 0x800;
        s2v = func_80077DC4(f << 5) / 32;
        func_800D004C(a1, 0x2BC, 0x2BC, 0xC, 0u, s1v, s1v, blk, 0u, s2v, 1);
        OV_SET16(0x800F3374u, 0);
        f = OV_FRAME();
        if (f < 0x11) {
            s2v = 0x80 - (f << 3);
            OV_SET16(vec + 0u, 0x400);
            OV_SET16(vec + 2u, 0);
            OV_SET16(vec + 4u, f << 7);
            OV_SET16(vec + 6u, 1);
            s1v = func_80077CF4(f << 6) * 2;
            func_800D0728(a1, 0x320, 0x3E8, 0x10, vec, s1v, s1v, 0u, blk, s2v, 1);
        }
        return 0;
    }
    return 0;
}

/* src/func_800D8B6C.c: player aura.  Mode 0 allocates 0xE x 0xC particles
 * (callback func_800D8978).  Mode 1 spawns one on odd frames below 0x20
 * (p[1] = rand, p[0] = p[2] = 0, p[4] = player y (func_800CE870 mode 1)
 * - 0x200), returns 1 from 0x32 and otherwise FALLS THROUGH into mode 2
 * (no break in the matched C).  Mode 2 refreshes D_800E21F8 with the player
 * position and sets the sprite state (ix D_800E11F6, D_800F336E = 1,
 * D_800F3374 = 8).  The leaf never reads a1. */
int func_800D8B6C(int a0, pe_addr_t a1)
{
    const pe_addr_t buf = PE_ABSENT_OVL_POS;
    pe_addr_t p, q;
    int t;

    (void)a1;
    switch (a0) {
    case 0:
        return (int)func_800CE560(OV_POOL(), 0xCu, 0xE, 0x800D8978u);
    case 1:
        if (OV_FRAME() < 0x20 && (OV_FRAME() & 1)) {
            p = func_800CE610(OV_POOL());
            if (p != 0u) {
                t = OV_RAND();
                q = OV_ACTOR();
                OV_SET16(p + 2u, t);
                OV_SET16(p + 0u, 0);
                OV_SET16(p + 4u, 0);
                func_800CE870(q, 1, buf);
                OV_SET16(p + 8u, OV_S16(buf + 2u) - 0x200);
            }
        }
        if (OV_FRAME() >= 0x32)
            return 1;
        /* fall through */
    case 2:
        func_800CE870(OV_ACTOR(), 1, 0x800E21F8u);
        ov_sprite_state(0x800E11F6u, 1u, 8u);
        break;
    default:
        return 0;
    }
    return 0;
}

/* src/func_800D9554.c: jittered smoke puff.  Mode 1: a1[0] = (u16)(a1[0]-1)
 * + a1[3] + (rand & 3), the same for y with a1[4], z += a1[5], x/z
 * velocity damped by 31/32, a1[4] decremented below frame 0x13 else
 * incremented; ends at 0x28.  Mode 2 draws one func_800D1DEC sprite whose
 * colour is the D_800E1AC8 curve below 0x13, else 0xC8C8 on one frame in
 * four (rand & 3 == 0) or black. */
int func_800D9554(int a0, pe_addr_t a1)
{
    const pe_addr_t buf = PE_ABSENT_OVL_POS, b = PE_ABSENT_OVL_BLK;
    int r;
    unsigned short w;

    switch (a0) {
    case 1:
        r = OV_RAND();
        w = (unsigned short)(OV_S16(a1 + 0u) - 1);
        OV_SET16(a1 + 0u, w + (OV_S16(a1 + 6u) + (r & 3)));
        r = OV_RAND();
        w = (unsigned short)(OV_S16(a1 + 2u) - 1);
        OV_SET16(a1 + 2u, w + (OV_S16(a1 + 8u) + (r & 3)));
        OV_SET16(a1 + 4u, OV_S16(a1 + 4u) + OV_S16(a1 + 10u));
        OV_SET16(a1 + 6u, OV_S16(a1 + 6u) * 31 / 32);
        OV_SET16(a1 + 10u, OV_S16(a1 + 10u) * 31 / 32);
        if (OV_FRAME() < 0x13)
            OV_SET16(a1 + 8u, OV_S16(a1 + 8u) - 1);
        else
            OV_SET16(a1 + 8u, OV_S16(a1 + 8u) + 1);
        if (OV_FRAME() < 0x28)
            break;
        return 1;
    case 2:
        OV_SET16(buf + 0u, OV_S16(a1 + 0u));
        OV_SET16(buf + 2u, OV_S16(a1 + 2u));
        OV_SET16(buf + 4u, OV_S16(a1 + 4u));
        if (OV_FRAME() < 0x13)
            func_800CF3AC(0x800E1AC8u, b, OV_FRAME());
        else if (!(OV_RAND() & 3))
            PE_StoreU32(b, 0xC8C8u);
        else
            PE_StoreU32(b, 0u);
        func_800D1DEC(buf, b, 0x80, 1);
        break;
    default:
        return 0;
    }
    return 0;
}

/* src/func_800D9FD4.c: player dust.  Mode 0 takes the player position into
 * a1 (func_800CE870 mode 1) and allocates 0xC x 8 particles (callback
 * func_800D9E5C).  Mode 1 spawns on odd frames below 0x20 at
 * a1 +- (rand % 400 - 200) in x/z with p[3] = -(rand & 7) - 16; ends at
 * 0x46.  Mode 2: sprite state (ix D_800E11E6, D_800F336E = 0, bias 8). */
int func_800D9FD4(int a0, pe_addr_t a1)
{
    pe_addr_t p;

    switch (a0) {
    case 0:
        func_800CE870(OV_ACTOR(), 1, a1);
        return (int)func_800CE560(OV_POOL(), 8u, 0xC, 0x800D9E5Cu);
    case 1:
        if (OV_FRAME() < 0x20 && (OV_FRAME() & 1)) {
            p = func_800CE610(OV_POOL());
            if (p != 0u) {
                OV_SET16(p + 0u, (int)PE_LoadU16(a1 + 0u) + OV_RAND() % 400 - 200);
                OV_SET16(p + 2u, PE_LoadU16(a1 + 2u));
                OV_SET16(p + 4u, (int)PE_LoadU16(a1 + 4u) + OV_RAND() % 400 - 200);
                OV_SET16(p + 6u, -(OV_RAND() & 7) - 16);
            }
        }
        if (OV_FRAME() < 0x46)
            break;
        return 1;
    case 2:
        ov_sprite_state(0x800E11E6u, 0u, 8u);
        break;
    default:
        return 0;
    }
    return 0;
}

/* src/func_800DA5D4.c: spiral.  Mode 0 seeds *a1 with rand, resets the
 * D_800F32D8 mesh UVs (func_800C6D5C(q, 0, 0)) and allocates 0x18 x 0x10
 * particles (callback func_800DA1FC).  Mode 1 spawns every 6th frame below
 * 0x28 (p[5] = *a1, p[3] = p[6] = 0, *a1 -= 0x555 + (rand & 0xFF)); ends at
 * 0x46.  Mode 2 publishes the target position to D_800E2214..18, then
 * overwrites D_800E2216 with D_800942EC. */
int func_800DA5D4(int a0, pe_addr_t a1)
{
    pe_addr_t p, q;
    int t;

    switch (a0) {
    case 0:
        t = OV_RAND();
        q = PE_LoadU32(0x800F32D8u);
        PE_StoreU32(a1, (uint32_t)t);
        func_800C6D5C(q, 0u, 0u);
        return (int)func_800CE560(OV_POOL(), 0x10u, 0x18, 0x800DA1FCu);
    case 1:
        if (OV_FRAME() < 0x28 && OV_FRAME() % 6 == 0) {
            p = func_800CE610(OV_POOL());
            if (p != 0u) {
                OV_SET16(p + 10u, PE_LoadU32(a1));
                OV_SET16(p + 6u, 0);
                OV_SET16(p + 12u, 0);
                t = OV_RAND();
                PE_StoreU32(a1, PE_LoadU32(a1) - (uint32_t)(0x555 + (t & 0xFF)));
            }
        }
        if (OV_FRAME() < 0x46)
            break;
        return 1;
    case 2:
        OV_SET16(0x800E2214u, ov_target16(0x268u));
        OV_SET16(0x800E2216u, ov_target16(0x26Au));
        OV_SET16(0x800E2218u, ov_target16(0x26Cu));
        OV_SET16(0x800E2216u, PE_LoadU16(0x800942ECu));
        break;
    default:
        return 0;
    }
    return 0;
}

/* src/func_800DA934.c: falling sparks at the target.  Mode 0 copies the
 * target position into a1 (y replaced by D_800942EC) and allocates 0xC x 8
 * particles (callback func_800DA780).  Mode 1 spawns on odd frames below
 * 0x28 at a1 +- (rand % 400 - 200), y - 0x320, p[3] = (rand & 7) + 0x2A;
 * ends at 0x46.  Mode 2: sprite state (ix D_800E11E6, D_800F336E = 0,
 * bias 0x20; func_800CEDA8(1) with one argument in the C). */
int func_800DA934(int a0, pe_addr_t a1)
{
    pe_addr_t p, r;

    switch (a0) {
    case 0:
        OV_SET16(a1 + 0u, ov_target16(0x268u));
        OV_SET16(a1 + 2u, ov_target16(0x26Au));
        r = PE_LoadU32(0x800F33E0u);
        OV_SET16(a1 + 4u, ov_target16(0x26Cu));
        OV_SET16(a1 + 2u, PE_LoadU16(0x800942ECu));
        return (int)func_800CE560(PE_LoadU32(r + 8u), 8u, 0xC, 0x800DA780u);
    case 1:
        if (OV_FRAME() < 0x28 && (OV_FRAME() & 1)) {
            p = func_800CE610(OV_POOL());
            if (p != 0u) {
                OV_SET16(p + 0u, (int)PE_LoadU16(a1 + 0u) + OV_RAND() % 400 - 200);
                OV_SET16(p + 2u, (int)PE_LoadU16(a1 + 2u) - 0x320);
                OV_SET16(p + 4u, (int)PE_LoadU16(a1 + 4u) + OV_RAND() % 400 - 200);
                OV_SET16(p + 6u, (OV_RAND() & 7) + 0x2A);
            }
        }
        if (OV_FRAME() < 0x46)
            break;
        return 1;
    case 2:
        ov_sprite_state(0x800E11E6u, 0u, 0x20u);
        break;
    default:
        return 0;
    }
    return 0;
}

/* src/func_800DBA9C.c: orbiting flash.  Mode 0 stores the running angle
 * D_800E1D60 into *(int *)(a1 + 4 shorts) and advances it by 0x555, takes
 * the player position (func_800CE870 mode 0), lifts y by 100 and offsets
 * x/z by sin/cos(angle) * 300 / 4096.  Mode 1 ends at 8.  Mode 2 draws two
 * func_800D004C bursts and a func_800D0728 ring from the D_800E1D64 /
 * D_800E1D84 colour curves. */
int func_800DBA9C(int a0, pe_addr_t a1)
{
    const pe_addr_t buf = PE_ABSENT_OVL_POS, blk = PE_ABSENT_OVL_BLK;
    int s0v, t;
    pe_addr_t p;

    switch (a0) {
    case 0:
        p = OV_ACTOR();
        t = (int)PE_LoadU32(0x800E1D60u);
        PE_StoreU32(a1 + 8u, (uint32_t)t);
        PE_StoreU32(0x800E1D60u, (uint32_t)(t + 0x555));
        func_800CE870(p, 0, a1);
        OV_SET16(a1 + 2u, OV_S16(a1 + 2u) - 100);
        OV_SET16(a1 + 0u, OV_S16(a1 + 0u) +
                 func_80077DC4((int)PE_LoadU32(a1 + 8u)) * 300 / 4096);
        OV_SET16(a1 + 4u, OV_S16(a1 + 4u) +
                 func_80077CF4((int)PE_LoadU32(a1 + 8u)) * 300 / 4096);
        return 0;
    case 1:
        if (OV_FRAME() < 8)
            break;
        return 1;
    case 2:
        OV_SET16(0x800F3374u, 0x3C);
        OV_SET16(buf + 0u, OV_S16(a1 + 0u));
        OV_SET16(buf + 2u, OV_S16(a1 + 2u));
        OV_SET16(buf + 4u, OV_S16(a1 + 4u));
        s0v = (OV_FRAME() << 9) + 0x800;
        func_800CF3AC(0x800E1D64u, blk, OV_FRAME());
        func_800D004C(buf, 0x1F4, 0x1F4, 0x10, 0u, s0v, s0v, blk, 0u, 0x80, 1);
        func_800D0728(buf, 0x19A, 0x1F4, 0x14, 0u, 0x1000, 0x1000, 0u, blk, 0x80, 3);
        func_800CF3AC(0x800E1D84u, blk, OV_FRAME());
        func_800D004C(buf, 0x1F4, 0x64, 0x10, 0u, s0v, s0v, blk, 0u, 0x80, 1);
        return 0;
    }
    return 0;
}

/* Common mode-1 body of func_800DBE6C / func_800DC750: spawn at
 * a1 + ((rand & 0x1FF) - 0x100) per axis, x then y then z. */
static void ov_spawn_box(pe_addr_t a1)
{
    pe_addr_t p = func_800CE610(OV_POOL());

    if (p != 0u) {
        OV_SET16(p + 0u, (int)PE_LoadU16(a1 + 0u) + (OV_RAND() & 0x1FF) - 0x100);
        OV_SET16(p + 2u, (int)PE_LoadU16(a1 + 2u) + (OV_RAND() & 0x1FF) - 0x100);
        OV_SET16(p + 4u, (int)PE_LoadU16(a1 + 4u) + (OV_RAND() & 0x1FF) - 0x100);
    }
}

/* src/func_800DBE6C.c: target sparkle.  Mode 0 copies the target position
 * into a1 and allocates 0x14 x 8 particles (callback func_800DBCD8).
 * Mode 1 spawns every frame below 0x15; ends at 0x35.  Mode 2: sprite
 * state (ix D_800E11E6, D_800F336E = 0, bias 0x10). */
int func_800DBE6C(int a0, pe_addr_t a1)
{
    switch (a0) {
    case 0:
        OV_SET16(a1 + 0u, ov_target16(0x268u));
        OV_SET16(a1 + 2u, ov_target16(0x26Au));
        OV_SET16(a1 + 4u, ov_target16(0x26Cu));
        return (int)func_800CE560(OV_POOL(), 8u, 0x14, 0x800DBCD8u);
    case 1:
        if (OV_FRAME() < 0x15)
            ov_spawn_box(a1);
        if (OV_FRAME() < 0x35)
            break;
        return 1;
    case 2:
        ov_sprite_state(0x800E11E6u, 0u, 0x10u);
        break;
    default:
        return 0;
    }
    return 0;
}

/* src/func_800DC750.c: player sparkle.  Mode 0 takes the player position
 * (func_800CE870 mode 0) and allocates 0x14 x 8 particles (callback
 * func_800DC5BC).  Mode 1 spawns every frame below 0x28; ends at 0x35.
 * Mode 2: sprite state (ix D_800E11F6, D_800F336E = 1, bias 0x10). */
int func_800DC750(int a0, pe_addr_t a1)
{
    switch (a0) {
    case 0:
        func_800CE870(OV_ACTOR(), 0, a1);
        return (int)func_800CE560(OV_POOL(), 8u, 0x14, 0x800DC5BCu);
    case 1:
        if (OV_FRAME() < 0x28)
            ov_spawn_box(a1);
        if (OV_FRAME() < 0x35)
            break;
        return 1;
    case 2:
        ov_sprite_state(0x800E11F6u, 1u, 0x10u);
        break;
    default:
        return 0;
    }
    return 0;
}

/* src/func_800DCA80.c: player burst.  Mode 0 takes the player position
 * (func_800CE870 mode 1) and allocates 0x14 x 8 particles (callback
 * func_800DC910).  Mode 1 spawns on odd frames below 0x21 at
 * x + rand % 800 - 400, y + rand % 400 - 700, z + rand % 800 - 400; ends at
 * 0x3C.  Mode 2: sprite state (ix D_800E11E6, D_800F336E = 0, bias 0x10). */
int func_800DCA80(int a0, pe_addr_t a1)
{
    pe_addr_t p;

    switch (a0) {
    case 0:
        func_800CE870(OV_ACTOR(), 1, a1);
        return (int)func_800CE560(OV_POOL(), 8u, 0x14, 0x800DC910u);
    case 1:
        if (OV_FRAME() < 0x21 && (OV_FRAME() & 1)) {
            p = func_800CE610(OV_POOL());
            if (p != 0u) {
                OV_SET16(p + 0u, (int)PE_LoadU16(a1 + 0u) + OV_RAND() % 800 - 400);
                OV_SET16(p + 2u, (int)PE_LoadU16(a1 + 2u) + OV_RAND() % 400 - 700);
                OV_SET16(p + 4u, (int)PE_LoadU16(a1 + 4u) + OV_RAND() % 800 - 400);
            }
        }
        if (OV_FRAME() < 0x3C)
            break;
        return 1;
    case 2:
        ov_sprite_state(0x800E11E6u, 0u, 0x10u);
        break;
    default:
        return 0;
    }
    return 0;
}

/* src/func_800DCE94.c: player swirl.  Mode 0 seeds *(int *)(a1 + 8) with
 * rand, takes the player position (func_800CE870 mode 0) and allocates
 * 0x10 x 0x10 particles (callback func_800DCCCC).  Mode 1 spawns below 0x11
 * (p[6] = (rand & 0x1FF) - 0x100, p[1] = seed, p[2] = rand,
 * p[7] = (rand & 0x1F) - 0x10, seed += 0x8AA + (rand & 0x1F)); ends at 0x49.
 * Mode 2 draws the D_800E1F18 ring/burst below 0x41, then publishes a1 to
 * D_800E222C..30 and sets D_800F336C = 1, D_800F3370 = D_800E2850[ix
 * D_800E11E6], func_800CEDA8(1), D_800F336E = 0, D_800F3374 = 8 (no
 * D_800F3368/336A/3372/3376/3378 stores in this leaf). */
int func_800DCE94(int a0, pe_addr_t a1)
{
    const pe_addr_t buf = PE_ABSENT_OVL_POS, blk = PE_ABSENT_OVL_BLK;
    pe_addr_t p, q;
    int t, s1v, v, f;
    unsigned ix, av;

    switch (a0) {
    case 0:
        t = OV_RAND();
        q = OV_ACTOR();
        PE_StoreU32(a1 + 8u, (uint32_t)t);
        func_800CE870(q, 0, a1);
        return (int)func_800CE560(OV_POOL(), 0x10u, 0x10, 0x800DCCCCu);
    case 1:
        if (OV_FRAME() < 0x11) {
            p = func_800CE610(OV_POOL());
            if (p != 0u) {
                OV_SET16(p + 12u, (OV_RAND() & 0x1FF) - 0x100);
                OV_SET16(p + 2u, PE_LoadU32(a1 + 8u));
                OV_SET16(p + 4u, OV_RAND());
                OV_SET16(p + 14u, (OV_RAND() & 0x1F) - 0x10);
                PE_StoreU32(a1 + 8u, PE_LoadU32(a1 + 8u) +
                            (uint32_t)(0x8AA + (OV_RAND() & 0x1F)));
            }
        }
        if (OV_FRAME() < 0x49)
            break;
        return 1;
    case 2:
        if (OV_FRAME() < 0x41) {
            f = OV_FRAME();
            s1v = (f << 6) + 0x800;
            OV_SET16(0x800F3374u, 0x3C);
            OV_SET16(buf + 0u, PE_LoadU16(a1 + 0u));
            OV_SET16(buf + 2u, PE_LoadU16(a1 + 2u));
            OV_SET16(buf + 4u, PE_LoadU16(a1 + 4u));
            func_800CF3AC(0x800E1F18u, blk, f);
            v = 1;
            if (OV_FRAME() & 1)
                v = 3;
            func_800D0728(buf, 0xC8, 0x12C, 0x10, 0u, s1v, s1v, blk, 0u, 0x80, v);
            s1v = func_80077CF4(OV_FRAME() << 4) / 2 + 0x800;
            func_800D004C(buf, 0x2BC, 0x2BC, 0x10, 0u, s1v, s1v, blk, 0u, 0x80, 1);
            func_800D0728(buf, 0x258, 0x320, 0x18, 0u, s1v, s1v, 0u, blk, 0x80, 3);
        }
        OV_SET16(0x800E222Cu, PE_LoadU16(a1 + 0u));
        OV_SET16(0x800E222Eu, PE_LoadU16(a1 + 2u));
        OV_SET16(0x800E2230u, PE_LoadU16(a1 + 4u));
        ix = PE_LoadU16(0x800E11E6u);
        av = PE_LoadU16(0x800E2850u + ix * 2u);
        OV_SET16(0x800F336Cu, 1);
        OV_SET16(0x800F3370u, av);
        func_800CEDA8(1);
        OV_SET16(0x800F336Eu, 0);
        OV_SET16(0x800F3374u, 8);
        break;
    default:
        return 0;
    }
    return 0;
}

/* src/func_800DD19C.c: rising flash.  Mode 0 seeds *(int *)(a1 + 4 shorts)
 * with rand, takes the player position (func_800CE870 mode 1) and lifts y
 * by 0x1FE.  Mode 1 ends at 0x40.  Mode 2 draws the D_800E1FA4 burst and
 * ring and the D_800E1FCC burst with vec = {0, 0, frame * 16, 0}. */
int func_800DD19C(int a0, pe_addr_t a1)
{
    const pe_addr_t buf = PE_ABSENT_OVL_POS, vec = PE_ABSENT_OVL_VEC;
    const pe_addr_t blk = PE_ABSENT_OVL_BLK;
    int s0v, r;
    pe_addr_t p;

    switch (a0) {
    case 0:
        r = OV_RAND();
        p = OV_ACTOR();
        PE_StoreU32(a1 + 8u, (uint32_t)r);
        func_800CE870(p, 1, a1);
        OV_SET16(a1 + 2u, OV_S16(a1 + 2u) - 0x1FE);
        return 0;
    case 1:
        if (OV_FRAME() < 0x40)
            break;
        return 1;
    case 2:
        OV_SET16(0x800F3374u, 0x3C);
        OV_SET16(buf + 0u, OV_S16(a1 + 0u));
        OV_SET16(buf + 2u, OV_S16(a1 + 2u));
        OV_SET16(buf + 4u, OV_S16(a1 + 4u));
        s0v = OV_FRAME() * 32 + 0x1000;
        func_800CF3AC(0x800E1FA4u, blk, OV_FRAME());
        func_800D004C(buf, 0x1F4, 0x1F4, 0x10, 0u, s0v, s0v, blk, 0u, 0x80, 1);
        func_800D0728(buf, 0x19A, 0x1F4, 0x14, 0u, 0x1000, 0x1000, 0u, blk, 0x80, 3);
        func_800CF3AC(0x800E1FCCu, blk, OV_FRAME());
        OV_SET16(vec + 0u, 0);
        OV_SET16(vec + 2u, 0);
        OV_SET16(vec + 6u, 0);
        OV_SET16(vec + 4u, OV_FRAME() * 16);
        func_800D004C(buf, 0x190, 0x64, 0x18, vec, s0v, s0v, blk, 0u, 0x80, 1);
        return 0;
    }
    return 0;
}

/* src/func_800DD76C.c: player fountain.  Mode 0 takes the player position
 * (func_800CE870 mode 1) into a1[0..2], mirrors it to a1[4..6] with
 * a1[5] -= 0x226 and allocates 0x18 x 0x10 particles (callback
 * func_800DD380).  Mode 1 spawns on odd frames below 0x31 at
 * a1 +- (rand % 700 - 350) in x/z, p[4] = p[5] = 0, p[7] = rand; ends at
 * 0x8C.  Mode 2 publishes a1[4..6] to D_800E2234..38 and sets the sprite
 * state (ix D_800E11E6, D_800F336E = 0, bias 0x10). */
int func_800DD76C(int a0, pe_addr_t a1)
{
    pe_addr_t p;

    switch (a0) {
    case 0:
        func_800CE870(OV_ACTOR(), 1, a1);
        OV_SET16(a1 + 8u, OV_S16(a1 + 0u));
        OV_SET16(a1 + 10u, OV_S16(a1 + 2u));
        OV_SET16(a1 + 12u, OV_S16(a1 + 4u));
        OV_SET16(a1 + 10u, OV_S16(a1 + 10u) - 0x226);
        return (int)func_800CE560(OV_POOL(), 0x10u, 0x18, 0x800DD380u);
    case 1:
        if (OV_FRAME() < 0x31 && (OV_FRAME() & 1)) {
            p = func_800CE610(OV_POOL());
            if (p != 0u) {
                OV_SET16(p + 0u, OV_S16(a1 + 0u) + OV_RAND() % 700 - 350);
                OV_SET16(p + 2u, OV_S16(a1 + 2u));
                OV_SET16(p + 4u, OV_S16(a1 + 4u) + OV_RAND() % 700 - 350);
                OV_SET16(p + 8u, 0);
                OV_SET16(p + 10u, 0);
                OV_SET16(p + 14u, OV_RAND());
            }
        }
        if (OV_FRAME() < 0x8C)
            break;
        return 1;
    case 2:
        OV_SET16(0x800E2234u, PE_LoadU16(a1 + 8u));
        OV_SET16(0x800E2236u, PE_LoadU16(a1 + 10u));
        OV_SET16(0x800E2238u, PE_LoadU16(a1 + 12u));
        ov_sprite_state(0x800E11E6u, 0u, 0x10u);
        break;
    default:
        return 0;
    }
    return 0;
}

/* Scratchpad (1F800000 fast RAM, PE_RangeIsScratchpad) light parameters the
 * two glow updaters below publish before their shared draw helper. */
static void ov_glow_scratch(pe_addr_t a0)
{
    OV_SET16(0x1F800024u, PE_LoadU16(a0 + 4u));
    OV_SET16(0x1F800026u, PE_LoadU16(a0 + 6u));
    OV_SET16(0x1F800028u, PE_LoadU16(a0 + 8u));
    PE_StoreU32(0x1F80002Cu, (uint32_t)OV_S16(a0 + 0xEu));
    PE_StoreU32(0x1F800034u, PE_LoadU32(0x800BCFA4u));
}

/* src/func_800E026C.c: fading glow.  Inactive (a0[0] == 0) returns at once.
 * Otherwise publishes the scratchpad draw parameters (0x77D3 / 0x34 / grey
 * 0x80 RGB / alpha a0[3] / 0xD0 / position / depth / D_800BCFA4); state 1
 * steps: when the delay a0[2] is 0, alpha += 0x10 (byte) and the delay
 * reloads from a0[1] — past 0x30 the glow is freed (a0[0] = a0[3] = 0,
 * D_800E21A4--) and returns; else the delay counts down.  Still in state 1
 * it calls func_800E051C.  That callee has no matched C and no pc_port
 * implementation: a loud boundary.  It is declared without arguments;
 * pe_dis.sh 0x800E026C 0x134 shows $a0 never written, so the live $a0 at
 * the `jal 0x800e051c` (0x800E0388) is the record — recorded as arity 1. */
void func_800E051C(void);
void func_800E026C(pe_addr_t a0)
{
    unsigned char c;

    if (PE_LoadU8(a0) == 0u)
        return;
    OV_SET16(0x1F80001Eu, 0x77D3);
    OV_SET16(0x1F800022u, 0x34);
    PE_StoreU8(0x1F800018u, 0x80u);
    PE_StoreU8(0x1F800019u, 0x80u);
    PE_StoreU8(0x1F80001Au, 0x80u);
    PE_StoreU8(0x1F80001Cu, PE_LoadU8(a0 + 3u));
    PE_StoreU8(0x1F80001Du, 0xD0u);
    ov_glow_scratch(a0);
    if (PE_LoadU8(a0) != 1u)
        return;
    if (PE_LoadU8(a0 + 2u) == 0u) {
        c = (unsigned char)(PE_LoadU8(a0 + 3u) + 0x10u);
        PE_StoreU8(a0 + 3u, c);
        PE_StoreU8(a0 + 2u, PE_LoadU8(a0 + 1u));
        if (c > 0x30u) {
            PE_StoreU8(a0, 0u);
            PE_StoreU8(a0 + 3u, 0u);
            OV_SET16(0x800E21A4u, OV_S16(0x800E21A4u) - 1);
            return;
        }
    } else {
        PE_StoreU8(a0 + 2u, (uint8_t)(PE_LoadU8(a0 + 2u) - 1u));
    }
    if (PE_LoadU8(a0) == 1u)
        func_800E051C();   /* src/func_800E051C.c (native: func_8003F3C4_port.c) */
}

/* src/func_800E03A0.c: coloured glow.  Inactive returns at once.  Publishes
 * 0x7713 / 0x34 / 0xC0 / 0xCA / position / depth / D_800BCFA4 to the
 * scratchpad and the colour a0[0x10..0x12] minus the level (short at +0xC),
 * each clamped at 0.  State != 1 fades the level down by a0[1] (freed below
 * 0: a0[0] = 0, level 0, D_800E21A4--, return); state 1 fades it up,
 * clamping at 0xFF and entering state 2.  Then func_800E051C (no matched C,
 * no pc_port implementation: loud boundary).  pe_dis.sh 0x800E03A0 0x17C:
 * at the `jal 0x800e051c` (0x800E0504) $a0 = clamped blue, $a1 = the record
 * (0x800E03A4 `move a1,a0`), $a2 = clamped red — recorded as arity 3. */
void func_800E03A0(pe_addr_t a0)
{
    int r, g, b, lv;

    if (PE_LoadU8(a0) == 0u)
        return;
    OV_SET16(0x1F80001Eu, 0x7713);
    OV_SET16(0x1F800022u, 0x34);
    PE_StoreU8(0x1F80001Cu, 0xC0u);
    PE_StoreU8(0x1F80001Du, 0xCAu);
    ov_glow_scratch(a0);
    lv = OV_S16(a0 + 0xCu);
    r = (int)PE_LoadU8(a0 + 0x10u) - lv;
    g = (int)PE_LoadU8(a0 + 0x11u) - lv;
    b = (int)PE_LoadU8(a0 + 0x12u) - lv;
    if (r < 0)
        r = 0;
    if (g < 0)
        g = 0;
    if (b < 0)
        b = 0;
    PE_StoreU8(0x1F800018u, (uint8_t)r);
    PE_StoreU8(0x1F800019u, (uint8_t)g);
    PE_StoreU8(0x1F80001Au, (uint8_t)b);
    if (PE_LoadU8(a0) != 1u) {
        OV_SET16(a0 + 0xCu, OV_S16(a0 + 0xCu) - (int)PE_LoadU8(a0 + 1u));
        if (OV_S16(a0 + 0xCu) < 0) {
            PE_StoreU8(a0, 0u);
            lv = OV_S16(0x800E21A4u);
            OV_SET16(a0 + 0xCu, 0);
            OV_SET16(0x800E21A4u, lv - 1);
            return;
        }
    } else {
        OV_SET16(a0 + 0xCu, OV_S16(a0 + 0xCu) + (int)PE_LoadU8(a0 + 1u));
        if (OV_S16(a0 + 0xCu) >= 0x100) {
            OV_SET16(a0 + 0xCu, 0xFF);
            PE_StoreU8(a0, 2u);
        }
    }
    func_800E051C();   /* src/func_800E051C.c reads the scratchpad block, not $a0-$a2 */
}
