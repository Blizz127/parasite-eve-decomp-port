/* room_m0429i — func_80199684, blob offset 0xA69C, 0x5A4 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (2026-10-08, written from the retail asm).
 * Effect callback (called through func_800D413C after func_8019AE8C). Mode 0
 * takes three bone points of the current actor (func_800CE8F0), draws the two
 * gradient segments between them (func_800D1384, alpha 128) and opens a
 * 20-entry particle pool with callback D_801991F8. Mode 1 runs 12 frames:
 * each frame spawns one particle at the third bone point projected to the
 * floor and one at the point itself, then fades the segments by 16; returns 2
 * when done. Mode 2 redraws the segments while the alpha is >= 0 and sets the
 * sprite globals (16x16, page from D_800E2850).
 * Matching notes: the first zero store goes through an in-struct view so the
 * scheduler can hoist the D_800F32D0 load above it (gcc 2.7 alias rule);
 * `if (...) return 2; break;` keeps retail's branch; the mode-2 block follows
 * the vendor template (volatile globals and the register fence). */
typedef struct { short vx, vy, vz, pad; } SVECTOR;
typedef struct { unsigned char r, g, b, cd; } CVECTOR;

extern int *D_800F32D0;
extern int *D_800F33E0;
extern short D_800942EC[];
extern volatile short D_800F3368;
extern volatile short D_800F336A;
extern volatile short D_800F336C;
extern volatile short D_800F336E;
extern volatile short D_800F3370;
extern short D_800F3372;
extern short D_800F3374;
extern volatile short D_800F3376;
extern volatile short D_800F3378;
extern unsigned short D_800E11E8;
extern unsigned short D_800E2850[];
extern short D_8019B55C[];
extern SVECTOR D_8018F248;
extern SVECTOR D_8018F250;
extern CVECTOR D_8018F258;
extern CVECTOR D_8018F25C;
extern CVECTOR D_8018F260;
extern int D_801991F8;
extern int func_80071A54();
extern int func_800CE560();
extern int func_800CE610();
extern void func_800CE8F0();
extern void func_800D1384();

typedef struct { short v[100]; } EFF;

int func_80199684(int mode, short *p)
{
    SVECTOR a;
    SVECTOR b;
    SVECTOR c;
    SVECTOR sv0 = D_8018F248;
    SVECTOR sv1 = D_8018F250;
    CVECTOR c0 = D_8018F258;
    CVECTOR c1 = D_8018F25C;
    CVECTOR c2 = D_8018F260;
    short *q;

    switch (mode) {
    case 0:
        p[2] = 128;
        ((EFF *)p)->v[0] = 0;
        p[1] = 0;
        func_800CE8F0(D_800F32D0[2], D_8019B55C[0], &sv0, &a);
        func_800CE8F0(D_800F32D0[2], D_8019B55C[1], &sv0, &b);
        func_800CE8F0(D_800F32D0[2], D_8019B55C[2], &sv1, &c);
        func_800D1384(&a, &b, 1010, &c0, &c1, 128, p + 4, 1);
        func_800D1384(&b, &c, 1010, &c1, &c2, 128, p + 92, 1);
        return func_800CE560(D_800F33E0[2], 20, 36, &D_801991F8);
    case 1:
        p[1]++;
        func_800CE8F0(D_800F32D0[2], D_8019B55C[2], &sv0, &a);
        b.vx = a.vx;
        b.vy = a.vy;
        b.vz = a.vz;
        a.vy = D_800942EC[0];
        if (p[1] < 13) {
            q = (short *)func_800CE610(D_800F33E0[2]);
            if (q != 0) {
                q[0] = a.vx;
                q[1] = a.vy;
                q[2] = a.vz;
                q[4] = func_80071A54() % 32 - 16;
                q[5] = -(func_80071A54() % 32);
                q[6] = func_80071A54() % 32 - 16;
                q[8] = 0;
                q[9] = 0;
            }
            q = (short *)func_800CE610(D_800F33E0[2]);
            if (q != 0) {
                q[0] = b.vx;
                q[1] = b.vy;
                q[2] = b.vz;
                q[4] = func_80071A54() % 32 - 16;
                q[5] = func_80071A54() % 32 - 16;
                q[6] = func_80071A54() % 32 - 16;
                q[8] = 1;
                q[9] = 0;
            }
        } else if (p[2] > 0) {
            p[2] -= 16;
        }
        if (p[1] >= 12)
            return 2;
        break;
    case 2:
        if (p[2] >= 0) {
            func_800CE8F0(D_800F32D0[2], D_8019B55C[0], &sv0, &a);
            func_800CE8F0(D_800F32D0[2], D_8019B55C[1], &sv0, &b);
            func_800CE8F0(D_800F32D0[2], D_8019B55C[2], &sv1, &c);
            func_800D1384(&a, &b, 10, &c0, &c1, p[2], p + 4, 1);
            func_800D1384(&b, &c, 10, &c1, &c2, p[2], p + 92, 1);
        }
    {
        int value;
        int index;
        index = D_800E11E8;
        D_800F3368 = 16;
        D_800F336A = 1;
        D_800F3376 = 16;
        D_800F3378 = 16;
        value = D_800E2850[index];
        asm("" : : "r"(value) : "$2", "memory");
        D_800F336C = 2;
        D_800F336E = 0;
        D_800F3372 = 0;
        D_800F3374 = 16;
        D_800F3370 = value;
    }
        break;
    }
    return 0;
}
