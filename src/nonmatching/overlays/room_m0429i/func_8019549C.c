/* room_m0429i — func_8019549C, blob offset 0x64B4, 0xC28 bytes. NON_MATCHING
 * (behaviour C, proven by the e21 differential; not a byte match).
 * Effect-object callback (registered by func_801960C4 through func_800CE560;
 * same family as func_801982A4 / room_m0269i func_8019070C).  Seeds sp+0x30
 * from the 8 text bytes at func_8018F210 and sp+0x48 from D_8018F218.
 * state at a1+0x1C, frame counter at a1+0x1E, position at a1+0/2/4,
 * velocity at a1+8/0xA/0xC.
 * cmd 1 state 0: spawn, seeds position from func_800CE8F0 / func_800CE9D4 and
 *   func_800CFE94 (height test), random jitter into a1+8/0xA/0xC.
 * cmd 1 state 1: integrate (func_800CFAA8/CFD50/CFB7C), clamp speed a1+0x18
 *   and a1+0x1A, move, floor test D_800942EC; state 2: timed life (0x18 ticks)
 *   with the same gate-flag block; state 3: 12 ticks.
 * cmd 2 state 1: three func_800CEE20 sprites + func_800D004C ring, flash
 *   func_800D3BC8 pair on odd a1+6, func_800D0E88 trail; state 2: grow/shrink
 *   glow (func_80077DC4 envelope) with a func_800CEE20 pair.
 * inline GTE/asm: Psy-Q inline_c.h-shaped GTE (COP2) SetRot/SetTrans macros. */
#define gte_SetRotMatrix(r0) __asm__ volatile ( \
    "lw $12,0(%0)\n\tlw $13,4(%0)\n\tctc2 $12,$0\n\tctc2 $13,$1\n\t" \
    "lw $12,8(%0)\n\tlw $13,12(%0)\n\tlw $14,16(%0)\n\t" \
    "ctc2 $12,$2\n\tctc2 $13,$3\n\tctc2 $14,$4" : : "r"(r0) : "$12", "$13", "$14")
#define gte_SetTransMatrix(r0) __asm__ volatile ( \
    "lw $12,20(%0)\n\tlw $13,24(%0)\n\tctc2 $12,$5\n\tlw $14,28(%0)\n\t" \
    "ctc2 $13,$6\n\tctc2 $14,$7" : : "r"(r0) : "$12", "$13", "$14")

typedef struct { short x, y, z, w; } GteShortVector;
typedef struct { unsigned char c[4]; } B4;
typedef struct { unsigned char c[8]; } B8;
typedef struct { short m[3][3]; int t[3]; } MATRIX;

#define H(o, x) (*(unsigned short *)((char *)(o) + (x)))
#define SH(o, x) (*(short *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))

extern unsigned char func_8018F210[];
extern B4 D_8018F218;
extern short D_800942EC;
extern unsigned short D_800E11EA;
extern unsigned short D_800E120A;
extern unsigned short D_800E2850[];
extern unsigned short D_800E1204[];
extern unsigned char *D_800E2368;
extern void *D_800F32D0;
extern void *D_8009D254;
extern int D_800F3428;
extern MATRIX *D_800BCFA4[];
extern struct { short a68, a6A; unsigned short a6C; short a6E; unsigned short a70; short a72, a74, a76, a78; } D_800F3368;

extern int func_80077CF4();
extern int func_80077DC4();
extern int func_80077AA4();
extern int func_80071A54();
extern int func_800C6B90();
extern void func_800CE8F0();
extern void func_800CE9D4();
extern void func_800CE870();
extern int func_800CFE94();
extern void func_800CFAA8();
extern void func_800CFD50();
extern void func_800CFB7C(void *, short, void *);
extern void func_800CEE20();
extern void func_800D004C();
extern void func_800D3BC8();
extern void func_800D0E88();

/* the gate-flag block shared by cmd 1 states 1 and 2 */
#define GATE_FLAGS() do { \
    unsigned int **gp_; \
    unsigned int *gq_; \
    void *gb_ = D_800F32D0; \
    if (D_800E2368[0xD] != 0) { \
        gp_ = (unsigned int **)P(gb_, 8); \
        if ((**gp_ & 0x3F000000) == 0x01000000) { \
            gq_ = *(unsigned int **)D_8009D254; \
            gq_[0x4C / 4] |= 0x4000; \
            gp_ = (unsigned int **)P(gb_, 8); \
            gq_ = *gp_; \
            *gq_ = (*gq_ & 0xC0FFFFFF) | 0x29000000; \
            gp_ = (unsigned int **)P(gb_, 8); \
            gq_ = *gp_; \
            *gq_ |= 0x80000000; \
        } \
    } \
} while (0)

#define PAL_W() (D_800E1204[D_800F3368.a6C] + ((D_800F3368.a6C == 4 && D_800F3428 != 0) ? 4 : 0))

int func_8019549C(int mode, unsigned char *a1)
{
    B8 b30;             /* frame order = retail sp+0x30, 0x38, 0x40, 0x48, 0x50, 0x58 */
    GteShortVector v38;
    GteShortVector v40;
    struct { B4 b; B4 hole; } b48;   /* retail leaves sp+0x4C unwritten */
    GteShortVector v50;
    GteShortVector v58;
    int s0, s2, t, w, clut, sq;
    short y;
    unsigned short n;
    short a2v;

    b30 = *(B8 *)func_8018F210;
    b48.b = D_8018F218;
    switch (mode) {
    case 1:
        switch (SH(a1, 0x1C)) {
        case 0:
            SH(a1, 0x1E) = 0;
            SH(a1, 0x1C) = 1;
            SH(a1, 0xE) = 0;
            func_800CE8F0(P(D_800F32D0, 8), 0, &b30, a1);
            func_800CE9D4(P(D_800F32D0, 8), 0, &v40);
            v40.x = 0;
            func_800CE870(D_8009D254, 1, a1 + 0x10);
            func_800CE870(D_8009D254, 0, &v38);
            s0 = (func_800CFE94(a1, &v38) < 0x76C) ? 0x1F4 : 0;
            t = func_80071A54();
            a2v = (unsigned short)v40.x + ((t & 0x3FF) - (s0 + 0x186));
            v40.x = a2v;
            v40.y = (unsigned short)v40.y - 0x200 + ((SH(a1, 6) % 5) << 8);
            SH(a1, 8) = a2v;
            SH(a1, 0xA) = v40.y;
            SH(a1, 0x18) = 0x18;
            SH(a1, 0x1A) = 0;
            SH(a1, 0xC) = v40.z;
            return 0;
        case 1:
            n = H(a1, 0x1E) + 1;
            H(a1, 0x1E) = n;
            func_800CFAA8(a1, a1 + 0x10, &v40);
            t = func_80077CF4(SH(a1, 0x1A));
            func_800CFD50(&v40, a1 + 8, (unsigned short)(t / 5));
            y = SH(a1, 0x18);
            if (y < 0x3E)
                SH(a1, 0x18) = y + 4;
            if (SH(a1, 0x18) >= 0x28) {
                y = SH(a1, 0x1A);
                if (y < 0x800)
                    SH(a1, 0x1A) = y + 0x66;
                if (SH(a1, 0x1A) >= 0x801)
                    SH(a1, 0x1A) = 0x800;
            }
            sq = SH(a1, 0x18) * SH(a1, 0x18);
            func_800CFB7C(a1 + 8, (int)((unsigned)sq << 12) >> 16, &v38);
            H(a1, 0) = H(a1, 0) + (unsigned short)v38.x;
            H(a1, 2) = H(a1, 2) + (unsigned short)v38.y;
            H(a1, 4) = H(a1, 4) + (unsigned short)v38.z;
            if (func_800C6B90(a1, 0xBE) != 0 && D_800942EC - 0x202 < SH(a1, 2)) {
                GATE_FLAGS();
                SH(a1, 0x1C) = 2;
                SH(a1, 0x1E) = 0;
            }
            if (SH(a1, 2) >= D_800942EC) {
                SH(a1, 2) = D_800942EC;
                SH(a1, 0x1C) = 2;
                SH(a1, 0x1E) = 0;
            }
            return 0;
        case 2:
            n = H(a1, 0x1E) + 1;
            H(a1, 0x1E) = n;
            if (func_800C6B90(a1, 0xC8) != 0)
                GATE_FLAGS();
            return SH(a1, 0x1E) < 0x18 ? 0 : 1;
        case 3:
            n = H(a1, 0x1E) + 1;
            H(a1, 0x1E) = n;
            return (short)n < 0xC ? 0 : 1;
        }
        return 0;
    case 2:
        switch (SH(a1, 0x1C)) {
        case 1:
            s2 = (H(a1, 0x1E) & 1) ? 0x5C : 0x80;
            v40.x = 0;
            v40.y = 0;
            D_800F3368.a68 = 0x40;
            D_800F3368.a76 = 0x40;
            D_800F3368.a78 = 0x40;
            v40.w = 0;
            D_800F3368.a6A = 4;
            v40.z = SH(a1, 0x1E) << 7;
            clut = func_80077AA4(0x10, PAL_W());
            func_800CEE20(a1, &v40, 0x1800, 0x1800, 0x44, clut & 0xFFFF, 1, s2 / 2, 0);
            D_800F3368.a6A = 2;
            y = v40.z;
            D_800F3368.a68 = 0x20;
            D_800F3368.a76 = 0x20;
            D_800F3368.a78 = 0x20;
            v40.z = -y;
            clut = func_80077AA4(0x10, PAL_W());
            func_800CEE20(a1, &v40, 0x2000, 0x2000, 0x98, clut & 0xFFFF, 1, s2, 0);
            gte_SetRotMatrix(D_800BCFA4[0]);
            gte_SetTransMatrix(D_800BCFA4[0]);
            func_800D004C(a1, 0x258, 0x258, 5, 0, 0x1000, 0x1000, &b48, 0, 0x46, 1);
            if (SH(a1, 0x1E) < 0x18 && (H(a1, 6) & 1) != 0) {
                s2 = func_80077DC4((SH(a1, 0x1E) << 10) / 24);
                if (s2 < 0)
                    s2 += 0x3F;
                s2 >>= 6;
                v38.x = H(a1, 0);
                v38.y = H(a1, 2);
                v38.z = H(a1, 4);
                D_800F3368.a6A = 4;
                D_800F3368.a68 = 0x40;
                D_800F3368.a76 = 0x40;
                D_800F3368.a78 = 0x40;
                clut = func_80077AA4(0xA0, PAL_W());
                func_800D3BC8(&v38, 0x1800, 0x1800, 0xC, clut & 0xFFFF, 1, s2 / 2, 0, 0xC00);
                D_800F3368.a68 = 0x20;
                D_800F3368.a6A = 2;
                D_800F3368.a76 = 0x20;
                D_800F3368.a78 = 0x20;
                clut = func_80077AA4(0xB0, PAL_W());
                func_800D3BC8(&v38, 0x1800, 0x1800, 0x9A, clut & 0xFFFF, 1, s2 / 2, 0, 0x600);
            }
            gte_SetRotMatrix(D_800BCFA4[0]);
            gte_SetTransMatrix(D_800BCFA4[0]);
            w = SH(a1, 0x1E);
            if (w >= 0xD)
                return 0;
            if ((H(a1, 6) & 1) != 0)
                return 0;
            func_800D0E88(a1, a1 + 8, 0x76C, (0xC - w) << 8, &b48, 0, 0, 0x3E, 1);
            return 0;
        case 2:
            D_800F3368.a68 = 0x40;
            D_800F3368.a6A = 4;
            D_800F3368.a76 = 0x40;
            D_800F3368.a78 = 0x40;
            s2 = func_80077DC4((SH(a1, 0x1E) << 10) / 24);
            if (s2 < 0)
                s2 += 0x1F;
            s2 >>= 5;
            s0 = func_80077DC4((SH(a1, 0x1E) << 10) / 24) * 2;
            if (H(a1, 0x1E) & 1)
                s2 = (s2 * 2) / 3;
            clut = func_80077AA4(0x10, PAL_W());
            func_800CEE20(a1, 0, s0, s0, 8, clut & 0xFFFF, 1, s2, 0);
            v58.x = 0x400;
            v58.y = 0;
            v58.w = 1;
            v58.z = SH(a1, 0x1E) << 5;
            v50.x = H(a1, 0);
            v50.y = D_800942EC;
            v50.z = H(a1, 4);
            clut = func_80077AA4(0x10, PAL_W());
            func_800CEE20(&v50, &v58, 0x2000 - s0, 0x2000 - s0, 0x40, clut & 0xFFFF, 1, s2 / 2, 0);
            D_800F3368.a68 = 0x20;
            D_800F3368.a6A = 2;
            D_800F3368.a76 = 0x20;
            D_800F3368.a78 = 0x20;
            return 0;
        }
        return 0;
    }
    return 0;
}
