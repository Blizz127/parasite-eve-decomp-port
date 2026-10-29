/* room_m0432i — func_8019408C, blob offset 0x50A4, 0x7D0 bytes. NON_MATCHING
 * (behaviour C, proven by the e21 differential; not a byte match).
 * Effect-object callback (a0 = command, a1 = object):
 *  cmd 0  init: clear +8/+A/+C, register the 0x5DB sound/effect id, create
 *         the func_80193940 callback task (returns that call's result);
 *  cmd 1  update: state 0 (16 frames, sound + lift at +C), state 1 (24 frames,
 *         spawns spark objects with random velocity), state 2 (24 frames);
 *  cmd 2  draw: sets the D_800F3368 draw block and, when +C is non-zero,
 *         draws two glow quads (func_800D004C / func_800D0728) with a stack
 *         colour block chosen by D_800E27EC bit 0.
 * Read from the retail disassembly.
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
#define W(o, x) (*(unsigned int *)((char *)(o) + (x)))
#define P(o, x) (*(unsigned char **)((char *)(o) + (x)))

extern B4 D_8018F208;
extern B4 D_8018F20C;
extern unsigned char *D_800F32D0;
extern int *D_800F33E0;
extern unsigned char **D_8009D254;
extern unsigned char *D_800E2368;
extern unsigned short D_800942EC;
extern unsigned short D_800E11EA;
extern unsigned short D_800E2850[];
extern int D_800E27EC;
extern MATRIX *D_800BCFA4[];
extern short D_800F3368;
extern short D_800F336A;
extern short D_800F336C;
extern short D_800F336E;
extern short D_800F3370;
extern short D_800F3372;
extern short D_800F3374;
extern short D_800F3376;
extern short D_800F3378;

extern unsigned char func_80193940[];
extern int func_80020D50();
extern int func_80020DD0();
extern int func_80071A54();
extern int func_80077CF4();
extern int func_80077DC4();
extern int func_800D3FD8();
extern void func_800D3F64();
extern void func_800CE870();
extern int func_800CE560();
extern unsigned char *func_800CE610();
extern void func_800D004C();
extern void func_800D0728();

int func_8019408C(int mode, unsigned char *a1)
{
    /* retail keeps the two colour blocks 8 bytes apart on the stack (callees
     * read both through one pointer), so keep that layout. */
    struct { B4 b0; unsigned char gap[4]; B4 b1; } buf;
    unsigned char *s1;
    unsigned char *q;
    int t;
    int s3;
    int r;

    buf.b0 = D_8018F208;
    buf.b1 = D_8018F20C;
    switch (mode) {
    case 0:
        SH(a1, 8) = 0;
        SH(a1, 0xA) = 0;
        SH(a1, 0xC) = 0;
        t = func_800D3FD8();
        func_800D3F64(0x5DB, t);
        func_800CE870(D_8009D254, 1, a1);
        H(a1, 2) = D_800942EC;
        if (D_800E2368[0xD] != 0) {
            q = P(D_800F32D0, 8);
            if (q != 0) {
                q = P(q, 0);
                if (q != 0) {
                    q = P(q, 0x18);
                    if (*q == 1)
                        *q = 2;
                }
            }
        }
        return func_800CE560(D_800F33E0[2], 0x14, 0x28, func_80193940);
    case 1:
        switch (SH(a1, 8)) {
        case 0:
            H(a1, 0xA)++;
            func_80020D50();
            t = func_80077CF4(SH(a1, 0xA) << 6);
            SH(a1, 0xC) = t / 32;
            if (SH(a1, 0xA) < 0x10)
                return 0;
            SH(a1, 8) = 1;
            SH(a1, 0xA) = 0;
            return 0;
        case 1:
            H(a1, 0xA)++;
            if (SH(a1, 0xA) < 0x18)
                func_80020D50();
            s1 = func_800CE610(D_800F33E0[2]);
            if (s1 != 0) {
                H(s1, 0) = H(a1, 0);
                H(s1, 2) = H(a1, 2);
                H(s1, 4) = H(a1, 4);
                r = func_80071A54();
                H(s1, 8) = r % 50 - 25;
                r = func_80071A54();
                H(s1, 0xA) = -(r % 32);
                r = func_80071A54();
                H(s1, 0x10) = 1;
                H(s1, 0x12) = 0;
                H(s1, 0xC) = r % 50 - 25;
            }
            if (H(a1, 0xA) & 1) {
                s1 = func_800CE610(D_800F33E0[2]);
                if (s1 != 0) {
                    H(s1, 0) = H(a1, 0);
                    H(s1, 2) = H(a1, 2);
                    H(s1, 4) = H(a1, 4);
                    r = func_80071A54();
                    H(s1, 2) = H(s1, 2) - (r & 0x1FF);
                    r = func_80071A54();
                    H(s1, 0) = H(s1, 0) - 0x40 + (r & 0x7F);
                    r = func_80071A54();
                    H(s1, 4) = H(s1, 4) - 0x40 + (r & 0x7F);
                    r = func_80071A54();
                    H(s1, 8) = r % 24 - 12;
                    r = func_80071A54();
                    H(s1, 0xA) = r % 24 - 12;
                    r = func_80071A54();
                    H(s1, 0x10) = 0;
                    H(s1, 0x12) = 0;
                    H(s1, 0xC) = r % 24 - 12;
                }
            }
            if ((short)(SH(a1, 0xA) % 6) == 0) {
                s1 = func_800CE610(D_800F33E0[2]);
                if (s1 != 0) {
                    H(s1, 0) = H(a1, 0);
                    H(s1, 2) = H(a1, 2);
                    t = H(a1, 4);
                    H(s1, 0x10) = 2;
                    H(s1, 0x12) = 0;
                    H(s1, 4) = t;
                }
            }
            if (SH(a1, 0xA) == 0x18)
                func_80020DD0();
            if (SH(a1, 0xA) == 0x19) {
                if (D_800E2368[0xD] != 0) {
                    unsigned char *a5 = D_800F32D0;
                    if ((W(P(P(a5, 8), 0), 0) & 0x3F000000) == 0x01000000) {
                        q = *D_8009D254;
                        W(q, 0x4C) = W(q, 0x4C) | 0x4000;
                        q = P(P(a5, 8), 0);
                        W(q, 0) = (W(q, 0) & 0xC0FFFFFF) | 0x19000000;
                        q = P(P(a5, 8), 0);
                        W(q, 0) = W(q, 0) | 0x80000000;
                    }
                }
            }
            if (SH(a1, 0xA) < 0x20)
                return 0;
            SH(a1, 8) = 2;
            SH(a1, 0xA) = 0;
            return 0;
        case 2:
            H(a1, 0xA)++;
            t = func_80077DC4((SH(a1, 0xA) << 10) / 24);
            SH(a1, 0xC) = t / 32;
            if (SH(a1, 0xA) < 0x18)
                return 0;
            return 1;
        }
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
        D_800F3370 = D_800E2850[D_800E11EA];
        if (SH(a1, 0xC) != 0) {
            unsigned char *col;
            s3 = SH(a1, 0xC);
            if (D_800E27EC & 1)
                col = (unsigned char *)&buf.b0;
            else
                col = (unsigned char *)&buf.b1;
            D_800F3374 = 0x28;
            func_800D004C(a1, 0x190, 0x190, 0x10, 0, 0x1000, 0x1000, col, 0, s3, 1);
            func_800D0728(a1, 0x1F4, 0x258, 0x18, 0, 0x1000, 0x1000, 0, col, s3 / 2, 3);
        }
        D_800F3372 = 0;
        D_800F3374 = 4;
        return 0;
    }
    return 0;
}
