/* room_m0432i — func_801931B8, blob offset 0x41D0, 0x25C bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (2026-10-08, written from the retail asm).
 * Particle callback started by func_80193414: mode 1 drifts with 63/64
 * damping and a floor bounce, done after 8 frames; mode 2 draws the
 * rotating sprite. Matching notes: ang.pad after ang.vz and p[9] * 32 (not
 * << 5) give retail's lh; D_800F336A between the 0x20 stores and the clut
 * offset as one ternary keep retail's constant scheduling.
 * p[] layout: x, y, z, (pad), vx, vy, vz, (pad), paused, frame. */

typedef struct { short vx, vy, vz, pad; } SVECTOR;
typedef struct { unsigned char r, g, b, cd; } CVECTOR;
extern short D_800942EC[];
extern unsigned short D_800E1204[];
extern short D_800F3368;
extern short D_800F336A;
extern unsigned short D_800F336C;
extern short D_800F3376;
extern short D_800F3378;
extern int D_800F3428;
extern int D_80199190[];
extern int func_80077AA4();
extern int func_80077DC4();
extern void func_800CEE20();
extern void func_800CF3AC();

int func_801931B8(int mode, short *p)
{
    SVECTOR ang;
    CVECTOR col;
    int t;
    int sz;
    int w;
    int clut;

    switch (mode) {
    case 1:
        if (p[8] == 0) {
            p[9]++;
            p[0] += p[4];
            p[1] += p[5];
            p[2] += p[6];
            p[4] = p[4] * 63 / 64;
            p[6] = p[6] * 63 / 64;
            p[5] -= 0;
            if (p[1] >= D_800942EC[0]) {
                p[5] *= -1;
            }
            if (p[9] >= 8) {
                return 1;
            }
        }
        break;
    case 2:
        if (p[8] == 0) {
            t = p[9] << 7;
            sz = func_80077DC4(t) / 2 + 0x800;
            w = func_80077DC4(t) / 32;
            ang.vx = 0;
            ang.vy = 0;
            ang.vz = p[9] * 32;
            ang.pad = 0;
            func_800CF3AC(D_80199190, &col, p[9] * 2);
            D_800F3368 = 0x20;
            D_800F336A = 2;
            D_800F3376 = 0x20;
            D_800F3378 = 0x20;
            clut = D_800E1204[D_800F336C] + ((D_800F336C == 4 && D_800F3428) ? 8 : 4);
            func_800CEE20(p, &ang, sz, sz, D_800F336A + 0x64, func_80077AA4(0, clut) & 0xFFFF, 1, w, &col);
        }
        break;
    }
    return 0;
}
