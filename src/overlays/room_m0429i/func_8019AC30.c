/* room_m0429i — func_8019AC30, blob offset 0xBC48, 0x25C bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (2026-10-08, written from the retail asm).
 * Spark particle callback registered by the effect callback func_8019AE8C
 * (pool of 0x14-byte particles: x y z spin, velocity at +8, size at +0xE,
 * frame at +0x12). Mode 1 steps a live spark: drift by its velocity, 63/64
 * damping on x and z, bounce off the floor (D_800942EC), +-3 jitter, spin
 * +8; done after 8 frames. Mode 2 draws it as a rotating sprite through
 * func_800CEE20 (size 6 x +0xE, brightness 184 + D_800F336A x frame).
 * Matching notes: the floor declared as an array (as in room_m0432i
 * func_80192548); the y velocity's `+= 0` keeps retail's store; an unused
 * SVECTOR sizes the frame; `break` to the single `return 0`. */
typedef struct { short vx, vy, vz, pad; } SVECTOR;

extern SVECTOR D_8018F1CC;
extern short D_800942EC[];
extern unsigned short D_800E1204[];
extern short D_800F336A;
extern unsigned short D_800F336C;
extern int D_800F3428;
extern int func_80071A54();
extern int func_80077AA4();
extern int func_80077DC4();
extern void func_800CEE20();

int func_8019AC30(int mode, short *p)
{
    SVECTOR ang = D_8018F1CC;
    SVECTOR pos;
    int s;
    int clut;

    switch (mode) {
    case 1:
        if (p[8] != 0)
            break;
        p[9]++;
        p[0] += p[4];
        p[1] += p[5];
        p[2] += p[6];
        p[4] = p[4] * 63 / 64;
        p[6] = p[6] * 63 / 64;
        p[5] += 0;
        if (p[1] >= D_800942EC[0])
            p[5] *= -1;
        p[0] += (func_80071A54() & 7) - 3;
        p[1] += (func_80071A54() & 7) - 3;
        p[2] += (func_80071A54() & 7) - 3;
        p[3] += 8;
        if (p[9] >= 8)
            return 1;
        break;
    case 2:
        if (p[8] != 0)
            break;
        func_80077DC4(p[9] << 5);
        ang.vz = p[3];
        clut = D_800E1204[D_800F336C];
        s = p[7] << 2;
        if (D_800F336C == 4 && D_800F3428 != 0)
            clut += 4;
        func_800CEE20(p, &ang, (s * 3) >> 1, (s * 3) >> 1, D_800F336A * p[9] + 184,
                      func_80077AA4(208, clut) & 0xFFFF, 1, 88, 0);
        break;
    }
    return 0;
}
