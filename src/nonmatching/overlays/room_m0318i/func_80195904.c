/* room_m0318i — func_80195904, blob offset 0x691C, 0x41C bytes.
 * NON_MATCHING behaviour C written from the retail asm (2026-10-18).
 * Particle callback, same shape as room_m0429i func_80199C28 (halfword
 * layout: x y z spin at +0..+6, velocity at +8/+0xA/+0xC, kind at +0x10,
 * frame at +0x12). Mode 1 steps a particle: drift, damping (127/128 for
 * kind 0, 31/32 and a floor bounce for kind 1); done after 24 frames.
 * Mode 2 draws it through func_800CEE20 as a sprite sized by sin/cos of
 * frame*1024/24 (kind 0: 32x32 sprite, kind 1: 64x64 sprite). */
typedef struct { short vx, vy, vz, pad; } SVECTOR;

extern short D_800942EC;
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

int func_80195904(int mode, short *p)
{
    SVECTOR ang;
    int t;
    int s1;
    int s2;
    int clut;
    int r;

    switch (mode) {
    case 1:
        switch (p[8]) {
        case 0:
            p[0] += p[4];
            p[9] += 1;
            p[1] += p[5];
            p[2] += p[6];
            p[4] = p[4] * 127 / 128;
            p[5] = p[5] * 127 / 128;
            p[6] = p[6] * 127 / 128;
            break;
        case 1:
            p[0] += p[4];
            p[9] += 1;
            p[1] += p[5];
            p[2] += p[6];
            p[4] = p[4] * 31 / 32;
            p[6] = p[6] * 31 / 32;
            p[5] += 1;
            if (p[1] >= D_800942EC)
                p[5] = -p[5];
            break;
        default:
            return 0;
        }
        if (p[9] >= 24)
            return 1;
        return 0;
    case 2:
        switch (p[8]) {
        case 0:
            t = (p[9] << 10) / 24;
            s2 = ((func_80077CF4(t) + 0x800) * p[3]) / 4096;
            s1 = func_80077DC4(t) / 32 + 0x28;
            ang.vx = 0;
            ang.vy = 0;
            D_800F3368 = 0x20;
            D_800F3376 = 0x20;
            D_800F3378 = 0x20;
            ang.pad = 0;
            D_800F336A = 2;
            ang.vz = p[9] << 5;
            clut = D_800E1204[D_800F336C];
            if (D_800F336C == 4 && D_800F3428 != 0)
                r = func_80077AA4(0, clut + 8);
            else
                r = func_80077AA4(0, clut + 4);
            func_800CEE20(p, &ang, s2, s2, 6, r & 0xFFFF, 1, s1, 0);
            return 0;
        case 1:
            t = (p[9] << 10) / 24;
            s2 = func_80077CF4(t) + 0x1000;
            s1 = func_80077CF4(t * 2) / 32;
            ang.vx = 0;
            ang.vy = 0;
            D_800F3368 = 0x40;
            D_800F3376 = 0x40;
            D_800F3378 = 0x40;
            ang.pad = 0;
            D_800F336A = 4;
            ang.vz = p[9] * 14;
            clut = D_800E1204[D_800F336C];
            if (D_800F336C == 4 && D_800F3428 != 0)
                r = func_80077AA4(0, clut + 6);
            else
                r = func_80077AA4(0, clut + 2);
            func_800CEE20(p, &ang, s2 * 3, s2 * 3, 0, r & 0xFFFF, 1, s1, 0);
            return 0;
        default:
            return 0;
        }
    default:
        return 0;
    }
}
