/* room_m0318i — func_801955E4, blob offset 0x65FC, 0x320 bytes. NON_MATCHING
 * (behaviour C, proven by the e21 differential; not a byte match).
 * Effect callback, a0 = mode, a1 = effect record, a2 = parameter block.
 *   mode 0: clear a1 +8/+0xA, seed the sprite pool (func_800D3F64(0x5EA, ...)),
 *           register func_80195190 with the pool at D_800F33E0->+8; returns
 *           that call's result (v0 is not cleared on this path).
 *   mode 1: advance frame a1 +0xA, call func_800CE870, move a1 +2 by a ramp
 *           of a2 (+8 * frame / +4), spawn trail particles every 4th frame
 *           (while frame <= a2 +4), returns 1 once frame >= a2 +4 + 8.
 *   mode 2: load the rotation/translation matrix D_800BCFA4[0] into the GTE
 *           and latch the a1 position into D_80199924/26/28.
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

extern unsigned short D_800942EC;
extern unsigned char *D_800F32D0;
extern unsigned char *D_800F33E0;
extern short D_800F3372;
extern short D_800F3374;
extern unsigned short D_80199924;
extern unsigned short D_80199926;
extern unsigned short D_80199928;
extern MATRIX *D_800BCFA4[];
extern unsigned char func_80195190[];

extern int func_80071A54();
extern int func_800CE560();
extern unsigned char *func_800CE610();
extern void func_800CE870();
extern void func_800D3F64();
extern int func_800D3FD8();

int func_801955E4(int mode, unsigned char *a1, unsigned char *a2)
{
    unsigned char *p;
    int a, b, lo, n, v;

    switch (mode) {
    case 0:
        SH(a1, 8) = 0;
        SH(a1, 0xA) = 0;
        func_800D3F64(0x5EA, func_800D3FD8());
        return func_800CE560(W(D_800F33E0, 8), 0x14, 0x20, func_80195190);
    case 1:
        H(a1, 0xA) = H(a1, 0xA) + 1;
        func_800CE870(W(D_800F32D0, 8), 0, a1);
        a = D_800942EC - W(a2, 0xC);
        SH(a1, 2) = a;
        b = W(a2, 8);
        lo = (b * SH(a1, 0xA)) / W(a2, 4);
        if (W(a2, 0) == 0)
            SH(a1, 2) = a - (b - lo);
        else
            SH(a1, 2) = a - lo;
        n = SH(a1, 0xA);
        if (W(a2, 4) >= n) {
            if ((n & 3) == 0) {
                p = func_800CE610(W(D_800F33E0, 8));
                if (p != 0) {
                    SH(p, 0x10) = 0;
                    SH(p, 0x12) = 0;
                }
            }
            p = func_800CE610(W(D_800F33E0, 8));
            if (p != 0) {
                H(p, 0) = H(a1, 0);
                H(p, 2) = H(a1, 2);
                H(p, 4) = H(a1, 4);
                v = func_80071A54() & 0xFF;
                H(p, 0) = H(p, 0) - 0x80 + v;
                v = func_80071A54() & 0xFF;
                H(p, 4) = H(p, 4) - 0x80 + v;
                v = func_80071A54();
                SH(p, 8) = (v % 32) - 0x10;
                func_80071A54();
                SH(p, 0xA) = 0;
                v = func_80071A54();
                SH(p, 0xC) = (v % 32) - 0x10;
                SH(p, 0x10) = 1;
                SH(p, 0x12) = 0;
            }
        }
        if (SH(a1, 0xA) < W(a2, 4) + 8)
            return 0;
        return 1;
    case 2: {
        MATRIX *m = D_800BCFA4[0];
        gte_SetRotMatrix(m);
        gte_SetTransMatrix(m);
        D_80199924 = H(a1, 0);
        D_80199926 = H(a1, 2);
        D_800F3372 = 0;
        D_800F3374 = 4;
        D_80199928 = H(a1, 4);
        return 0;
    }
    default:
        return 0;
    }
}
