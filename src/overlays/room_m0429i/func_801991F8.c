/* room_m0429i — func_801991F8, blob offset 0xA210, 0x48C bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (2026-10-08, written from the retail asm).
 * Particle callback registered by the effect callback func_80199684
 * (particles: x y z spin, velocity at +8, kind at +0x10, frame at
 * +0x12). Mode 1 steps a particle: kind 0 drifts with 63/64 damping, falls
 * (+1/frame) and bounces off the floor (D_800942EC), done after 24 frames;
 * kind 1 also spins +32 and is done after 16. Mode 2 draws it through
 * func_800CEE20: kind 0 as a growing, rotating flash (cos/sin of frame), kind
 * 1 as a shrinking tinted sprite (colour D_8018F244).
 * Matching notes: the sprite globals are plain (not volatile) stores so reorg
 * can fill the bne delay slot; the frame/8 term has its own variable; the
 * bounce is `*= -1` (retail re-extends the halfword). */
typedef struct { short vx, vy, vz, pad; } SVECTOR;
typedef struct { unsigned char r, g, b, cd; } CVECTOR;

extern short D_800942EC[];
extern short D_800F3368;
extern short D_800F336A;
extern unsigned short D_800F336C;
extern short D_800F3376;
extern short D_800F3378;
extern int D_800F3428;
extern unsigned short D_800E1204[];
extern CVECTOR D_8018F244;
extern int func_80077AA4();
extern int func_80077CF4();
extern int func_80077DC4();
extern void func_800CEE20();

int func_801991F8(int mode, short *p)
{
    SVECTOR ang;
    CVECTOR col = D_8018F244;
    int t;
    int size;
    int scale;
    int idx;
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
            p[3] += 32;
            p[0] += p[4];
            p[1] += p[5];
            p[2] += p[6];
            p[4] = p[4] * 63 / 64;
            p[6] = p[6] * 63 / 64;
            p[5] += 0;
            if (p[1] >= D_800942EC[0])
                p[5] *= -1;
            if (p[9] >= 16)
                return 1;
            break;
        }
        break;
    case 2:
        switch (p[8]) {
        case 0:
            t = (p[9] << 10) / 24;
            size = func_80077CF4(t) + 6144;
            scale = func_80077DC4(t) / 128;
            ang.vx = 0;
            ang.vy = 0;
            ang.vz = p[3] + p[9] * 32;
            ang.pad = 0;
            n = p[9] / 8;
            idx = D_800F336C;
            D_800F3368 = 16;
            D_800F336A = 1;
            D_800F3376 = 16;
            D_800F3378 = 16;
            clut = D_800E1204[idx];
            if (idx == 4 && D_800F3428 != 0)
                clut += 4;
            func_800CEE20(p, &ang, size, size, D_800F336A * n + 253,
                          func_80077AA4(128, clut) & 0xFFFF, 1, scale, 0);
            break;
        case 1:
            t = p[9] << 6;
            scale = func_80077DC4(t) / 64 + 16;
            ang.vz = p[3];
            size = func_80077CF4(t) / 2 + 2048;
            idx = D_800F336C;
            D_800F3368 = 16;
            D_800F336A = 1;
            D_800F3376 = 16;
            D_800F3378 = 16;
            clut = D_800E1204[idx];
            if (idx == 4 && D_800F3428 != 0)
                clut += 4;
            func_800CEE20(p, &ang, size, size, D_800F336A * (p[9] / 2) + 184,
                          func_80077AA4(128, clut) & 0xFFFF, 1, scale, &col);
            break;
        }
        break;
    }
    return 0;
}
