/* room_m0318i — func_80196820, blob offset 0x7838, 0x428 bytes. NON_MATCHING
 * (behaviour C, proven by the e21 differential; not a byte match).
 * Effect callback with three modes (a0):
 *   0: set up the effect record (func_800CE870 / func_800CE9D4), clear the
 *      halfword at +8 and register func_8019646C (0x14, 0x28) via
 *      func_800CE560;
 *   1: gate on the halfword at D_800E2368->0x16 (0 -> return 1); with
 *      D_800E27EC == 2 spawn one record copied from the argument block plus
 *      four randomised ones (x/+0 jitter, +0xA += 0x400, +8 += rand); with
 *      D_800E27EC == 8 the same with the roles of +0/+2 and +8/+0xA swapped;
 *      returns 2 when D_800E27EC >= 8, otherwise 0;
 *   2: load the rotation/translation of *D_800BCFA4 into the GTE, copy the
 *      three position halfwords of the argument block to D_80199930.. and set
 *      the D_800F336C draw block.
 * inline GTE/asm: Psy-Q inline_c.h-shaped GTE (COP2) SetRot/SetTrans macros. */
#define gte_SetRotMatrix(r0) __asm__ volatile ( \
    "lw $12,0(%0)\n\tlw $13,4(%0)\n\tctc2 $12,$0\n\tctc2 $13,$1\n\t" \
    "lw $12,8(%0)\n\tlw $13,12(%0)\n\tlw $14,16(%0)\n\t" \
    "ctc2 $12,$2\n\tctc2 $13,$3\n\tctc2 $14,$4" : : "r"(r0) : "$12", "$13", "$14")
#define gte_SetTransMatrix(r0) __asm__ volatile ( \
    "lw $12,20(%0)\n\tlw $13,24(%0)\n\tctc2 $12,$5\n\tlw $14,28(%0)\n\t" \
    "ctc2 $13,$6\n\tctc2 $14,$7" : : "r"(r0) : "$12", "$13", "$14")

typedef struct { short m[3][3]; int t[3]; } MATRIX;

#define H(o, x) (*(unsigned short *)((char *)(o) + (x)))
#define SH(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))

extern int D_8009D254;
extern unsigned char *D_800F32D0;
extern unsigned char *D_800F33E0;
extern unsigned char *D_800E2368;
extern int D_800E27EC;
extern unsigned short D_800E11EA;
extern unsigned short D_800E2850[];
extern MATRIX *D_800BCFA4[];
extern unsigned short D_80199930;
extern unsigned short D_80199932;
extern unsigned short D_80199934;
extern short D_800F336C;
extern short D_800F336E;
extern unsigned short D_800F3370;
extern short D_800F3372;
extern short D_800F3374;
extern unsigned char func_8019646C[];

extern int func_80071A54();
extern int func_800CE870();
extern int func_800CE9D4();
extern int func_800CE560();
extern unsigned char *func_800CE610();

int func_80196820(int mode, unsigned char *a1)
{
    unsigned char *s0;
    int s2;
    int t;
    int r;
    unsigned short v;

    switch (mode) {
    case 0:
        func_800CE870(D_8009D254, 0, a1);
        func_800CE9D4(W(D_800F32D0, 8), 0, a1 + 8);
        SH(a1, 8) = 0;
        return func_800CE560(W(D_800F33E0, 8), 0x14, 0x28, func_8019646C);
    case 1:
        if (SH(D_800E2368, 0x16) == 0)
            return 1;
        t = D_800E27EC;
        if (t == 2) {
            s0 = func_800CE610(W(D_800F33E0, 8));
            if (s0) {
                H(s0, 0) = H(a1, 0);
                H(s0, 2) = H(a1, 2);
                H(s0, 4) = H(a1, 4);
                H(s0, 8) = H(a1, 8);
                H(s0, 0xA) = H(a1, 0xA);
                v = H(a1, 0xC);
                H(s0, 0xE) = 1;
                H(s0, 0x10) = 0;
                H(s0, 0x12) = 0;
                H(s0, 0xC) = v;
            }
            for (s2 = 0; s2 < 4; s2++) {
                s0 = func_800CE610(W(D_800F33E0, 8));
                if (s0) {
                    r = func_80071A54();
                    H(s0, 2) = 0;
                    H(s0, 0) = (r & 0x7F) - 0x40;
                    r = func_80071A54();
                    H(s0, 4) = (r & 0x1F) + 0x10;
                    H(s0, 8) = H(a1, 8);
                    H(s0, 0xA) = H(a1, 0xA);
                    v = H(a1, 0xC);
                    H(s0, 0xC) = v;
                    H(s0, 0xA) = H(s0, 0xA) + 0x400;
                    r = func_80071A54();
                    v = H(s0, 8);
                    H(s0, 0xE) = 0;
                    H(s0, 0x10) = 1;
                    H(s0, 0x12) = 0;
                    H(s0, 8) = v + r;
                }
            }
            t = D_800E27EC;
        }
        if (t == 8) {
            s0 = func_800CE610(W(D_800F33E0, 8));
            if (s0) {
                H(s0, 0) = H(a1, 0);
                H(s0, 2) = H(a1, 2);
                H(s0, 4) = H(a1, 4);
                H(s0, 8) = H(a1, 8);
                H(s0, 0xA) = H(a1, 0xA);
                v = H(a1, 0xC);
                H(s0, 0x10) = 0;
                H(s0, 0x12) = 0;
                H(s0, 8) = H(s0, 8) + 0x400;
                H(s0, 0xE) = 1;
                H(s0, 0xC) = v;
            }
            for (s2 = 0; s2 < 4; s2++) {
                s0 = func_800CE610(W(D_800F33E0, 8));
                if (s0) {
                    H(s0, 0) = 0;
                    r = func_80071A54();
                    H(s0, 2) = (r & 0x7F) - 0x40;
                    r = func_80071A54();
                    H(s0, 4) = (r & 0x1F) + 0x10;
                    H(s0, 8) = H(a1, 8);
                    H(s0, 0xA) = H(a1, 0xA);
                    v = H(a1, 0xC);
                    H(s0, 0xC) = v;
                    r = func_80071A54();
                    v = H(s0, 0xA);
                    H(s0, 0xE) = 0;
                    H(s0, 0x10) = 1;
                    H(s0, 0x12) = 0;
                    H(s0, 0xA) = v + r;
                }
            }
        }
        if (D_800E27EC < 8)
            return 0;
        return 2;
    case 2:
        gte_SetRotMatrix(D_800BCFA4[0]);
        gte_SetTransMatrix(D_800BCFA4[0]);
        D_80199930 = H(a1, 0);
        D_80199932 = H(a1, 2);
        D_80199934 = H(a1, 4);
        v = D_800E2850[D_800E11EA];
        D_800F336C = 3;
        D_800F336E = 0;
        D_800F3372 = 0;
        D_800F3374 = 0x20;
        D_800F3370 = v;
        return 0;
    }
    return 0;
}
