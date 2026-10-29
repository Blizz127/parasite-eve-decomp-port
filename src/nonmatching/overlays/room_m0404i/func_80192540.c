/* room_m0404i — func_80192540, blob offset 0x3558, 0xE28 bytes. NON_MATCHING
 * (behaviour C, proven by the e21 differential; not a byte match).
 * Effect-object callback a0 = mode (1 step, 2 draw), a1 = object, state at
 * +0x10 (0..4), frame at +0x12, position x/y/z at +0/+2/+4, velocity at
 * +8/+0xA/+0xC.
 * mode 1 (jump table 0x8018F1DC):
 *   0: ballistic fall with +-8 jitter on x/z and y += 0x40 until the floor
 *      D_800942EC - 0x10, then state 1 (velocity doubled, vy = 0); every odd
 *      frame spawns a state-2 spark from func_800CE610(D_800F33E0[2]);
 *      func_800C6B90(a1, 0x6E) near the floor gates the 0x3D000000 tag
 *      rewrite of the root word; returns 1 when func_8001CAB0 reports clear.
 *   1: same with +-16 jitter and a frame limit of 0x20.
 *   2: 127/128 decay, gravity -1 with a floor bounce; at frame 0x10 -> state 4.
 *   3: 511/512 decay, gravity -3 with a floor bounce; ends at frame 0xC.
 *   4: every 16th odd frame spawns a state-3 spark; ends at frame 0x2E.
 * mode 2: SetRot/SetTrans from D_800BCFA4[0], then draw by state:
 *   0/1: sprite sized sin(D_800E27EC << 9) / 4 + 0x1000, then (1 in 4) the
 *        func_800D004C burst; 2: curve colour D_80193EFC at f * 16 / 12; 3: size
 *        sin(f << 6) * 4 / 9 + 0x1000; 4: size from sin(f * 2048 / 46) with
 *        a second func_800CEE20 and a func_800D004C burst.
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
typedef struct { short m[3][3]; int t[3]; } MATRIX;

#define H(o, x) (*(unsigned short *)((char *)(o) + (x)))
#define SH(o, x) (*(short *)((char *)(o) + (x)))

extern GteShortVector D_8018F1CC;
extern B4 D_8018F1D4;
extern B4 D_8018F1D8;
extern unsigned char D_80193EFC[];
extern short D_800942EC;
extern int *D_800F33E0;
extern int *D_800F32D0;
extern int *D_8009D254;
extern unsigned char *D_800E2368;
extern int D_8009D248;
extern unsigned short D_8009D1CC;
extern unsigned short D_800E1204[];
extern int D_800E27EC;
extern int D_800F3428;
extern unsigned short D_800F3368;
extern short D_800F336A;
extern unsigned short D_800F336C;
extern unsigned short D_800F3376;
extern unsigned short D_800F3378;
extern MATRIX *D_800BCFA4[];

extern int func_80071A54();
extern int func_80077CF4();
extern int func_80077DC4();
extern int func_80077AA4();
extern unsigned char *func_800CE610();
extern int func_800C6B90();
extern int func_8001CAB0();
extern void func_800CF3AC();
extern void func_800CEE20();
extern void func_800D004C();

/* the root-word tag rewrite shared by the near-floor gates; retail loads
 * D_800F32D0 once into a base register and re-reads base[2] at each use */
#define RP() ((int **)b[2])
#define ROOT_TAG(tag)                                                      \
    do {                                                                   \
        if (D_800E2368[0xD] != 0) {                                        \
            b = D_800F32D0;                                                \
            if ((**RP() & 0x3F000000) == 0x01000000) {                     \
                ((int *)D_8009D254[0])[0x13] |= 0x4000;                    \
                o = *RP();                                                 \
                *o = (*o & 0xC0FFFFFF) | (tag);                            \
                o = *RP();                                                 \
                *o |= 0x80000000;                                          \
            }                                                              \
        }                                                                  \
    } while (0)

/* spawn a spark node: copy the position, then the per-state fields */
#define SPARK_COPY()                                                       \
    do {                                                                   \
        H(p, 0) = H(a1, 0);                                                \
        H(p, 2) = H(a1, 2);                                                \
        H(p, 4) = H(a1, 4);                                                \
    } while (0)

#define CLUT_W()                                                           \
    (cw = D_800E1204[D_800F336C],                                          \
     ((D_800F336C == 4 && D_800F3428 != 0) ? (cw += 4) : cw))

int func_80192540(int mode, unsigned char *a1)
{
    struct {
        GteShortVector v;   /* sp+0x30 */
        B4 c38;             /* sp+0x38 */
        B4 c40;             /* sp+0x40 */
        B4 c48;             /* sp+0x48 */
    } st;
    unsigned char *p;
    int *b;
    int *o;
    int cw;
    int s0;
    int s3;
    int clut;

    st.v = D_8018F1CC;
    st.c38 = D_8018F1D4;
    st.c40 = D_8018F1D8;
    if (mode == 1) {
        switch (SH(a1, 0x10)) {
        case 0:
            H(a1, 0x12)++;
            H(a1, 0) += H(a1, 8);
            H(a1, 2) += H(a1, 0xA);
            H(a1, 4) += H(a1, 0xC);
            H(a1, 0) = H(a1, 0) - 8 + (func_80071A54() & 0xF);
            H(a1, 4) = H(a1, 4) - 8 + (func_80071A54() & 0xF);
            H(a1, 2) += 0x40;
            if (!(SH(a1, 2) < D_800942EC - 0x10)) {
                SH(a1, 0x10) = 1;
                H(a1, 2) = D_800942EC - 0x10;
                SH(a1, 0xA) = 0;
                SH(a1, 0xC) = SH(a1, 0xC) * 2;
                SH(a1, 8) = SH(a1, 8) * 2;
            }
            if (H(a1, 0x12) & 1) {
                p = func_800CE610(D_800F33E0[2]);
                if (p != 0) {
                    SPARK_COPY();
                    H(p, 8) = (func_80071A54() & 0x1F) - 0x10;
                    H(p, 0xA) = 0;
                    H(p, 0xC) = (func_80071A54() & 0x1F) - 0x10;
                    H(p, 0x10) = 2;
                    H(p, 0x12) = 0;
                }
            }
            if (func_800C6B90(a1, 0x6E)) {
                if (!(SH(a1, 2) < D_800942EC - 0x202)) {
                    ROOT_TAG(0x3D000000);
                }
            }
            if (func_8001CAB0(SH(a1, 0) << 16, SH(a1, 4) << 16, D_8009D248, D_8009D1CC))
                return 0;
            return 1;
        case 1:
            H(a1, 0x12)++;
            H(a1, 0) += H(a1, 8);
            H(a1, 2) += H(a1, 0xA);
            H(a1, 4) += H(a1, 0xC);
            H(a1, 0) = H(a1, 0) - 0x10 + (func_80071A54() & 0x1F);
            H(a1, 4) = H(a1, 4) - 0x10 + (func_80071A54() & 0x1F);
            if (H(a1, 0x12) & 1) {
                p = func_800CE610(D_800F33E0[2]);
                if (p != 0) {
                    SPARK_COPY();
                    H(p, 8) = (func_80071A54() & 0x1F) - 0x10;
                    H(p, 0xA) = 0;
                    H(p, 0xC) = (func_80071A54() & 0x1F) - 0x10;
                    H(p, 0x10) = 2;
                    H(p, 0x12) = 0;
                }
            }
            if (func_800C6B90(a1, 0x6E)) {
                ROOT_TAG(0x19000000);
            }
            if (func_8001CAB0(SH(a1, 0) << 16, SH(a1, 4) << 16, D_8009D248, D_8009D1CC)) {
                return SH(a1, 0x12) >= 0x20;
            }
            return 1;
        case 2:
            H(a1, 0) += H(a1, 8);
            H(a1, 0x12)++;
            H(a1, 2) += H(a1, 0xA);
            H(a1, 4) += H(a1, 0xC);
            SH(a1, 8) = SH(a1, 8) * 127 / 128;
            SH(a1, 0xC) = SH(a1, 0xC) * 127 / 128;
            {
                short y = H(a1, 0xA) - 1;
                SH(a1, 0xA) = y;
                if (!(SH(a1, 2) < D_800942EC))
                    SH(a1, 0xA) = -y;
            }
            if (SH(a1, 0x12) < 0x10)
                return 0;
            SH(a1, 0x10) = 4;
            SH(a1, 0x12) = 0;
            H(a1, 2) = D_800942EC;
            return 0;
        case 3:
            H(a1, 0) += H(a1, 8);
            H(a1, 0x12)++;
            H(a1, 2) += H(a1, 0xA);
            H(a1, 4) += H(a1, 0xC);
            SH(a1, 8) = SH(a1, 8) * 511 / 512;
            SH(a1, 0xC) = SH(a1, 0xC) * 511 / 512;
            {
                short y = H(a1, 0xA) - 3;
                SH(a1, 0xA) = y;
                if (!(SH(a1, 2) < D_800942EC))
                    SH(a1, 0xA) = -y;
            }
            return SH(a1, 0x12) >= 0xC;
        case 4:
            H(a1, 0x12)++;
            if (func_800C6B90(a1, 0x6E)) {
                ROOT_TAG(0x19000000);
            }
            if (H(a1, 0x12) & 1) {
                if ((func_80071A54() & 0xF) == 0) {
                    p = func_800CE610(D_800F33E0[2]);
                    if (p != 0) {
                        SPARK_COPY();
                        H(p, 0) = H(p, 0) - 0x40 + (func_80071A54() & 0x7F);
                        H(p, 2) = H(p, 2) - (func_80071A54() & 0x3F);
                        H(p, 4) = H(p, 4) - 0x40 + (func_80071A54() & 0x7F);
                        H(p, 8) = (func_80071A54() & 0x1F) - 0x10;
                        H(p, 0xA) = -(func_80071A54() & 7);
                        H(p, 0xC) = (func_80071A54() & 0x1F) - 0x10;
                        H(p, 0x10) = 3;
                        H(p, 0x12) = 0;
                    }
                }
            }
            return SH(a1, 0x12) >= 0x2E;
        }
        return 0;
    }
    if (mode == 2) {
        gte_SetRotMatrix(D_800BCFA4[0]);
        gte_SetTransMatrix(D_800BCFA4[0]);
        switch (SH(a1, 0x10)) {
        case 0:
        case 1:
            s0 = func_80077CF4(D_800E27EC << 9) / 4 + 0x1000;
            s3 = (D_800E27EC & 1) * 3 * 16 + 0x80;
            clut = func_80077AA4(0x10, CLUT_W());
            func_800CEE20(a1, 0, s0, s0, 0x20, clut & 0xFFFF, 1, s3, &st.c40);
            if (func_80071A54() & 3)
                return 0;
            gte_SetRotMatrix(D_800BCFA4[0]);
            gte_SetTransMatrix(D_800BCFA4[0]);
            func_800D004C(a1, 0xC8, 0xC8, 5, 0, s0, s0, &st.c40, 0, s3, 1);
            return 0;
        case 2:
            func_800CF3AC(D_80193EFC, &st.c48, (SH(a1, 0x12) << 4) / 12);
            clut = func_80077AA4(0x10, CLUT_W());
            func_800CEE20(a1, 0, 0x1000, 0x1000, D_800F336A * (SH(a1, 0x12) / 2) + 0x20,
                          clut & 0xFFFF, 1, 0x80, &st.c48);
            return 0;
        case 3:
            s0 = func_80077CF4(SH(a1, 0x12) << 6) * 4 / 9 + 0x1000;
            clut = func_80077AA4(0, CLUT_W());
            func_800CEE20(a1, 0, s0, s0, D_800F336A * (SH(a1, 0x12) / 2),
                          clut & 0xFFFF, 2, 0x20, 0);
            return 0;
        case 4:
            s3 = func_80077CF4((SH(a1, 0x12) << 11) / 46) / 32;
            s0 = H(a1, 0x12) & 0xF;
            if (!(s0 < 8))
                s0 += 0x18;
            D_800F3376 = D_800F3368;
            D_800F3378 = D_800F3368 << 1;
            clut = func_80077AA4(0x20, CLUT_W());
            func_800CEE20(a1, 0, 0x1800, 0x1000, D_800F336A * s0 + 0x40,
                          clut & 0xFFFF, 1, s3, 0);
            D_800F3376 = D_800F3368;
            D_800F3378 = D_800F3368;
            gte_SetRotMatrix(D_800BCFA4[0]);
            gte_SetTransMatrix(D_800BCFA4[0]);
            if (func_80071A54() & 3)
                return 0;
            func_800D004C(a1, 0x140, 0x140, 6, &st.v, 0x1000, 0x1000, &st.c38, 0, s3 / 2, 1);
            return 0;
        }
        return 0;
    }
    return 0;
}
