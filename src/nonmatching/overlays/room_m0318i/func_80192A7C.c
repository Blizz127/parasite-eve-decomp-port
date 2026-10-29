/* room_m0318i — func_80192A7C, blob offset 0x3A94, 0x324 bytes. NON_MATCHING
 * (behaviour C, proven by the e21 differential; not a byte match).
 * Effect callback, a0 = mode, a1 = effect record, a2 = parameter block.
 *   mode 0: a1 +8 = func_80071A54() (RNG); walk the D_8009D20C list (stop at
 *           the D_8009D254 sentinel / null) for a node whose +0 pointer has
 *           a positive +0x10 and whose bytes +0xD/+0xC equal a2 +0 / +4, then
 *           func_800CE870(node, 0, a1); else printf D_8018F1CC. Seeds a1 +2,
 *           the 0x5F3/0x5F4 params, registers func_80192718 (pool at
 *           D_800F33E0->+8) and func_80192620 (a1 +0xC); returns the sum of
 *           both call results.
 *   mode 1: while D_800E27EC < 0x20 and odd, spawn a particle (+0xC=0xC8,
 *           +2 = a1 +8) and drop a1 +8 by 0x555; returns 0 and latches
 *           a1 +0xC into D_80199900 while D_800E27EC < 0x46, else 1.
 *   mode 2: load the D_800BCFA4[0] rotation/translation matrix into the GTE,
 *           set up the D_800F3368.. draw-state shorts, latch a1 position
 *           into D_801998F8/FA/FC; returns 0.
 * a3 is not an input: retail never sets $a3 before the not-found printf, so the
 * 4th printf argument is whatever $a3 holds after func_80071A54 returns (callee
 * scratch, TTY text only). The C passes only the three defined arguments.
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

extern unsigned char *D_8009D20C;
extern unsigned char *D_8009D254;
extern unsigned short D_800942EC;
extern unsigned char *D_800F33E0;
extern int D_800E27EC;
extern unsigned short D_800E11FA;
extern unsigned short D_800E2850[];
extern MATRIX *D_800BCFA4[];
extern int D_80199900;
extern unsigned short D_801998F8;
extern unsigned short D_801998FA;
extern unsigned short D_801998FC;
extern short D_800F3368;
extern short D_800F336A;
extern short D_800F336C;
extern short D_800F336E;
extern short D_800F3370;
extern short D_800F3372;
extern short D_800F3374;
extern short D_800F3376;
extern short D_800F3378;
extern char D_8018F1CC[];
extern unsigned char func_80192718[];
extern unsigned char func_80192620[];

extern int func_80071A54();
extern int func_80071A74();
extern int func_800CE560();
extern int func_800CE5AC();
extern unsigned char *func_800CE610();
extern void func_800CE688();
extern void func_800CE78C();
extern void func_800CE870();
extern void func_800D3F64();
extern int func_800D3FD8();

int func_80192A7C(int mode, unsigned char *a1, unsigned char *a2)
{
    unsigned char *p, *sentinel;
    int key0, key1, r, v;

    switch (mode) {
    case 0:
        W(a1, 8) = func_80071A54();
        p = D_8009D20C;
        key0 = W(a2, 0);
        key1 = W(a2, 4);
        if (p != 0) {
            sentinel = D_8009D254;
            do {
                if (p != sentinel && W(p, 0) != 0 && W(*(unsigned char **)p, 0x10) > 0
                    && p[0xD] == key0 && p[0xC] == key1) {
                    func_800CE870(p, 0, a1);
                    goto found;
                }
                p = *(unsigned char **)(p + 4);
            } while (p != 0);
        }
        func_80071A74(D_8018F1CC, key0, key1);
    found:
        H(a1, 2) = D_800942EC;
        func_800D3F64(0x5F3, func_800D3FD8());
        func_800D3F64(0x5F4, 0x80);
        r = func_800CE560(W(D_800F33E0, 8), 0x10, 0x18, func_80192718);
        v = func_800CE5AC(a1 + 0xC, r, 8, 0x12, func_80192620);
        return r + v;
    case 1:
        if (D_800E27EC < 0x20 && (D_800E27EC & 1)) {
            p = func_800CE610(W(D_800F33E0, 8));
            if (p != 0) {
                v = W(a1, 8);
                SH(p, 0xC) = 0xC8;
                SH(p, 0) = 0;
                SH(p, 4) = 0;
                SH(p, 2) = v;
                W(a1, 8) = W(a1, 8) - 0x555;
            }
        }
        if (D_800E27EC >= 0x46)
            return 1;
        func_800CE688(W(a1, 0xC));
        D_80199900 = W(a1, 0xC);
        return 0;
    case 2: {
        MATRIX *m = D_800BCFA4[0];
        gte_SetRotMatrix(m);
        gte_SetTransMatrix(m);
        D_800F3374 = 8;
        func_800CE78C(W(a1, 0xC));
        D_801998F8 = H(a1, 0);
        D_801998FA = H(a1, 2);
        D_800F3368 = 0x20;
        D_800F336A = 2;
        D_800F3376 = 0x20;
        D_800F3378 = 0x20;
        D_801998FC = H(a1, 4);
        v = D_800E2850[D_800E11FA];
        D_800F336C = 3;
        D_800F336E = 1;
        D_800F3372 = 0;
        D_800F3370 = v;
        return 0;
    }
    default:
        return 0;
    }
}
