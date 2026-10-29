/* room_m0269i — func_801913B4, blob offset 0x23CC, 0x948 bytes. Profile era_o2_g0.
 * Particle callback registered by func_80191CFC (func_800CE560, 0x37).
 * cmd 1: state 0 flies by its velocity with a sine lift and a +0xC8 spin, lands
 * on func_800C6B90 contact (owner flag poke 0x19000000), halves its velocity
 * after 0x22 frames and dies on a func_8001CAB0 wall miss; state 1 falls
 * (y -= 2) for 0x10 frames; state 2 bounces (127/128 drag, reflect at
 * D_800942EC) for 0x20 frames.
 * cmd 2: state 0 draws two func_800CEE20 glows (trail plus floor shadow), state 1
 * one glow, state 2 an expanding then fading glow.
 * inline GTE/asm: Psy-Q inline_c.h-shaped GTE (COP2) SetRot/SetTrans macros; count
 * separately from plain C.
 * Levers: m0413i-style `>= N) return 1; break;` tails (a goto form cross-jumps the
 * func_8001CAB0 tail into state 2's); `{ int z = SH(a1,0x12) << 5; m.z = z; }` keeps
 * lh (a short expression narrows to lhu); s2/s1 doubled in place (`s2 *= 2`);
 * state-2 `s1 = n - 0x10` before the branch; q stores x, z, y; u stores x, y, z, w. */
#define gte_SetRotMatrix(r0) __asm__ volatile ( \
    "lw $12,0(%0)\n\tlw $13,4(%0)\n\tctc2 $12,$0\n\tctc2 $13,$1\n\t" \
    "lw $12,8(%0)\n\tlw $13,12(%0)\n\tlw $14,16(%0)\n\t" \
    "ctc2 $12,$2\n\tctc2 $13,$3\n\tctc2 $14,$4" : : "r"(r0) : "$12", "$13", "$14")
#define gte_SetTransMatrix(r0) __asm__ volatile ( \
    "lw $12,20(%0)\n\tlw $13,24(%0)\n\tctc2 $12,$5\n\tlw $14,28(%0)\n\t" \
    "ctc2 $13,$6\n\tctc2 $14,$7" : : "r"(r0) : "$12", "$13", "$14")

typedef struct { short x, y, z, w; } GteShortVector;
typedef struct { short m[3][3]; int t[3]; } MATRIX;

#define H(o, x) (*(unsigned short *)((char *)(o) + (x)))
#define SH(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))

extern unsigned char *D_800E2368;
extern void *D_800F32D0;
extern void **D_8009D254;
extern short D_800942EC;
extern int D_8009D248;
extern unsigned short D_8009D1CC;
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern MATRIX *D_800BCFA4[];
extern struct { short a68, a6A; unsigned short a6C; short a6E; unsigned short a70; short a72, a74, a76, a78; } D_800F3368;

extern int func_800C6B90();
extern int func_8001CAB0();
extern int func_80077CF4();
extern int func_80077DC4();
extern int func_80077AA4();
extern void func_800CEE20();

#define TEX(w) \
    w = D_800E1204[D_800F3368.a6C]; \
    if (D_800F3368.a6C == 4 && D_800F3428 != 0) \
        w += 4;

int func_801913B4(int mode, unsigned char *a1)
{
    GteShortVector m;
    GteShortVector v;
    GteShortVector q;
    GteShortVector u;
    void *o;
    int s1;
    int s2;
    int w;

    switch (mode) {
    case 1:
        switch (SH(a1, 0x10)) {
        case 0:
            H(a1, 0x12)++;
            H(a1, 0) += H(a1, 8);
            H(a1, 2) += H(a1, 0xA);
            H(a1, 4) += H(a1, 0xC);
            H(a1, 2) += func_80077CF4(SH(a1, 0x12) * 0x124) / 256;
            H(a1, 6) += 0xC8;
            v.x = H(a1, 0);
            v.y = H(a1, 2);
            v.z = H(a1, 4);
            v.x += func_80077DC4(SH(a1, 6)) / 128;
            v.z += func_80077CF4(SH(a1, 6)) / 128;
            if (func_800C6B90(&v, 100) != 0) {
                if (D_800E2368[0xD] != 0) {
                    o = D_800F32D0;
                    if ((**(int **)P(o, 8) & 0x3F000000) == 0x01000000) {
                        W(*D_8009D254, 0x4C) |= 0x4000;
                        **(int **)P(o, 8) = (**(int **)P(o, 8) & 0xC0FFFFFF) | 0x19000000;
                        **(int **)P(o, 8) |= 0x80000000;
                    }
                }
                SH(a1, 8) = 0;
                SH(a1, 0xA) = 0;
                SH(a1, 0xC) = 0;
                SH(a1, 0x10) = 1;
                SH(a1, 0x12) = 0;
            }
            if (SH(a1, 0x12) >= 0x22) {
                SH(a1, 0x10) = 1;
                SH(a1, 0x12) = 0;
                SH(a1, 8) = SH(a1, 8) / 2;
                SH(a1, 0xA) = SH(a1, 0xA) / 2;
                SH(a1, 0xC) = SH(a1, 0xC) / 2;
            }
            if (func_8001CAB0(SH(a1, 0) << 16, SH(a1, 4) << 16, D_8009D248, D_8009D1CC) == 0)
                return 1;
            break;
        case 1:
            H(a1, 0x12)++;
            H(a1, 2) -= 2;
            H(a1, 0) += H(a1, 8);
            H(a1, 2) += H(a1, 0xA);
            H(a1, 4) += H(a1, 0xC);
            if (SH(a1, 0x12) >= 0x10)
                return 1;
            break;
        case 2:
            H(a1, 0x12)++;
            H(a1, 0) += H(a1, 8);
            H(a1, 2) += H(a1, 0xA);
            H(a1, 4) += H(a1, 0xC);
            SH(a1, 8) = SH(a1, 8) * 127 / 128;
            SH(a1, 0xC) = SH(a1, 0xC) * 127 / 128;
            {
                short y = SH(a1, 0xA) - 1;
                SH(a1, 0xA) = y;
                if (SH(a1, 2) >= D_800942EC) {
                    int n = y;
                    SH(a1, 0xA) = -n;
                }
            }
            if (SH(a1, 0x12) >= 0x20)
                return 1;
            break;
        }
        break;
    case 2:
        gte_SetRotMatrix(D_800BCFA4[0]);
        gte_SetTransMatrix(D_800BCFA4[0]);
        switch (SH(a1, 0x10)) {
        case 0:
            m.x = 0;
            m.y = 0;
            {
                int z = SH(a1, 0x12) << 5;

                m.w = 0;
                m.z = z;
            }
            v.x = H(a1, 0);
            v.y = H(a1, 2);
            v.z = H(a1, 4);
            v.x += func_80077DC4(SH(a1, 6)) / 128;
            v.z += func_80077CF4(SH(a1, 6)) / 128;
            s2 = func_80077DC4(SH(a1, 0x12) * 384) / 4 + 0xC00;
            s2 *= 2;
            s1 = func_80077CF4(SH(a1, 0x12) * 384) / 4 + 0xC00;
            s1 *= 2;
            TEX(w);
            func_800CEE20(&v, &m, s2, s1, 0xA3, func_80077AA4(0x70, w) & 0xFFFF, 1, 0x80, 0);
            u.x = 0x400;
            u.y = 0;
            u.z = SH(a1, 0x12) * 80;
            u.w = 1;
            q.x = v.x;
            q.z = v.z;
            q.y = D_800942EC;
            TEX(w);
            func_800CEE20(&q, &u, s2, s1, 0xA0, func_80077AA4(0x70, w) & 0xFFFF, 2, 0x30, 0);
            break;
        case 1:
            v.x = H(a1, 0);
            v.y = H(a1, 2);
            v.z = H(a1, 4);
            v.x += func_80077DC4(SH(a1, 6)) / 128;
            v.z += func_80077CF4(SH(a1, 6)) / 128;
            TEX(w);
            func_800CEE20(&v, 0, 0x1000, 0x1000, D_800F3368.a6A * (SH(a1, 0x12) / 2) + 0x80,
                          func_80077AA4(0x70, w) & 0xFFFF, 1, 0x80, 0);
            break;
        case 2:
            s1 = SH(a1, 0x12) - 0x10;
            if (SH(a1, 0x12) < 0x11) {
                s2 = func_80077CF4(SH(a1, 0x12) << 6) / 2 + 0x800;
                TEX(w);
                func_800CEE20(a1, 0, s2, s2, 0xA3, func_80077AA4(0x70, w) & 0xFFFF, 1, 0x80, 0);
            } else {
                s1 /= 2;
                TEX(w);
                func_800CEE20(a1, 0, 0xAAA, 0xAAA, D_800F3368.a6A * s1 + 0x80,
                              func_80077AA4(0x70, w) & 0xFFFF, 1, 0x80, 0);
            }
            break;
        }
        break;
    }
end:
    return 0;
}
