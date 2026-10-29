/* room_m0318i — func_80197CBC, blob offset 0x8CD4, 0x5AC bytes. NON_MATCHING
 * (behaviour C, proven by the e21 differential; not a byte match).
 * Effect callback, a0 = mode, a1 = effect record (frame counter at +0xA).
 *   mode 0: clear a1 +8/+0xA/+0xC/+0xE, func_800D3F64(0x5EB,
 *           func_800D3FD8()), func_800D3F64(0x5EC, 0x80), then
 *           return func_800CE560(D_800F33E0->+8, 0x14, 0x28, func_80197618).
 *   mode 1: a1 +0xA++, func_800CE870(D_800F32D0->+8, 0, a1); position copy
 *           sp+0x30 = a1 +0/+2/+4; angle = (frame << 10) / 141:
 *           a1 +0xC = cos/32, a1 +0xE = sin; while frame < 0x86 spawn up to
 *           three particles in the D_800F33E0->+8 pool (every frame a
 *           +-0x20 burst; odd frames a +-0x100 / +-16 spray, state 0; every
 *           fourth frame a +-0x100 ring, state 2); D_8019993C.. = a1 +0..+4.
 *           Returns 1 from frame 0x8D on.
 *   mode 2: load the D_800BCFA4 rotation/translation into the GTE; while
 *           frame < 0x8E draw func_800D004C (0x3E8 ring, +0xC / 0.75 on odd
 *           D_800E27EC) and func_800D0728 (0x578 / 0x708 ring) around the
 *           position; then D_800F3372 = 0, D_800F3374 = 4. Returns 0.
 * a0 outside 0..2 returns 0. The buf seed (lwl/lwr of D_8018F200) is passed
 * by address to the ring draws.
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
#define W(o, x) (*(int *)((char *)(o) + (x)))

extern B4 D_8018F200;
extern unsigned char *D_800F33E0;
extern unsigned char *D_800F32D0;
extern MATRIX *D_800BCFA4[];
extern int D_800E27EC;
extern short D_800F3372;
extern short D_800F3374;
extern unsigned short D_8019993C;
extern unsigned short D_8019993E;
extern unsigned short D_80199940;
extern unsigned char func_80197618[];

extern int func_800D3FD8();
extern void func_800D3F64();
extern int func_800CE560();
extern void func_800CE870();
extern unsigned char *func_800CE610();
extern int func_80077CF4();
extern int func_80077DC4();
extern int func_80071A54();
extern void func_800D004C();
extern void func_800D0728();

int func_80197CBC(int mode, unsigned char *a1)
{
    /* retail frame: position at sp+0x30 (3 shorts), colour seed at sp+0x38 */
    struct { unsigned short pos[4]; B4 buf; } f;
    unsigned char *s;
    int t, c, a;

    f.buf = D_8018F200;
    switch (mode) {
    case 0:
        SH(a1, 8) = 0;
        SH(a1, 0xA) = 0;
        SH(a1, 0xC) = 0;
        SH(a1, 0xE) = 0;
        t = func_800D3FD8();
        func_800D3F64(0x5EB, t);
        func_800D3F64(0x5EC, 0x80);
        return func_800CE560(W(D_800F33E0, 8), 0x14, 0x28, func_80197618);
    case 1:
        H(a1, 0xA) = H(a1, 0xA) + 1;
        func_800CE870(W(D_800F32D0, 8), 0, a1);
        f.pos[0] = H(a1, 0);
        f.pos[1] = H(a1, 2);
        f.pos[2] = H(a1, 4);
        a = (SH(a1, 0xA) << 10) / 141;
        t = func_80077CF4(a);
        SH(a1, 0xC) = t / 32;
        SH(a1, 0xE) = func_80077DC4(a);
        if (SH(a1, 0xA) < 0x86) {
            s = func_800CE610(W(D_800F33E0, 8));
            if (s != 0) {
                H(s, 0) = f.pos[0];
                H(s, 2) = f.pos[1];
                H(s, 4) = f.pos[2];
                H(s, 0) = H(s, 0) - 0x20 + (func_80071A54() & 0x3F);
                H(s, 2) = H(s, 2) - 0x20 + (func_80071A54() & 0x3F);
                H(s, 4) = H(s, 4) - 0x20 + (func_80071A54() & 0x3F);
                SH(s, 6) = (func_80071A54() & 0x3FF) + 0x3E8;
                SH(s, 8) = func_80071A54();
                SH(s, 0xA) = func_80071A54();
                SH(s, 0xC) = func_80071A54();
                SH(s, 0x10) = 1;
                SH(s, 0x12) = 0;
            }
            if (H(a1, 0xA) & 1) {
                s = func_800CE610(W(D_800F33E0, 8));
                if (s != 0) {
                    H(s, 0) = f.pos[0];
                    H(s, 2) = f.pos[1];
                    H(s, 4) = f.pos[2];
                    H(s, 2) = H(s, 2) + 0x100 - (func_80071A54() & 0x1FF);
                    H(s, 0) = H(s, 0) - 0x100 + (func_80071A54() & 0x1FF);
                    H(s, 4) = H(s, 4) - 0x100 + (func_80071A54() & 0x1FF);
                    c = func_80071A54();
                    SH(s, 8) = c % 32 - 0x10;
                    c = func_80071A54();
                    SH(s, 0xA) = c % 32 - 0x10;
                    c = func_80071A54();
                    SH(s, 0xC) = c % 32 - 0x10;
                    SH(s, 0x10) = 0;
                    SH(s, 0x12) = 0;
                }
            }
            if ((H(a1, 0xA) & 3) == 0) {
                s = func_800CE610(W(D_800F33E0, 8));
                if (s != 0) {
                    H(s, 0) = f.pos[0];
                    H(s, 2) = f.pos[1];
                    H(s, 4) = f.pos[2];
                    H(s, 0) = H(s, 0) - 0x100 + (func_80071A54() & 0x1FF);
                    H(s, 2) = H(s, 2) - 0x100 + (func_80071A54() & 0x1FF);
                    c = func_80071A54() & 0x1FF;
                    SH(s, 0x10) = 2;
                    SH(s, 0x12) = 0;
                    H(s, 4) = H(s, 4) - 0x100 + c;
                }
            }
        }
        D_8019993C = H(a1, 0);
        D_8019993E = H(a1, 2);
        D_80199940 = H(a1, 4);
        if (SH(a1, 0xA) < 0x8D)
            return 0;
        return 1;
    case 2:
        gte_SetRotMatrix(D_800BCFA4[0]);
        gte_SetTransMatrix(D_800BCFA4[0]);
        f.pos[0] = H(a1, 0);
        f.pos[1] = H(a1, 2);
        f.pos[2] = H(a1, 4);
        if (SH(a1, 0xA) < 0x8E) {
            c = SH(a1, 0xC);
            if (D_800E27EC & 1)
                c = c * 3 / 4;
            D_800F3374 = 0x28;
            func_800D004C(f.pos, 0x3E8, 0x3E8, 0x10, 0, 0x1000, 0x1000, &f.buf, 0, c, 1);
            func_800D0728(f.pos, 0x578, 0x708, 0x18, 0, SH(a1, 0xE), SH(a1, 0xE), 0, &f.buf, c / 2, 1);
            D_800F3372 = 0;
            D_800F3374 = 4;
            return 0;
        }
        D_800F3372 = 0;
        D_800F3374 = 4;
        return 0;
    }
    return 0;
}
