/* room_m0429i — func_80199C28, blob offset 0xAC40, 0x42C bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (2026-10-08, written from the retail asm).
 * Particle callback registered by the effect callback func_8019A054
 * (particles: x y z spin, velocity at +8, kind at +0x10, frame at +0x12).
 * Mode 1 steps a particle: drift, damping (63/64 for kind 0, 31/32 and spin
 * +20 for kind 1), falls +1/frame and bounces off the floor (D_800942EC);
 * done after 24 frames. Mode 2 draws it through func_800CEE20: kind 0 as a
 * growing, rotating flash (16x16 sprite), kind 1 as a 32x32 sprite sized by
 * sin(frame). Sibling of func_801991F8.
 * Matching notes: plain sprite globals; D_800F336C read straight into the
 * clut lookup (one CSE'd load); the frame/8 term has its own variable. */
typedef struct { short vx, vy, vz, pad; } SVECTOR;

extern short D_800942EC[];
extern short D_800F3368;
extern short D_800F336A;
extern unsigned short D_800F336C;
extern short D_800F3376;
extern short D_800F3378;
extern int D_800F3428;
extern unsigned short D_800E1204[];
extern int func_80077AA4();
extern int func_80077CF4();
extern int func_80077DC4();
extern void func_800CEE20();

int func_80199C28(int mode, short *p)
{
    SVECTOR ang;
    int t;
    int size;
    int scale;
    int clut;
    int n;

    switch (mode) {
    case 1:
        switch (p[8]) {
        case 0:
            p[9]++;
            p[0] += p[4];
            p[1] += p[5];
            p[2] += p[6];
            p[4] = p[4] * 63 / 64;
            p[6] = p[6] * 63 / 64;
            p[5] += 1;
            if (p[1] >= D_800942EC[0])
                p[5] *= -1;
            if (p[9] >= 24)
                return 1;
            break;
        case 1:
            p[9]++;
            p[3] += 20;
            p[0] += p[4];
            p[1] += p[5];
            p[2] += p[6];
            p[4] = p[4] * 31 / 32;
            p[6] = p[6] * 31 / 32;
            p[5] += 1;
            if (p[1] >= D_800942EC[0])
                p[5] *= -1;
            if (p[9] >= 24)
                return 1;
            break;
        }
        break;
    case 2:
        switch (p[8]) {
        case 0:
            t = (p[9] << 10) / 24;
            size = func_80077CF4(t) + 8192;
            scale = func_80077DC4(t) / 80;
            ang.vx = 0;
            ang.vy = 0;
            ang.vz = p[3] + p[9] * 32;
            ang.pad = 0;
            n = p[9] / 8;
            D_800F3368 = 16;
            D_800F336A = 1;
            D_800F3376 = 16;
            D_800F3378 = 16;
            clut = D_800E1204[D_800F336C];
            if (D_800F336C == 4 && D_800F3428 != 0)
                clut += 4;
            func_800CEE20(p, &ang, size, size, D_800F336A * n + 253,
                          func_80077AA4(128, clut) & 0xFFFF, 1, scale, 0);
            break;
        case 1:
            t = (p[9] << 10) / 24;
            scale = func_80077DC4(t) / 46;
            ang.vz = p[3];
            size = func_80077DC4(t) / 4;
            D_800F3368 = 32;
            D_800F336A = 2;
            D_800F3376 = 32;
            D_800F3378 = 32;
            clut = D_800E1204[D_800F336C];
            if (D_800F336C == 4 && D_800F3428 != 0)
                clut += 4;
            func_800CEE20(p, &ang, size, size, 152,
                          func_80077AA4(16, clut) & 0xFFFF, 1, scale, 0);
            break;
        }
        break;
    }
    return 0;
}
