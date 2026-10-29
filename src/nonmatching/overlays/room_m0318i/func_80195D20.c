/* room_m0318i — func_80195D20, blob offset 0x6D38, 0x74C bytes. NON_MATCHING
 * behaviour C written from the retail asm (2026-10-18; not a byte match).
 * Effect task, a0 = mode, a1 = parameter block (x y z halfwords at +0/+2/+4).
 *   mode 0: func_800CE870(D_8009D254, 0, a1, buf), then registers the
 *           func_80195904 particle callback with func_800CE560 (returned).
 *   mode 1: per-frame step on the D_800E27EC frame counter. Returns 1 when
 *           the D_800E2368 record's +0x16 halfword is zero. Frame 2 plays two
 *           sound effects (0x5E7 / 0x5E8) through func_8006DF50 when the
 *           D_800B0E64 voice exists. Frame 3 spawns three particles (kind 1,
 *           velocity from three rand()%50-25), frames < 5 spawn two more
 *           (kind 0, spread rand()&0x7F, velocity rand()%190-95) through the
 *           func_800CE610 pool at D_800F33E0[2]. Returns 2 from frame 8 on.
 *   mode 2: draw. Sets the D_800F3368 block, loads the camera GTE
 *           matrix (D_800BCFA4[0]), sets the screen flash (func_800D1AE0 on
 *           frames 1 and 2), and for frame <= 32 draws three sprites
 *           (func_800CEE20 / func_800D004C / func_800D0728) sized by the sin
 *           of frame << 5. Always ends by resetting the draw block; returns 0.
 * inline GTE/asm: Psy-Q inline_c.h-shaped GTE (COP2) SetRot/SetTrans macros. */
#define gte_SetRotMatrix(r0) __asm__ volatile ( \
    "lw $12,0(%0)\n\tlw $13,4(%0)\n\tctc2 $12,$0\n\tctc2 $13,$1\n\t" \
    "lw $12,8(%0)\n\tlw $13,12(%0)\n\tlw $14,16(%0)\n\t" \
    "ctc2 $12,$2\n\tctc2 $13,$3\n\tctc2 $14,$4" : : "r"(r0) : "$12", "$13", "$14")
#define gte_SetTransMatrix(r0) __asm__ volatile ( \
    "lw $12,20(%0)\n\tlw $13,24(%0)\n\tctc2 $12,$5\n\tlw $14,28(%0)\n\t" \
    "ctc2 $13,$6\n\tctc2 $14,$7" : : "r"(r0) : "$12", "$13", "$14")

typedef struct { unsigned char c[4]; } B4;
typedef struct { short m[3][3]; int t[3]; } MATRIX;

#define H(o, x) (*(unsigned short *)((char *)(o) + (x)))
#define SH(o, x) (*(short *)((char *)(o) + (x)))

extern B4 D_8018F210;
extern unsigned char D_80199770[];
extern int D_8009D254;
extern int D_800B0E64;
extern unsigned char *D_800E2368;
extern int D_800E27EC;
extern unsigned short D_800E11EA;
extern unsigned short D_800E11FA;
extern unsigned short D_800E1204[];
extern unsigned short D_800E2850[];
extern int *D_800F33E0;
extern int D_800F3428;
extern MATRIX *D_800BCFA4[];
extern short D_800F3368;
extern short D_800F336A;
extern unsigned short D_800F336C;
extern short D_800F336E;
extern unsigned short D_800F3370;
extern short D_800F3372;
extern short D_800F3374;
extern short D_800F3376;
extern short D_800F3378;

extern unsigned char func_80195904[];
extern int func_80071A54();
extern int func_80077AA4();
extern int func_80077CF4();
extern int func_80077DC4();
extern int func_8006DF50();
extern int func_800CE560();
extern int func_800CE610();
extern int func_800CE870();
extern int func_800CEE20();
extern int func_800CF3AC();
extern int func_800D004C();
extern int func_800D0728();
extern int func_800D1AE0();
extern int func_800D3FD8();

int func_80195D20(int mode, unsigned char *a1)
{
    unsigned char col[8];
    B4 buf;
    unsigned char *p;
    int i;
    int r;
    int f;
    int s1;
    int s2;
    int t;
    int w;
    int v;

    buf = D_8018F210;
    switch (mode) {
    case 0:
        func_800CE870(D_8009D254, 0, a1);
        return func_800CE560(D_800F33E0[2], 0x14, 0x10, func_80195904);
    case 1:
        if (SH(D_800E2368, 0x16) == 0)
            return 1;
        if (D_800E27EC == 2 && D_800B0E64 != 0) {
            r = func_800D3FD8();
            func_8006DF50(D_800B0E64, 0x5E7, r, 0x80, 0x7F);
            if (D_800B0E64 != 0)
                func_8006DF50(D_800B0E64, 0x5E8, 0x80, 0x80, 0x7F);
        }
        if (D_800E27EC == 3) {
            for (i = 0; i < 3; i++) {
                p = (unsigned char *)func_800CE610(D_800F33E0[2]);
                if (p) {
                    H(p, 0) = H(a1, 0);
                    H(p, 2) = H(a1, 2);
                    H(p, 4) = H(a1, 4);
                    r = func_80071A54();
                    SH(p, 8) = r % 50 - 0x19;
                    r = func_80071A54();
                    SH(p, 0xA) = r % 50 - 0x19;
                    r = func_80071A54();
                    SH(p, 0x10) = 1;
                    SH(p, 0x12) = 0;
                    SH(p, 0xC) = r % 50 - 0x19;
                }
            }
        }
        if (D_800E27EC < 5) {
            for (i = 0; i < 2; i++) {
                p = (unsigned char *)func_800CE610(D_800F33E0[2]);
                if (p) {
                    H(p, 0) = H(a1, 0);
                    H(p, 2) = H(a1, 2);
                    H(p, 4) = H(a1, 4);
                    SH(p, 6) = (func_80071A54() & 0x7FF) + 0x800;
                    H(p, 0) = H(p, 0) - 0x40 + (func_80071A54() & 0x7F);
                    H(p, 2) = H(p, 2) - 0x40 + (func_80071A54() & 0x7F);
                    H(p, 4) = H(p, 4) - 0x40 + (func_80071A54() & 0x7F);
                    r = func_80071A54();
                    SH(p, 8) = r % 190 - 0x5F;
                    r = func_80071A54();
                    SH(p, 0xA) = r % 190 - 0x5F;
                    r = func_80071A54();
                    SH(p, 0x10) = 0;
                    SH(p, 0x12) = 0;
                    SH(p, 0xC) = r % 190 - 0x5F;
                }
            }
        }
        if (D_800E27EC >= 8)
            return 2;
        return 0;
    case 2:
        gte_SetRotMatrix(D_800BCFA4[0]);
        gte_SetTransMatrix(D_800BCFA4[0]);
        D_800F3368 = 0x20;
        D_800F336A = 2;
        D_800F3376 = 0x20;
        D_800F3378 = 0x20;
        D_800F336C = 3;
        D_800F336E = 0;
        D_800F3372 = 0;
        D_800F3374 = 0x20;
        D_800F3370 = D_800E2850[D_800E11EA];
        if (D_800E27EC == 1)
            func_800D1AE0(&buf, 0x80, 2, 8);
        else if (D_800E27EC == mode)
            func_800D1AE0(&buf, 0x80, 3, 8);
        if (D_800E27EC <= 0x20) {
            s2 = D_800E27EC << 5;
            s1 = func_80077DC4(s2) / 32;
            w = D_800E1204[D_800F336C];
            if (D_800F336C == 4 && D_800F3428 != 0)
                w += 4;
            t = func_80077AA4(0x30, w);
            func_800CEE20(a1, 0, 0x2000, 0x2000, D_800F336A * (D_800E27EC / 4) + 0xA0,
                          t & 0xFFFF, 1, s1, 0);
            func_800CF3AC(D_80199770, col, D_800E27EC);
            t = func_80077CF4(s2) + 0x1000;
            if (D_800E27EC & 1)
                s1 = s1 * 2 / 3;
            func_800D004C(a1, 0x1F4, 0x1F4, 0xA, 0, t, t, col, 0, s1, 1);
            v = func_80077DC4(s2);
            s1 = v / 32;
            t = func_80077CF4(s2) + 0x800;
            func_800D0728(a1, 0x44C, 0x6A4, 0x10, 0, t, t, 0, col, s1 / 2, 1);
        }
        D_800F336C = 3;
        D_800F336E = 1;
        D_800F3372 = 0;
        D_800F3374 = 0x10;
        D_800F3370 = D_800E2850[D_800E11FA];
        return 0;
    }
    return 0;
}
