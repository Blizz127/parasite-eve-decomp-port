/* room_m0432i — func_80193414, blob offset 0x442C, 0x52C bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (2026-10-08, written from the retail asm).
 * Case 2 loads the palette entry before the D_800F336C.. stores (barrier).
 * 3-mode effect controller: arm (two beam segments between three actor
 * joints, then the particle task), per-frame (emit from the third joint,
 * fade the beam after frame 32), and draw/palette setup.
 * p[] layout: (unused), frame, alpha, joint set, beam work areas at +8/+0xB8. */

typedef struct { short vx, vy, vz, pad; } SVECTOR;
typedef struct { unsigned char r, g, b, cd; } CVECTOR;
typedef struct { short a, b, c; } JointSet;
/* Head of the effect work area; a member store (not a bare *p) lets gcc 2.7
 * schedule the D_800F32D0 load ahead of it, as retail does. */
typedef struct { short unused; } EffectHead;
extern SVECTOR D_8018F1D8;
extern SVECTOR D_8018F1E0;
extern CVECTOR D_8018F1E8;
extern CVECTOR D_8018F1EC;
extern CVECTOR D_8018F1F0;
extern JointSet D_801991B8[];
extern unsigned char *D_800E2368;
extern unsigned char *D_800F32D0;
extern unsigned char *D_800F33E0;
extern int D_800E27EC;
extern unsigned short D_800E11FA;
extern unsigned short D_800E2850[];
extern short D_800F336C;
extern short D_800F336E;
extern short D_800F3370;
extern short D_800F3372;
extern short D_800F3374;
extern void func_801931B8();
extern int func_800CE560();
extern void func_800CE8F0();
extern void func_800CE9D4();
extern short *func_800CE610();
extern void func_800CFB7C();
extern void func_800D1384();
extern int func_800D3FD8();
extern void func_800D3F64();

int func_80193414(int mode, short *p)
{
    SVECTOR a;
    SVECTOR b;
    SVECTOR c;
    SVECTOR q;
    SVECTOR off0;
    SVECTOR off1;
    CVECTOR c0;
    CVECTOR c1;
    CVECTOR c2;
    short *e;
    unsigned char *ctx;

    off0 = D_8018F1D8;
    off1 = D_8018F1E0;
    c0 = D_8018F1E8;
    c1 = D_8018F1EC;
    c2 = D_8018F1F0;
    switch (mode) {
    case 0:
        ctx = D_800E2368;
        p[2] = 0x80;
        ((EffectHead *)p)->unused = 0;
        p[1] = 0;
        p[3] = *(unsigned short *)(ctx + 0x12);
        func_800CE8F0(*(void **)(D_800F32D0 + 8), D_801991B8[p[3]].a, &off0, &a);
        func_800CE8F0(*(void **)(D_800F32D0 + 8), D_801991B8[p[3]].b, &off0, &b);
        func_800CE8F0(*(void **)(D_800F32D0 + 8), D_801991B8[p[3]].c, &off1, &c);
        func_800D1384(&a, &b, 0x3F2, &c2, &c1, 0x80, p + 4, 1);
        func_800D1384(&b, &c, 0x3F2, &c1, &c0, 0x80, p + 0x5C, 1);
        return func_800CE560(*(void **)(D_800F33E0 + 8), 0x14, 8, func_801931B8);
    case 1:
        p[1]++;
        func_800CE8F0(*(void **)(D_800F32D0 + 8), D_801991B8[p[3]].c, &off1, &a);
        func_800CE9D4(*(void **)(D_800F32D0 + 8), D_801991B8[p[3]].c, &q);
        if (p[1] < 0x21) {
            e = func_800CE610(*(void **)(D_800F33E0 + 8));
            if (e) {
                e[0] = a.vx;
                e[1] = a.vy;
                e[2] = a.vz;
                func_800CFB7C(&q, 0x32, e + 4);
                e[8] = 0;
                e[9] = 0;
            }
        } else if (p[2] > 0) {
            p[2] -= 0x10;
        }
        if (D_800E27EC == 4) {
            func_800D3F64(0x5D3, func_800D3FD8());
        }
        if (p[1] >= 0x20) {
            return 2;
        }
        break;
    case 2:
        if (p[2] >= 0) {
            func_800CE8F0(*(void **)(D_800F32D0 + 8), D_801991B8[p[3]].a, &off0, &a);
            func_800CE8F0(*(void **)(D_800F32D0 + 8), D_801991B8[p[3]].b, &off0, &b);
            func_800CE8F0(*(void **)(D_800F32D0 + 8), D_801991B8[p[3]].c, &off1, &c);
            func_800D1384(&a, &b, 0xA, &c2, &c1, p[2], p + 4, 1);
            func_800D1384(&b, &c, 0xA, &c1, &c0, p[2], p + 0x5C, 1);
        }
        {
            int value = D_800E2850[D_800E11FA];
            asm volatile("" : : : "memory");
            D_800F336C = 3;
            D_800F336E = 1;
            D_800F3372 = 0;
            D_800F3374 = 0x10;
            D_800F3370 = value;
        }
        break;
    }
    return 0;
}
