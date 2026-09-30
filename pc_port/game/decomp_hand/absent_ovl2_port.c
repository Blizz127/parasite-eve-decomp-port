/*
 * Hand adapters — overlay-resident matched leaves (VRAM 0x800D0000..) that
 * were `absent` from pc_port (port_absent lane round 2, 2026-09-24).
 *
 * Each function is the matched leaf src/func_XXXXXXXX.c with the host
 * adaptation spelled out, following absent_ovl_port.c: the `short *a1`
 * record is a guest address (element k at a1 + 2k, signed 16-bit), guest
 * pointer globals are loaded with PE_LoadU32, `*(int *)(a1 + k)` is the
 * 32-bit word at a1 + 2k, struct copies of D_800C22xx go into
 * PE_HAND_OVL2_STACK (hand_absent_ovl2_protos.h) and particle callbacks are
 * the callee's retail VMA.  Every callee argument list below is the matched
 * C's explicit one; `func_800CEDA8(1, ix * 2)` passes one argument because
 * func_800CEDA8's matched C takes one (the extra register is dead — same
 * ruling as absent_ovl_port.c).  func_800CF5B0 and func_800CEAE8 have no
 * pc_port implementation and are loud boundaries with the guest arguments.
 *
 * int products are formed with 32-bit wraparound (retail `mult`/`mflo`).
 */
#include "pe_guest_decomp.h"

#define O2_FRAME()     ((int)PE_LoadU32(0x800E27ECu))
#define O2_POOL()      PE_LoadU32(PE_LoadU32(0x800F33E0u) + 8u)
#define O2_ACTOR()     PE_LoadU32(0x8009D254u)
#define O2_RAND()      ((int)func_80071A54())
#define O2_S16(a)      ((int)(int16_t)PE_LoadU16(a))
#define O2_SET16(a, v) PE_StoreU16((a), (uint16_t)(v))
#define A(k)           (a1 + 2u * (uint32_t)(k))
#define O2_MUL(x, y)   ((int)((uint32_t)(x) * (uint32_t)(y)))

/* The sprite-state prefix shared by these emitters:
 *   ix = *ixsym; D_800F3368 = size; D_800F336A = mode; D_800F3376 = size;
 *   D_800F3378 = size; av = D_800E2850[ix]; D_800F336C = 1;
 *   D_800F3370 = av; func_800CEDA8(1);
 * the callers store the rest in their own C order. */
static void o2_sprite_prefix(pe_addr_t ixsym, int size, int mode)
{
    unsigned ix = PE_LoadU16(ixsym);
    unsigned av;

    O2_SET16(0x800F3368u, size);
    O2_SET16(0x800F336Au, mode);
    O2_SET16(0x800F3376u, size);
    O2_SET16(0x800F3378u, size);
    av = PE_LoadU16(0x800E2850u + ix * 2u);
    O2_SET16(0x800F336Cu, 1);
    O2_SET16(0x800F3370u, av);
    func_800CEDA8(1);
}

/* w = D_800E1204[D_800F336C] + (D_800F336C == 4 && D_800F3428 ? hi : lo) */
static int o2_clut_index(int hi, int lo)
{
    unsigned idx = PE_LoadU16(0x800F336Cu);
    int w = (int)PE_LoadU16(0x800E1204u + idx * 2u);

    return w + ((idx == 4u && PE_LoadU32(0x800F3428u) != 0u) ? hi : lo);
}

static void o2_copy(pe_addr_t dst, pe_addr_t src, unsigned n)
{
    unsigned i;

    for (i = 0; i < n; i++)
        PE_StoreU8(dst + i, PE_LoadU8(src + i));
}

static void o2_pos_from(pe_addr_t pos, pe_addr_t a1)
{
    O2_SET16(pos + 0u, O2_S16(A(0)));
    O2_SET16(pos + 2u, O2_S16(A(1)));
    O2_SET16(pos + 4u, O2_S16(A(2)));
}

static void o2_vec(pe_addr_t vec, int v0, int v1, int v2, int v3)
{
    O2_SET16(vec + 0u, v0);
    O2_SET16(vec + 2u, v1);
    O2_SET16(vec + 4u, v2);
    O2_SET16(vec + 6u, v3);
}

/* src/func_800D7764.c: blk = D_800C22D8.  Mode 0: a1[4] = a1[5] = 0 and
 * the player position into a1 (func_800CE870 mode 0).  Mode 1 ends at 0x3C.
 * Mode 2: sprite state (ix D_800E11F6, 0x20/2, D_800F336E = 1,
 * D_800F3374 = 0x64), one func_800CEE20 sprite at a1 (texture 4, abr 3),
 * then unless D_800E2368->+0x1E == 0xB a func_800D0728 ring (abr 1 when
 * +0x1E == 0xA, else 3). */
int func_800D7764(int a0, pe_addr_t a1)
{
    const pe_addr_t pos = PE_OVL2_POS, vec = PE_OVL2_VEC, blk = PE_OVL2_BLK;
    int s0v, s2v, t, v, f;
    pe_addr_t q;

    o2_copy(blk, 0x800C22D8u, 4u);
    switch (a0) {
    case 0:
        q = O2_ACTOR();
        O2_SET16(A(4), 0);
        O2_SET16(A(5), 0);
        func_800CE870(q, 0, a1);
        return 0;
    case 1:
        if (O2_FRAME() < 0x3C)
            break;
        return 1;
    case 2:
        o2_sprite_prefix(0x800E11F6u, 0x20, 2);
        O2_SET16(0x800F336Eu, 1);
        O2_SET16(0x800F3372u, 0);
        O2_SET16(0x800F3374u, 0x64);
        s2v = func_80077CF4((O2_FRAME() << 11) / 60) / 32;
        s0v = 0x2800;
        t = func_80077AA4(0, o2_clut_index(6, 2));
        func_800CEE20(a1, 0u, s0v, s0v, 4, (uint32_t)(t & 0xFFFF), 3, s2v, 0u);
        if ((int16_t)PE_LoadU16(PE_LoadU32(0x800E2368u) + 0x1Eu) == 0xB)
            break;
        o2_pos_from(pos, a1);
        f = O2_FRAME();
        O2_SET16(vec + 0u, 0);
        O2_SET16(vec + 2u, 0);
        O2_SET16(vec + 4u, f << 3);
        O2_SET16(vec + 6u, 0);
        s0v = (O2_FRAME() & 1) * 60 + 0x1000;
        v = 3;
        if ((int16_t)PE_LoadU16(PE_LoadU32(0x800E2368u) + 0x1Eu) == 0xA)
            v = 1;
        func_800D0728(pos, 0x190, 0x1F4, 0x14, vec, s0v, s0v, 0u, blk, s2v, v);
        break;
    default:
        return 0;
    }
    return 0;
}

/* src/func_800D7B70.c: spiral of particles around the player.  Mode 0:
 * *(int *)(a1 + 4) = rand, *(int *)(a1 + 6) = 0, player position (mode 0)
 * into a1, allocate 0x18 x 8 (callback func_800D7A1C).  Mode 1 spawns one
 * particle per frame below 0x33 at a1 + (sin, cos)(angle) * radius / 4096,
 * p[3] = rand & 3, angle += (rand & 0x1F) + 0x8AA; ends at 0x4A.  Mode 2:
 * D_800F3374 = 8; below 0x33 radius = cos((T << 10) / 50) / 8 and a
 * func_800D0728 ring coloured by the D_800E18F0 curve; then sprite state
 * (ix D_800E11E6, 0x10/1, D_800F336E = 0, D_800F3374 = 4). */
int func_800D7B70(int a0, pe_addr_t a1)
{
    const pe_addr_t pos = PE_OVL2_POS, vec = PE_OVL2_VEC, blk = PE_OVL2_BLK;
    pe_addr_t p, q;
    int t, T, s0v, u, ang, rad;

    switch (a0) {
    case 0:
        t = O2_RAND();
        q = O2_ACTOR();
        PE_StoreU32(A(4), (uint32_t)t);
        PE_StoreU32(A(6), 0u);
        func_800CE870(q, 0, a1);
        return (int)func_800CE560(O2_POOL(), 8u, 0x18, 0x800D7A1Cu);
    case 1:
        if (O2_FRAME() < 0x33) {
            p = func_800CE610(O2_POOL());
            if (p != 0u) {
                ang = (int)PE_LoadU32(A(4));
                rad = (int)PE_LoadU32(A(6));
                O2_SET16(p + 0u, O2_S16(A(0)) + O2_MUL(func_80077DC4(ang), rad) / 4096);
                ang = (int)PE_LoadU32(A(4));
                rad = (int)PE_LoadU32(A(6));
                O2_SET16(p + 4u, O2_S16(A(2)) + O2_MUL(func_80077CF4(ang), rad) / 4096);
                O2_SET16(p + 2u, O2_S16(A(1)));
                O2_SET16(p + 6u, O2_RAND() & 3);
                t = (O2_RAND() & 0x1F) + 0x8AA;
                PE_StoreU32(A(4), PE_LoadU32(A(4)) + (uint32_t)t);
            }
        }
        if (O2_FRAME() < 0x4A)
            break;
        return 1;
    case 2:
        T = O2_FRAME();
        O2_SET16(0x800F3374u, 8);
        if (T < 0x33) {
            o2_pos_from(pos, a1);
            o2_vec(vec, 0x400, 0, T << 5, 1);
            s0v = func_80077CF4((T << 10) / 50);
            t = s0v / 8;
            u = O2_FRAME();
            PE_StoreU32(A(6), (uint32_t)t);
            func_800CF3AC(0x800E18F0u, blk, u);
            func_800D0728(pos, 350, 500, 20, vec, s0v, s0v, 0u, blk, 0x80, 1);
        }
        o2_sprite_prefix(0x800E11E6u, 0x10, 1);
        O2_SET16(0x800F336Eu, 0);
        O2_SET16(0x800F3372u, 0);
        O2_SET16(0x800F3374u, 4);
        break;
    default:
        return 0;
    }
    return 0;
}

/* Shared mode-2 body of func_800D7FBC / func_800D8E74 (both leaves spell it
 * out; the sequences are identical apart from the constants passed here):
 * stage a1[0] 0 -> 1 -> 2 advanced when a1[2] >= 0xC; draw one
 * func_800CEE20 sprite at a1[3..5] spun by D_800E27EC << 7. */
static void o2_orb_draw(pe_addr_t a1, int sprite_with_s2v, int texture,
                        int hi, int lo)
{
    const pe_addr_t pos = PE_OVL2_POS, vec = PE_OVL2_VEC, blk = PE_OVL2_BLK;
    int s1v, s2v, t;

    switch (O2_S16(A(0))) {
    case 0:
        s2v = func_80077CF4((O2_S16(A(2)) << 10) / 12) / 32;
        s1v = 0x1000;
        if (O2_S16(A(2)) >= 0xC) {
            O2_SET16(A(2), 0);
            O2_SET16(A(0), 1);
        }
        break;
    case 1:
        s1v = 0x1000;
        s2v = 0x80;
        if (O2_S16(A(2)) >= 0xC) {
            O2_SET16(A(2), 0);
            O2_SET16(A(0), 2);
        }
        break;
    default:
        s2v = 0x80;
        s1v = func_80077DC4((O2_S16(A(2)) << 10) / 12);
        O2_SET16(A(6), O2_S16(A(6)) + O2_S16(A(2)) * 4);
        break;
    }
    O2_SET16(pos + 0u, O2_S16(A(3)));
    O2_SET16(pos + 2u, O2_S16(A(4)));
    O2_SET16(pos + 4u, O2_S16(A(5)));
    o2_vec(vec, 0, 0, O2_FRAME() << 7, 0);
    s1v <<= 1;
    t = func_80077AA4(0, o2_clut_index(hi, lo));
    func_800CEE20(pos, vec, s1v, s1v, texture, (uint32_t)(t & 0xFFFF), 1,
                  sprite_with_s2v ? s2v : 0x80, blk);
}

/* src/func_800D7FBC.c: blk = D_800C22E0.  Mode 1: a1[3]/a1[5] orbit
 * (D_800E21EC, D_800E21F0) by sin/cos(a1[1]) * a1[6] / 4096, a1[1] += 0x60,
 * a1[6] = sin((T << 10) / 36) * 700 / 4096; every v-th frame (v = 1 when
 * D_800E2368->+0x1E == 0xB, else 3) a particle from pool D_800E21F4 at
 * a1[3..5] + (rand & 7) - 3; a1[2] += 1; ends at 0x24.  Mode 2: see
 * o2_orb_draw (texture 0x24, brightness s2v, clut +7/+3).
 * In stage 0 s2v = cos((a1[2] << 10) / 12) / 32 is the brightness. */
int func_800D7FBC(int a0, pe_addr_t a1)
{
    pe_addr_t p;
    int v;

    o2_copy(PE_OVL2_BLK, 0x800C22E0u, 4u);
    switch (a0) {
    case 1:
        O2_SET16(A(3), (int)PE_LoadU16(0x800E21ECu) +
                 O2_MUL(func_80077DC4(O2_S16(A(1))), O2_S16(A(6))) / 4096);
        O2_SET16(A(5), (int)PE_LoadU16(0x800E21F0u) +
                 O2_MUL(func_80077CF4(O2_S16(A(1))), O2_S16(A(6))) / 4096);
        O2_SET16(A(1), O2_S16(A(1)) + 0x60);
        O2_SET16(A(6), O2_MUL(func_80077DC4((O2_FRAME() << 10) / 36), 700) / 4096);
        v = 3;
        if ((int16_t)PE_LoadU16(PE_LoadU32(0x800E2368u) + 0x1Eu) == 0xB)
            v = 1;
        if (O2_FRAME() % v == 0) {
            p = func_800CE610(PE_LoadU32(0x800E21F4u));
            if (p != 0u) {
                O2_SET16(p + 0u, O2_S16(A(3)) + (O2_RAND() & 7) - 3);
                O2_SET16(p + 2u, O2_S16(A(4)) + (O2_RAND() & 7) - 3);
                O2_SET16(p + 4u, O2_S16(A(5)) + (O2_RAND() & 7) - 3);
            }
        }
        O2_SET16(A(2), O2_S16(A(2)) + 1);
        if (O2_FRAME() < 0x24)
            break;
        return 1;
    case 2:
        o2_orb_draw(a1, 1, 0x24, 7, 3);
        break;
    default:
        return 0;
    }
    return 0;
}

/* src/func_800D8E74.c: the same orbit about (D_800E2200, D_800E2204) with a
 * vertical sweep: a1[7] != 0 -> a1[4] = D_800E2202 - T * 500 / 36, else
 * D_800E2202 + T * 500 / 36 - 1000; spawns on even frames from pool
 * D_800E2208; ends at 0x24.  Mode 2: o2_orb_draw with texture 0xE4,
 * brightness 0x80 (stage 0's cos() result is computed and discarded),
 * clut +6/+2. */
int func_800D8E74(int a0, pe_addr_t a1)
{
    pe_addr_t p;
    int v;

    o2_copy(PE_OVL2_BLK, 0x800C22E0u, 4u);
    switch (a0) {
    case 1:
        O2_SET16(A(3), (int)PE_LoadU16(0x800E2200u) +
                 O2_MUL(func_80077DC4(O2_S16(A(1))), O2_S16(A(6))) / 4096);
        O2_SET16(A(5), (int)PE_LoadU16(0x800E2204u) +
                 O2_MUL(func_80077CF4(O2_S16(A(1))), O2_S16(A(6))) / 4096);
        if (O2_S16(A(7)) != 0) {
            O2_SET16(A(4), (int)PE_LoadU16(0x800E2202u) - O2_FRAME() * 500 / 36);
        } else {
            v = O2_FRAME() * 500 / 36 - 1000;
            O2_SET16(A(4), (int)PE_LoadU16(0x800E2202u) + v);
        }
        O2_SET16(A(1), O2_S16(A(1)) + 0x60);
        O2_SET16(A(6), O2_MUL(func_80077DC4((O2_FRAME() << 10) / 36), 700) / 4096);
        if (!(O2_FRAME() & 1)) {
            p = func_800CE610(PE_LoadU32(0x800E2208u));
            if (p != 0u) {
                O2_SET16(p + 0u, O2_S16(A(3)) + (O2_RAND() & 7) - 3);
                O2_SET16(p + 2u, O2_S16(A(4)) + (O2_RAND() & 7) - 3);
                O2_SET16(p + 4u, O2_S16(A(5)) + (O2_RAND() & 7) - 3);
            }
        }
        O2_SET16(A(2), O2_S16(A(2)) + 1);
        if (O2_FRAME() < 0x24)
            break;
        return 1;
    case 2:
        o2_orb_draw(a1, 0, 0xE4, 6, 2);
        break;
    default:
        return 0;
    }
    return 0;
}

/* src/func_800DACA4.c: blk = D_800C22E8, blk2 = D_800C22EC.  Mode 0:
 * *(int *)(a1 + 4) = rand, player position (mode 0) into a1, allocate
 * 0x18 x 8 with callback D_800DAB98.  Mode 1 below 0x33 spawns p[1] =
 * (rand & 0xFF) + 500, p[3] = (rand & 3) + 0x16, p[2] = the angle word,
 * angle += (rand & 0x1F) + 0x8AA; ends at 0x50.  Mode 2 below 0x51: two
 * func_800D004C bursts and a func_800D0728 ring (D_800F3374 = 0x3C), then
 * D_800F3374 = 0x40 and func_800CF5B0(a1, 0) — boundary. */
int func_800DACA4(int a0, pe_addr_t a1)
{
    const pe_addr_t pos = PE_OVL2_POS, blk = PE_OVL2_BLK, blk2 = PE_OVL2_BLK2;
    pe_addr_t p, q;
    int t, T, s0v, s1v;

    o2_copy(blk, 0x800C22E8u, 4u);
    o2_copy(blk2, 0x800C22ECu, 4u);
    switch (a0) {
    case 0:
        t = O2_RAND();
        q = O2_ACTOR();
        PE_StoreU32(A(4), (uint32_t)t);
        func_800CE870(q, 0, a1);
        return (int)func_800CE560(O2_POOL(), 8u, 0x18, 0x800DAB98u);
    case 1:
        if (O2_FRAME() < 0x33) {
            p = func_800CE610(O2_POOL());
            if (p != 0u) {
                O2_SET16(p + 2u, (O2_RAND() & 0xFF) + 500);
                O2_SET16(p + 6u, (O2_RAND() & 3) + 0x16);
                O2_SET16(p + 4u, PE_LoadU32(A(4)));
                t = (O2_RAND() & 0x1F) + 0x8AA;
                PE_StoreU32(A(4), PE_LoadU32(A(4)) + (uint32_t)t);
            }
        }
        if (O2_FRAME() < 0x50)
            break;
        return 1;
    case 2:
        T = O2_FRAME();
        if (T < 0x51) {
            O2_SET16(0x800F3374u, 0x3C);
            o2_pos_from(pos, a1);
            s1v = (T << 12) / 80;
            s0v = func_80077CF4((T << 11) / 80) / 32;
            func_800D004C(pos, 800, 800, 0x10, 0u, s1v, s1v, blk, 0u, s0v, 1);
            func_800D004C(pos, 250, 250, 8, 0u, s1v, s1v, blk2, 0u, s0v, 1);
            func_800D0728(pos, 590, 670, 0x10, 0u, s1v, s1v, 0u, blk, s0v, 3);
        }
        O2_SET16(0x800F3374u, 0x40);
        PE_Decomp_Boundary("func_800CF5B0", 0x800CF5B0u, 2u, a1, 0u, 0u, 0u);
        break;
    default:
        return 0;
    }
    return 0;
}

/* src/func_800DC5BC.c: jitter puff.  Mode 1: a1[0] = (u16)(a1[0] - 3) +
 * (rand & 7), a1[2] likewise, a1[1] = (u16)(a1[1] - 2) - (rand & 3); ends
 * at 0xE.  Mode 2: colour from the D_800E1E64 curve, one func_800CEE20
 * sprite at a1 (texture 0xE2, scale cos((T << 11) / 14), clut +6/+2). */
int func_800DC5BC(int a0, pe_addr_t a1)
{
    const pe_addr_t vec = PE_OVL2_VEC, blk = PE_OVL2_BLK;
    int s1v, t, r;
    unsigned short u;

    switch (a0) {
    case 1:
        r = O2_RAND();
        u = (unsigned short)(O2_S16(A(0)) - 3);
        O2_SET16(A(0), u + (r & 7));
        r = O2_RAND();
        u = (unsigned short)(O2_S16(A(2)) - 3);
        O2_SET16(A(2), u + (r & 7));
        r = O2_RAND();
        u = (unsigned short)(O2_S16(A(1)) - 2);
        O2_SET16(A(1), u - (r & 3));
        if (O2_FRAME() < 0xE)
            break;
        return 1;
    case 2:
        O2_SET16(vec + 0u, 0);
        O2_SET16(vec + 2u, 0);
        O2_SET16(vec + 6u, 0);
        O2_SET16(vec + 4u, O2_FRAME() << 7);
        func_800CF3AC(0x800E1E64u, blk, O2_FRAME());
        s1v = func_80077CF4((O2_FRAME() << 11) / 14);
        t = func_80077AA4(0, o2_clut_index(6, 2));
        func_800CEE20(a1, vec, s1v, s1v, 0xE2, (uint32_t)(t & 0xFFFF), 1, 0x80, blk);
        break;
    default:
        return 0;
    }
    return 0;
}

/* src/func_800DD380.c: rising wisp.  Mode 1: a1[5] += 1; ends once
 * a1[4] >= 2.  Mode 2 by stage a1[4]:
 *   0: pos = func_800783E4(a1, &D_800E2234, 0x1000 - s, s) blend with
 *      s = (a1[5] << 12) / 52; pos[0] += cos(a1[7] + T * 160) / 16;
 *      a1[1] -= 2; colour from the D_800E1FEC curve; scale cos(T << 7) / 2
 *      + 0x1000; stage 1 once a1[5] >= 0x34 (a1[5] = a1[6] = 0);
 *   1: pos = a1; blk[0] = b | b << 8 | (rand & 0xF) << 16 with
 *      b = cos((T << 11) / 12) / 32; once a1[5] >= 0xC, a1[6] += 1 and
 *      below 4 respawn at D_800E2234..38 +- (rand % 400 - 200) with
 *      a1[5] = 0, else a1[4] = 2;
 *   other: nothing is computed (scale 0, pos/blk as left in the scratch).
 * Then one func_800CEE20 sprite (vec[2] = T * 12, clut +4 when
 * D_800F336C == 4 && D_800F3428, base 0x20). */
int func_800DD380(int a0, pe_addr_t a1)
{
    const pe_addr_t pos = PE_OVL2_POS, vec = PE_OVL2_VEC, blk = PE_OVL2_BLK;
    int s0v, s2v, r, t, u;
    unsigned idx;
    int w;

    switch (a0) {
    case 1:
        O2_SET16(A(5), O2_S16(A(5)) + 1);
        if (O2_S16(A(4)) < 2)
            break;
        return 1;
    case 2:
        s2v = 0;
        switch (O2_S16(A(4))) {
        case 0:
            s0v = (O2_S16(A(5)) << 12) / 52;
            func_800783E4(a1, 0x800E2234u, 0x1000 - s0v, s0v, pos);
            O2_SET16(pos + 0u, O2_S16(pos + 0u) +
                     func_80077CF4(O2_S16(A(7)) + O2_FRAME() * 160) / 16);
            O2_SET16(A(1), O2_S16(A(1)) - 2);
            func_800CF3AC(0x800E1FECu, blk, (O2_FRAME() << 6) / 52);
            s2v = func_80077CF4(O2_FRAME() << 7) / 2 + 0x1000;
            if (O2_S16(A(5)) >= 0x34) {
                O2_SET16(A(4), 1);
                O2_SET16(A(5), 0);
                O2_SET16(A(6), 0);
            }
            break;
        case 1:
            o2_pos_from(pos, a1);
            s2v = func_80077CF4((O2_FRAME() << 11) / 12);
            s0v = s2v / 32;
            r = O2_RAND();
            u = O2_S16(A(5));
            PE_StoreU32(blk, (uint32_t)(s0v | (s0v << 8) | ((r & 0xF) << 16)));
            if (u >= 0xC) {
                O2_SET16(A(6), O2_S16(A(6)) + 1);
                if (O2_S16(A(6)) < 4) {
                    O2_SET16(A(0), O2_S16(0x800E2234u));
                    O2_SET16(A(1), O2_S16(0x800E2236u));
                    O2_SET16(A(2), O2_S16(0x800E2238u));
                    O2_SET16(A(0), O2_S16(A(0)) + O2_RAND() % 400 - 200);
                    O2_SET16(A(1), O2_S16(A(1)) + O2_RAND() % 400 - 200);
                    O2_SET16(A(2), O2_S16(A(2)) + O2_RAND() % 400 - 200);
                    O2_SET16(A(5), 0);
                } else {
                    O2_SET16(A(4), 2);
                }
            }
            break;
        }
        o2_vec(vec, 0, 0, O2_FRAME() * 12, 0);
        idx = PE_LoadU16(0x800F336Cu);
        w = (int)PE_LoadU16(0x800E1204u + idx * 2u);
        if (idx == 4u && PE_LoadU32(0x800F3428u) != 0u)
            w += 4;
        t = func_80077AA4(0x20, w);
        func_800CEE20(pos, vec, s2v, s2v, 0x80, (uint32_t)(t & 0xFFFF), 1, 0x80, blk);
        break;
    default:
        return 0;
    }
    return 0;
}

/* src/func_800DDD70.c: rising sparks at the player's stats-block anchor.
 * Mode 0: player position (mode 1) into a1, allocate 0x20 x 0x10 (callback
 * func_800DD9E4).  Mode 1 below 0x43 spawns p[0/2] = a1 +- (rand & 0x1FF)
 * - 0x100, p[1] = a1[1] - rand % 600, p[4] = p[5] = 0, p[6] = rand; ends at
 * 0x56.  Mode 2: pos = the three words at (D_8009D254->+0x238) + 0x274..
 * (truncated to short) also published to D_800E223C..40; below 0x57 a
 * func_800D004C burst and a func_800D0728 ring (blk byte 2 cleared, abr 3
 * on odd frames) with D_800F3374 = 0x10; then sprite state (ix D_800E11E6,
 * 0x20/2, D_800F336E = 0; D_800F3374 is not stored). */
int func_800DDD70(int a0, pe_addr_t a1)
{
    const pe_addr_t pos = PE_OVL2_POS, blk = PE_OVL2_BLK;
    pe_addr_t p;
    int T, s0v, v;

    switch (a0) {
    case 0:
        func_800CE870(O2_ACTOR(), 1, a1);
        return (int)func_800CE560(O2_POOL(), 0x10u, 0x20, 0x800DD9E4u);
    case 1:
        if (O2_FRAME() < 0x43) {
            p = func_800CE610(O2_POOL());
            if (p != 0u) {
                O2_SET16(p + 0u, O2_S16(A(0)) + (O2_RAND() & 0x1FF) - 0x100);
                O2_SET16(p + 2u, O2_S16(A(1)) - O2_RAND() % 600);
                O2_SET16(p + 4u, O2_S16(A(2)) + (O2_RAND() & 0x1FF) - 0x100);
                O2_SET16(p + 8u, 0);
                O2_SET16(p + 10u, 0);
                O2_SET16(p + 12u, O2_RAND());
            }
        }
        if (O2_FRAME() < 0x56)
            break;
        return 1;
    case 2:
        O2_SET16(pos + 0u, PE_LoadU32(PE_LoadU32(O2_ACTOR() + 0x238u) + 0x274u));
        O2_SET16(pos + 2u, PE_LoadU32(PE_LoadU32(O2_ACTOR() + 0x238u) + 0x278u));
        O2_SET16(pos + 4u, PE_LoadU32(PE_LoadU32(O2_ACTOR() + 0x238u) + 0x27Cu));
        T = O2_FRAME();
        O2_SET16(0x800E223Cu, O2_S16(pos + 0u));
        O2_SET16(0x800E223Eu, O2_S16(pos + 2u));
        O2_SET16(0x800E2240u, O2_S16(pos + 4u));
        if (T < 0x57) {
            s0v = (T << 12) / 86 + 0x800;
            O2_SET16(0x800F3374u, 0x10);
            func_800CF3AC(0x800E20ACu, blk, T);
            v = 1;
            if (O2_FRAME() & 1)
                v = 3;
            func_800D004C(pos, 500, 500, 0x10, 0u, s0v, s0v, blk, 0u, 0x80, 1);
            PE_StoreU8(blk + 2u, 0);
            func_800D0728(pos, 400, 300, 0x14, 0u, s0v, s0v, blk, 0u, 0x40, v);
        }
        o2_sprite_prefix(0x800E11E6u, 0x20, 2);
        O2_SET16(0x800F336Eu, 0);
        O2_SET16(0x800F3372u, 0);
        break;
    default:
        return 0;
    }
    return 0;
}

/* src/func_800DEA30.c: blk = D_800C22BC.  Mode 0: *(int *)(a1 + 4) = rand,
 * the target position (D_800F32D0->+8 record +0x268..) into a1, allocate
 * 0x20 x 0x10 (callback func_800DE7A8).  Mode 1 below 0x10 fills ONE
 * particle twice: v = (rand % 48 - 24, rand % 48 - 32, -(rand % 80)),
 * func_800CEAE8(stats, v, v) — boundary, v unchanged —, p[3..5] = v,
 * p[0..2] = a1 + (rand & 0xFF) - 0x80, p[7] = (rand & 0x3F) + 0x50,
 * p[6] = rand, angle += 0x955; ends at 0x30.  Mode 2: sprite state (ix
 * D_800E11F6, 0x20/2, D_800F3372 = 5, D_800F336E = 1, D_800F3374 = 0x40);
 * below 0x19 a1[1] -= 6 and a func_800CEE20 sprite (+ a func_800D004C burst
 * below 9); D_800F3374 = 0; below 0x11 a func_800D0728 ring; then sprite
 * state (ix D_800E11E6, D_800F336E = D_800F3372 = 0, D_800F3374 = 0x18). */
int func_800DEA30(int a0, pe_addr_t a1)
{
    const pe_addr_t vec = PE_OVL2_VEC, blk = PE_OVL2_BLK, v = PE_OVL2_V;
    pe_addr_t p, q;
    int t, T, i, s0v, s2v, T2;

    o2_copy(blk, 0x800C22BCu, 4u);
    switch (a0) {
    case 0:
        t = O2_RAND();
        q = PE_LoadU32(0x800F32D0u);
        PE_StoreU32(A(4), (uint32_t)t);
        O2_SET16(A(0), PE_LoadU16(PE_LoadU32(q + 8u) + 0x268u));
        O2_SET16(A(1), PE_LoadU16(PE_LoadU32(q + 8u) + 0x26Au));
        O2_SET16(A(2), PE_LoadU16(PE_LoadU32(q + 8u) + 0x26Cu));
        return (int)func_800CE560(O2_POOL(), 0x10u, 0x20, 0x800DE7A8u);
    case 1:
        if (O2_FRAME() < 0x10) {
            p = func_800CE610(O2_POOL());
            if (p != 0u) {
                i = 0;
                do {
                    i++;
                    O2_SET16(v + 0u, O2_RAND() % 48 - 24);
                    O2_SET16(v + 4u, -(O2_RAND() % 80));
                    O2_SET16(v + 2u, O2_RAND() % 48 - 32);
                    PE_Decomp_Boundary("func_800CEAE8", 0x800CEAE8u, 3u,
                                       PE_LoadU32(O2_ACTOR() + 0x238u), v, v, 0u);
                    O2_SET16(p + 6u, O2_S16(v + 0u));
                    O2_SET16(p + 8u, O2_S16(v + 2u));
                    O2_SET16(p + 10u, O2_S16(v + 4u));
                    O2_SET16(p + 0u, O2_S16(A(0)) + (O2_RAND() & 0xFF) - 0x80);
                    O2_SET16(p + 2u, O2_S16(A(1)) + (O2_RAND() & 0xFF) - 0x80);
                    O2_SET16(p + 4u, O2_S16(A(2)) + (O2_RAND() & 0xFF) - 0x80);
                    O2_SET16(p + 14u, (O2_RAND() & 0x3F) + 0x50);
                    O2_SET16(p + 12u, O2_RAND());
                    PE_StoreU32(A(4), PE_LoadU32(A(4)) + 0x955u);
                } while (i < 2);
            }
        }
        if (O2_FRAME() < 0x30)
            break;
        return 1;
    case 2:
        o2_sprite_prefix(0x800E11F6u, 0x20, 2);
        T = O2_FRAME();
        O2_SET16(0x800F3372u, 5);
        O2_SET16(0x800F336Eu, 1);
        O2_SET16(0x800F3374u, 0x40);
        if (T < 0x19) {
            O2_SET16(A(1), O2_S16(A(1)) - 6);
            s2v = O2_MUL(func_80077DC4((T << 10) / 24), 240) / 4096;
            o2_vec(vec, 0, 0, O2_FRAME() << 4, 0);
            s0v = 0x3000 - func_80077DC4((O2_FRAME() << 10) / 12);
            t = func_80077AA4(0, o2_clut_index(8, 4));
            func_800CEE20(a1, vec, s0v, s0v, 0x40, (uint32_t)(t & 0xFFFF), 1, s2v, 0u);
            if (O2_FRAME() < 9) {
                s2v = func_80077DC4(O2_FRAME() << 7) / 32;
                func_800D004C(a1, 100, 700, 0x10, vec, s0v, s0v, blk, 0u, s2v, 1);
            }
        }
        T2 = O2_FRAME();
        O2_SET16(0x800F3374u, 0);
        if (T2 < 0x11) {
            s2v = 0x80 - (T2 - 4) * 8;
            O2_SET16(v + 0u, O2_S16(A(0)));
            O2_SET16(v + 2u, O2_S16(A(1)));
            O2_SET16(v + 4u, O2_S16(A(2)));
            O2_SET16(vec + 0u, 0);
            O2_SET16(vec + 2u, 0);
            O2_SET16(vec + 4u, T2 << 7);
            s0v = func_80077CF4((T2 - 4) << 6) * 2;
            func_800D0728(v, 600, 700, 0x18, vec, s0v, s0v, 0u, blk, s2v, 1);
        }
        o2_sprite_prefix(0x800E11E6u, 0x20, 2);
        O2_SET16(0x800F336Eu, 0);
        O2_SET16(0x800F3372u, 0);
        O2_SET16(0x800F3374u, 0x18);
        break;
    default:
        return 0;
    }
    return 0;
}

/* src/func_800DEFFC.c: blk = D_800C22F0 (8 bytes).  Mode 1 by kind a1[8]:
 *   0: a1[9] += 1, func_800CE8F0(player, a1[3], &blk, a1); on frames
 *      T & 3 == 0 spawn one particle — one time in three (rand % 3 == 0)
 *      kind 2 at a1 with velocity ((rand & 7) - 3, (rand & 0xF) - 0xC,
 *      (rand & 7) - 3), otherwise kind 1 jittered by (rand & 0x7F) - 0x40
 *      with velocity ((rand & 0xF) - 7, (rand & 7) - 0xB, (rand & 0xF) - 7);
 *   1: integrate velocity a1[4..6] plus (rand & 7) - 3 jitter; ends when
 *      a1[9] reaches 0x10;
 *   2: integrate velocity; ends at 0x18;  other kinds return 0.
 * Mode 2 draws one func_800CEE20 sprite per kind (texture D_800F336A *
 * phase + 0x68 / 0x60, brightness capped by D_800E2244). */
int func_800DEFFC(int a0, pe_addr_t a1)
{
    const pe_addr_t blk = PE_OVL2_BLK, blk2 = PE_OVL2_BLK2;
    pe_addr_t p;
    int s0v, s2v, w, t;
    unsigned short idx;

    o2_copy(blk, 0x800C22F0u, 8u);
    switch (a0) {
    case 1:
        switch (O2_S16(A(8))) {
        case 0:
            O2_SET16(A(9), O2_S16(A(9)) + 1);
            func_800CE8F0(O2_ACTOR(), (uint32_t)O2_S16(A(3)), blk, a1);
            if (O2_FRAME() & 3)
                return 0;
            if (O2_RAND() % 3 == 0) {
                p = func_800CE610(O2_POOL());
                if (p == 0u)
                    break;
                O2_SET16(p + 0u, O2_S16(A(0)));
                O2_SET16(p + 2u, O2_S16(A(1)));
                O2_SET16(p + 4u, O2_S16(A(2)));
                O2_SET16(p + 8u, (O2_RAND() & 7) - 3);
                O2_SET16(p + 12u, (O2_RAND() & 7) - 3);
                O2_SET16(p + 10u, (O2_RAND() & 0xF) - 0xC);
                O2_SET16(p + 16u, 2);
                O2_SET16(p + 18u, 0);
            } else {
                p = func_800CE610(O2_POOL());
                if (p == 0u)
                    return 0;
                O2_SET16(p + 0u, O2_S16(A(0)));
                O2_SET16(p + 2u, O2_S16(A(1)));
                O2_SET16(p + 4u, O2_S16(A(2)));
                O2_SET16(p + 0u, O2_S16(p + 0u) + (O2_RAND() & 0x7F) - 0x40);
                O2_SET16(p + 2u, O2_S16(p + 2u) + (O2_RAND() & 0x7F) - 0x40);
                O2_SET16(p + 4u, O2_S16(p + 4u) + (O2_RAND() & 0x7F) - 0x40);
                O2_SET16(p + 8u, (O2_RAND() & 0xF) - 7);
                O2_SET16(p + 12u, (O2_RAND() & 0xF) - 7);
                O2_SET16(p + 10u, (O2_RAND() & 7) - 0xB);
                O2_SET16(p + 16u, 1);
                O2_SET16(p + 18u, 0);
            }
            break;
        case 1:
            O2_SET16(A(9), O2_S16(A(9)) + 1);
            O2_SET16(A(0), O2_S16(A(0)) + O2_S16(A(4)));
            O2_SET16(A(1), O2_S16(A(1)) + O2_S16(A(5)));
            O2_SET16(A(2), O2_S16(A(2)) + O2_S16(A(6)));
            O2_SET16(A(0), O2_S16(A(0)) + (O2_RAND() & 7) - 3);
            O2_SET16(A(1), O2_S16(A(1)) + (O2_RAND() & 7) - 3);
            O2_SET16(A(2), O2_S16(A(2)) + (O2_RAND() & 7) - 3);
            if (O2_S16(A(9)) < 0x10)
                break;
            return 1;
        case 2:
            O2_SET16(A(9), O2_S16(A(9)) + 1);
            O2_SET16(A(0), O2_S16(A(0)) + O2_S16(A(4)));
            O2_SET16(A(1), O2_S16(A(1)) + O2_S16(A(5)));
            O2_SET16(A(2), O2_S16(A(2)) + O2_S16(A(6)));
            if (O2_S16(A(9)) < 0x18)
                break;
            return 1;
        default:
            return 0;
        }
        break;
    case 2:
        switch (O2_S16(A(8))) {
        case 0:
            s2v = 0x1800;
            if (O2_S16(A(9)) < 0x21)
                s2v = func_80077CF4(O2_S16(A(9)) << 5) + 0x800;
            if (O2_S16(A(9)) < 0x19) {
                s0v = func_80077CF4((O2_S16(A(9)) << 10) / 24) / 32;
            } else {
                s0v = 0x64;
                if (O2_S16(A(9)) & 1)
                    s0v = 0x80;
            }
            if (O2_S16(0x800E2244u) < s0v)
                s0v = O2_S16(0x800E2244u);
            idx = PE_LoadU16(0x800F336Cu);
            w = (int)PE_LoadU16(0x800E1204u + idx * 2u);
            if (idx == 4 && PE_LoadU32(0x800F3428u) != 0u)
                w += 4;
            t = func_80077AA4(0x10, w);
            func_800CEE20(a1, 0u, s2v / 2, s2v / 2,
                          O2_S16(0x800F336Au) * (O2_S16(A(9)) & 3) + 0x68,
                          (uint32_t)(t & 0xFFFF), 1, s0v, 0u);
            break;
        case 1:
            s0v = O2_S16(A(9)) << 6;
            s2v = func_80077DC4(s0v) / 2 + 0x800;
            s0v = func_80077DC4(s0v) / 32;
            if (O2_S16(0x800E2244u) < s0v)
                s0v = O2_S16(0x800E2244u);
            idx = PE_LoadU16(0x800F336Cu);
            w = (int)PE_LoadU16(0x800E1204u + idx * 2u);
            if (idx == 4 && PE_LoadU32(0x800F3428u) != 0u)
                w += 4;
            t = func_80077AA4(0x10, w);
            func_800CEE20(a1, 0u, s2v, s2v,
                          O2_S16(0x800F336Au) * (O2_S16(A(9)) & 3) + 0x68,
                          (uint32_t)(t & 0xFFFF), 1, s0v / 2, 0u);
            break;
        case 2:
            s2v = func_80077CF4((O2_S16(A(9)) << 10) / 24) + 0x1000;
            func_800CF3AC(0x800E213Cu, blk2, O2_S16(A(9)));
            idx = PE_LoadU16(0x800F336Cu);
            w = (int)PE_LoadU16(0x800E1204u + idx * 2u);
            if (idx == 4 && PE_LoadU32(0x800F3428u) != 0u)
                w += 4;
            t = func_80077AA4(0, w);
            func_800CEE20(a1, 0u, s2v / 2, s2v,
                          O2_S16(0x800F336Au) * (short)(O2_S16(A(9)) / 6) + 0x60,
                          (uint32_t)(t & 0xFFFF), 1, 0x80, blk2);
            break;
        default:
            return 0;
        }
        break;
    default:
        return 0;
    }
    return 0;
}
