/* room_m0432i — func_80195B40, blob offset 0x6B58, 0x7BC bytes. NON_MATCHING
 * (behaviour C, proven by the e21 differential; not a byte match).
 * Effect spawner / callback (a0 = command 0/1/2, a1 = effect record), read
 * from the retail disassembly.
 *   cmd 0: clears the record's +8..+0xE, plays two sounds through
 *          func_8006DF50 when D_800B0E64 is set, registers the record with
 *          func_800CE8F0 and the callback func_801957CC with func_800CE560.
 *   cmd 1: steps the record. State 0 (30 frames): +0xA frame counter,
 *          +0xC = sin(f<<10 / 30) / 32, +0xE = cos(..) + 0x400, spawns four
 *          debris particles per frame while f < 9, then eight at the switch to
 *          state 1. State 1 (20 frames): counter / 20, sin and cos swapped.
 *   cmd 2: draws. Sets the D_800F3368 draw block and the GTE rotation /
 *          translation from D_800BCFA4[0]; state 0 draws through
 *          func_800D004C and func_800CEE20, state 1 through func_800D1AE0 /
 *          func_800D0728 and func_800CEE20. Every cmd 2 path ends with
 *          D_800F3372 = 0, D_800F3374 = 4 and returns 0.
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

extern GteShortVector D_8018F1E0;
extern GteShortVector D_8018F1F4;
extern B4 D_8018F214;
extern int D_800B0E64;
extern unsigned char *D_800F32D0;
extern unsigned char *D_800F33E0;
extern MATRIX *D_800BCFA4[];
extern unsigned short D_800E11FA;
extern unsigned short D_800E120A;
extern unsigned short D_800E2850[];
extern unsigned short D_800E1204[];
extern int D_800E27EC;
extern int D_800F3428;
extern struct { short a68, a6A; unsigned short a6C; short a6E; unsigned short a70; short a72, a74, a76, a78; } D_800F3368;

extern unsigned char func_801957CC[];
extern int func_80071A54();
extern int func_80077CF4();
extern int func_80077DC4();
extern int func_80077AA4();
extern int func_8006DF50();
extern int func_800CE560();
extern unsigned char *func_800CE610();
extern void func_800CE8F0();
extern void func_800CEE20();
extern void func_800D004C();
extern void func_800D0728();
extern void func_800D1AE0();
extern int func_800D3FD8();

int func_80195B40(int mode, unsigned char *a1)
{
    /* retail frame: b30 @ sp+0x30, b38 @ +0x38, b40 @ +0x40, b48 @ +0x48;
     * callees read these as one block through the b38 pointer */
    struct { GteShortVector b30; unsigned short b38[3]; short pad; GteShortVector b40; B4 b48; } L;
    unsigned char *p;
    int f, s0, v, i, s4, a, clut;

    L.b30 = D_8018F1E0;
    L.b40 = D_8018F1F4;
    L.b48 = D_8018F214;

    if (mode == 1) {
        L.b38[0] = H(a1, 0);
        L.b38[1] = H(a1, 2);
        L.b38[2] = H(a1, 4);
        switch (SH(a1, 8)) {
        case 0:
            f = H(a1, 0xA) + 1;
            H(a1, 0xA) = f;
            s0 = ((short)f * 1024) / 30;
            v = func_80077CF4(s0);
            H(a1, 0xC) = v / 32;
            H(a1, 0xE) = func_80077DC4(s0) + 0x400;
            if (SH(a1, 0xA) < 9) {
                for (i = 0; i < 4; i++) {
                    p = func_800CE610(*(int *)(D_800F33E0 + 8));
                    if (p) {
                        H(p, 0) = L.b38[0];
                        H(p, 2) = L.b38[1];
                        H(p, 4) = L.b38[2];
                        H(p, 0) = H(p, 0) - 0x80 + (func_80071A54() & 0xFF);
                        H(p, 2) = H(p, 2) - 0x80 + (func_80071A54() & 0xFF);
                        H(p, 4) = H(p, 4) - 0x80 + (func_80071A54() & 0xFF);
                        H(p, 6) = (func_80071A54() & 0x3FF) + 0x578;
                        H(p, 8) = func_80071A54();
                        H(p, 0xA) = func_80071A54();
                        H(p, 0xC) = func_80071A54();
                        H(p, 0x10) = 1;
                        H(p, 0x12) = 0;
                    }
                }
            }
            if (SH(a1, 0xA) >= 0x1E) {
                H(a1, 0xA) = 0;
                SH(a1, 8) = 1;
                for (i = 0; i < 8; i++) {
                    p = func_800CE610(*(int *)(D_800F33E0 + 8));
                    if (p) {
                        H(p, 0) = L.b38[0];
                        H(p, 2) = L.b38[1];
                        H(p, 4) = L.b38[2];
                        H(p, 8) = func_80071A54();
                        H(p, 0xA) = func_80071A54();
                        H(p, 0xC) = func_80071A54();
                        H(p, 0x10) = 0;
                        H(p, 0x12) = 0;
                    }
                }
            }
            return 0;
        case 1:
            f = H(a1, 0xA) + 1;
            H(a1, 0xA) = f;
            s0 = ((short)f * 1024) / 20;
            v = func_80077DC4(s0);
            H(a1, 0xC) = v / 32;
            H(a1, 0xE) = func_80077CF4(s0);
            if (SH(a1, 0xA) < 0x14)
                return 0;
            return 1;
        }
        return 0;
    }
    if (mode == 0) {
        H(a1, 8) = 0;
        H(a1, 0xA) = 0;
        H(a1, 0xC) = 0;
        H(a1, 0xE) = 0;
        if (D_800B0E64) {
            v = func_800D3FD8(0, 0, 0, 1);
            func_8006DF50(D_800B0E64, 0x5D6, v, 0x80, 0x7F);
            if (D_800B0E64)
                func_8006DF50(D_800B0E64, 0x5D7, 0x80, 0x80, 0x7F);
        }
        func_800CE8F0(*(int *)(D_800F32D0 + 8), 0, &L.b30, a1);
        return func_800CE560(*(int *)(D_800F33E0 + 8), 0x14, 0x28, func_801957CC);
    }
    if (mode != 2)
        return 0;

    gte_SetRotMatrix(D_800BCFA4[0]);
    gte_SetTransMatrix(D_800BCFA4[0]);
    L.b38[0] = H(a1, 0);
    L.b38[1] = H(a1, 2);
    D_800F3368.a68 = 0x20;
    D_800F3368.a6A = 2;
    D_800F3368.a76 = 0x20;
    D_800F3368.a78 = 0x20;
    L.b38[2] = H(a1, 4);
    D_800F3368.a6C = 3;
    D_800F3368.a6E = 1;
    D_800F3368.a72 = 0;
    D_800F3368.a70 = D_800E2850[D_800E11FA];

    switch (SH(a1, 8)) {
    case 0:
        s4 = SH(a1, 0xC);
        if (D_800E27EC & 1)
            s4 = (s4 * 3) / 4;
        D_800F3368.a74 = 0x20;
        func_800D004C(L.b38, 0x320, 0x2BC, 0x10, 0, 0x1000, 0x1000, &L.b48, 0, s4, 1);
        D_800F3368.a6A = 2;
        D_800F3368.a68 = 0x20;
        D_800F3368.a76 = 0x20;
        D_800F3368.a78 = 0x20;
        SH(&L.b40, 4) = SH(a1, 0xA) << 4;
        D_800F3368.a6C = 3;
        D_800F3368.a6E = 1;
        D_800F3368.a70 = D_800E2850[D_800E11FA];
        clut = func_80077AA4(0, D_800E120A + 4);
        func_800CEE20(a1, &L.b40, 0x1000, 0x1000,
                      100 + D_800F3368.a6A * (H(a1, 0xA) & 1), clut & 0xFFFF, 1, s4, 0);
        break;
    case 1:
        a = SH(a1, 0xA);
        if (a == 1) {
            func_800D1AE0(&L.b48, 0x80, 2, 8);
        } else if (a < 6) {
            v = func_80077CF4(((a - 2) << 10) / 3);
            func_800D1AE0(&L.b48, v / 32, 1, 8);
        }
        D_800F3368.a74 = 0x20;
        s4 = SH(a1, 0xC);
        func_800D0728(L.b38, 0x44C, 0x5DC, 0x18, 0, SH(a1, 0xE), SH(a1, 0xE), 0, &L.b48, s4 / 2, 1);
        s0 = SH(a1, 0xE) + 0x1000;
        a = D_800E1204[D_800F3368.a6C];
        /* Retail 80196260-80196278 (the a6C == 4 && D_800F3428 test).  a6C is
         * D_800F336C: the cmd 2 prefix stores 3 there at 80195FF0-80195FF4,
         * before the state switch at 8019602C/80196024, so state 1 always
         * reaches 8019623C with D_800F336C == 3 and 80196260-80196278 are
         * never executed.  Nothing in between writes it (func_800D1AE0,
         * func_80077CF4, func_800D0728 and their callees never reference
         * D_800F336C).  The arm is kept as retail has it. */
        if (D_800F3368.a6C == 4 && D_800F3428 != 0)
            a += 0xA;
        else
            a += 6;
        clut = func_80077AA4(0, a);
        func_800CEE20(L.b38, 0, s0, s0, 0x44, clut & 0xFFFF, 1, s4, 0);
        break;
    }
    D_800F3368.a72 = 0;
    D_800F3368.a74 = 4;
    return 0;
}
