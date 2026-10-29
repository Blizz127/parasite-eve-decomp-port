/* room_m0349i — func_8018F010, blob offset 0x28, 0x7B4 bytes. Profile
 * era_o2_g0 (-O2 -G0); LINK_EXACT at the target VMA (2026-10-08, written
 * from the retail asm).
 * Particle task callback started by the emitter func_8018F7C4. Mode 1 moves
 * one particle by kind (p[8]): 0 and 1 are drifting sparks (velocity decays
 * by 63/64, bounces off the floor height D_800942EC, random jitter, done at
 * frame 48 / 32), 2 is a rising puff (done at frame 8). Mode 2 draws it:
 * kinds 0 and 1 as animated sprites, kind 2 as a sprite plus a ring
 * (func_800D004C).
 * Matching notes: `p[9]++` comes first; `p[5] *= -1` keeps retail's
 * sign-extend before the negate (`-p[5]` folds it away); D_800F3374 as an
 * array keeps its store after the p[7] load in the kind-2 draw. */

typedef struct { short vx, vy, vz, pad; } SVECTOR;
typedef struct { unsigned char r, g, b, cd; } CVECTOR;
extern SVECTOR D_8018EFF4;
extern CVECTOR D_8018EFFC;
extern short D_800942EC[];
extern unsigned short D_800E1204[];
extern short D_800F336A[];
extern unsigned short D_800F336C;
extern short D_800F3374[];
extern int D_800F3428;
extern int func_80071A54();
extern int func_80077AA4();
extern int func_80077CF4();
extern int func_80077DC4();
extern void func_800CEE20();
extern void func_800D004C();

int func_8018F010(int mode, short *p)
{
    SVECTOR pos;
    CVECTOR col;
    int t;
    int w;
    int sz;
    int fr;
    int clut;
    int c;

    pos = D_8018EFF4;
    col = D_8018EFFC;
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
            p[5] -= 1;
            if (p[1] >= D_800942EC[0]) {
                p[5] *= -1;
            }
            p[4] += (func_80071A54() & 0xF) + -7;
            p[5] += (func_80071A54() & 0xF) + -7;
            p[6] += (func_80071A54() & 0xF) + -7;
            if (p[9] >= 0x30) {
                return 1;
            }
            break;
        case 1:
            p[9]++;
            p[0] += p[4];
            p[1] += p[5];
            p[2] += p[6];
            p[4] = p[4] * 63 / 64;
            p[6] = p[6] * 63 / 64;
            p[5] -= 2;
            if (p[1] >= D_800942EC[0]) {
                p[5] *= -1;
            }
            p[4] += (func_80071A54() & 7) + -3;
            p[5] += (func_80071A54() & 7) + -3;
            p[6] += (func_80071A54() & 7) + -3;
            if (p[9] >= 0x20) {
                return 1;
            }
            break;
        case 2:
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
            p[0] += (func_80071A54() & 7) + -3;
            p[1] += (func_80071A54() & 7) + -3;
            p[2] += (func_80071A54() & 7) + -3;
            p[3] += 8;
            if (p[9] >= 8) {
                return 1;
            }
            break;
        }
        break;
    case 2:
        switch (p[8]) {
        case 0:
            t = (p[9] << 10) / 48;
            w = func_80077CF4(t * 2) / 32;
            sz = func_80077DC4(t) / 2 + 0x2000;
            pos.vz = p[3] - p[9] * 8;
            fr = (p[9] / 2) & 7;
            if (fr >= 4) {
                fr += 12;
            }
            D_800F3374[0] = 0x226;
            clut = D_800E1204[D_800F336C];
            if (D_800F336C == 4 && D_800F3428) {
                clut += 4;
            }
            func_800CEE20(p, &pos, sz, sz, D_800F336A[0] * fr, func_80077AA4(0, clut) & 0xFFFF, 2, w / 3, 0);
            break;
        case 1:
            t = p[9] << 5;
            w = func_80077CF4(t * 2) / 32;
            sz = func_80077DC4(t) / 4 + 0x2000;
            pos.vz = p[3] + p[9] * 12;
            fr = (p[9] / 2) & 7;
            if (fr >= 4) {
                fr += 12;
            }
            D_800F3374[0] = 0x80;
            clut = D_800E1204[D_800F336C];
            if (D_800F336C == 4 && D_800F3428) {
                clut += 4;
            }
            func_800CEE20(p, &pos, sz, sz, D_800F336A[0] * fr, func_80077AA4(0, clut) & 0xFFFF, 3, w, 0);
            break;
        case 2:
            c = func_80077DC4(p[9] << 5);
            pos.vz = p[3];
            sz = p[7];
            D_800F3374[0] = 0x80;
            clut = D_800E1204[D_800F336C];
            w = c / 40;
            if (D_800F336C == 4 && D_800F3428) {
                clut += 4;
            }
            func_800CEE20(p, &pos, sz, sz * 2, D_800F336A[0] * (p[9] / 2) + 0x80, func_80077AA4(0x10, clut) & 0xFFFF, 1, 0x50, 0);
            func_800D004C(p, 500, 500, 8, 0, sz, sz, &col, 0, w, 3);
            break;
        }
        break;
    }
    return 0;
}
