/* room_m0413i (PE.IMG room m0413i chunk 2, VRAM 0x8018EFE8)
 * func_80192500 — blob offset 0x3518, 0x874 bytes. Profile era_o2_g0 (default; no div guards,
 * MASPSX_EXPAND_DIV is harmless but not needed).
 * Particle callback registered by func_80192D74 (func_800CE560, 0x37).
 * cmd 1: state 0 flies along a1+8 (func_800CFB7C), sheds func_800CE610 puffs every
 * 4th frame, lands on func_800C6B90 contact (owner flag poke 0x19000000) and dies on
 * a func_8001CAB0 wall miss; state 1 bounces (63/64 drag, -2 gravity, reflect at
 * D_800942EC) for 8 frames; state 2 is an 8-frame timer.
 * cmd 2: GTE rot+trans from D_800BCFA4[0]; state 0 draws func_800D2370 (plus a
 * func_800D004C ring every 4th D_800E27EC frame), states 1/2 draw func_800CEE20.
 * Masked-body twins (same address): room_m0416i, room_m0417i, room_m0437i.
 * Levers (lane r413; the family is room_m0203i func_80192F00):
 *   - gte_SetRot/TransMatrix(D_800BCFA4[0]) written directly (a matrix local gives
 *     lw %lo; the direct form gives retail's la + lw 0 into $t0, reused by case 0);
 *   - one function-scope `int t` shared by cmd-2 case 0 ((E27EC&1)<<5 + 0x60, passed
 *     as (short)t, halved signed) and case 1 (cos/64 + 0x40) — multiple sets stop
 *     combine proving t >= 0 (a single-set t gives srl 1) and home it in $s3;
 *   - cmd-2 cases 1/2: a70 = palette first (cse picks &D_800F3370 as the $s0 base,
 *     so a6A is read as -6($s0)), then a6C, a6E, a68, a6A, a76, a78;
 *   - D_800E11E8 as an array (room_m0203i lever); `== 0) return 1; break;` for the
 *     func_8001CAB0 tail.
 * inline GTE/asm: Psy-Q inline_c.h-shaped GTE (COP2) SetRot/SetTrans macros; count
 * separately from plain C. No ties or barriers. Hand-written in this repo (lane r413). */
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

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define H(o, x) (*(unsigned short *)((char *)(o) + (x)))
#define SH(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))

extern B4 D_8018F1CC;
extern B4 D_8018F1D0;
extern unsigned char *D_800E2368;
extern void *D_800F32D0;
extern void *D_800F33E0;
extern void **D_8009D254;
extern short D_800942EC;
extern int D_8009D248;
extern unsigned short D_8009D1CC;
extern int D_800E27EC;
extern unsigned short D_800E11E8[];
extern unsigned short D_800E11EC;
extern unsigned short D_800E2850[];
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern MATRIX *D_800BCFA4[];
extern struct { short a68, a6A; unsigned short a6C; short a6E; unsigned short a70; short a72, a74, a76, a78; } D_800F3368;

extern int func_800C6B90();
extern void *func_800CE610();
extern int func_80071A54();
extern void func_800CFB7C();
extern int func_8001CAB0();
extern int func_80077CF4();
extern int func_80077DC4();
extern int func_80077AA4();
extern void func_800CEE20();
extern void func_800D004C();
extern void func_800D2370();

#define TEX(w) \
    w = D_800E1204[D_800F3368.a6C]; \
    if (D_800F3368.a6C == 4 && D_800F3428 != 0) \
        w += 4;

int func_80192500(int mode, unsigned char *a1, short *a2)
{
    GteShortVector v;
    B4 buf1;
    B4 buf2;
    unsigned char *e;
    void *o;
    int s4;
    int w;
    int t;

    { B4 *p1 = &D_8018F1CC; buf1 = *p1; }
    { B4 *p2 = &D_8018F1D0; buf2 = *p2; }

    switch (mode) {
    case 1:
        switch (SH(a1, 0x10)) {
        case 0:
            H(a1, 0x12)++;
            func_800CFB7C(a1 + 8, *a2, &v);
            H(a1, 0) += v.x;
            H(a1, 2) += v.y;
            H(a1, 4) += v.z;
            if (!(H(a1, 0x12) & 3)) {
                e = func_800CE610(P(D_800F33E0, 8));
                if (e != 0) {
                    H(e, 0) = H(a1, 0);
                    H(e, 2) = H(a1, 2);
                    H(e, 4) = H(a1, 4);
                    H(e, 8) = (func_80071A54() & 0x1F) - 16;
                    H(e, 0xA) = 0;
                    H(e, 0xC) = (func_80071A54() & 0x1F) - 16;
                    SH(e, 0x10) = 1;
                    SH(e, 0x12) = 0;
                }
            }
            func_800CFB7C(a1 + 8, 0xB4, &v);
            v.x += H(a1, 0);
            v.y += H(a1, 2);
            v.z += H(a1, 4);
            if (func_800C6B90(&v, 0x32) != 0) {
                if (D_800E2368[0xD] != 0) {
                    o = D_800F32D0;
                    if ((**(int **)P(o, 8) & 0x3F000000) == 0x01000000) {
                        W(*D_8009D254, 0x4C) |= 0x4000;
                        **(int **)P(o, 8) = (**(int **)P(o, 8) & 0xC0FFFFFF) | 0x19000000;
                        **(int **)P(o, 8) |= 0x80000000;
                    }
                }
                H(a1, 0) = v.x;
                H(a1, 2) = v.y;
                H(a1, 4) = v.z;
                SH(a1, 0x10) = 2;
                SH(a1, 0x12) = 0;
            }
            if (func_8001CAB0(SH(a1, 0) << 16, SH(a1, 4) << 16, D_8009D248, D_8009D1CC) == 0)
                return 1;
            break;
        case 1:
            H(a1, 0x12)++;
            H(a1, 0) += H(a1, 8);
            H(a1, 2) += H(a1, 0xA);
            H(a1, 4) += H(a1, 0xC);
            SH(a1, 8) = SH(a1, 8) * 63 / 64;
            SH(a1, 0xC) = SH(a1, 0xC) * 63 / 64;
            {
            short y = SH(a1, 0xA) - 2;
            SH(a1, 0xA) = y;
            if (SH(a1, 2) >= D_800942EC) {
                int n = y;
                SH(a1, 0xA) = -n; }
            }
            if (SH(a1, 0x12) >= 8)
                return 1;
            break;
        case 2:
            H(a1, 0x12)++;
            if (SH(a1, 0x12) >= 8)
                return 1;
            break;
        }
        break;
    case 2:
        gte_SetRotMatrix(D_800BCFA4[0]);
        gte_SetTransMatrix(D_800BCFA4[0]);
        switch (SH(a1, 0x10)) {
        case 0:
            t = ((D_800E27EC & 1) << 5) + 0x60;
            gte_SetRotMatrix(D_800BCFA4[0]);
            gte_SetTransMatrix(D_800BCFA4[0]);
            D_800F3368.a70 = D_800E2850[D_800E11EC];
            D_800F3368.a6C = 4;
            D_800F3368.a6E = 0;
            TEX(w);
            func_800D2370(a1, a1 + 8, 0x140, 0x78, 0,
                          (D_800F3368.a6C == 4 && D_800F3428 != 0) ? 0x60 : 0,
                          0x40, 0x10, func_80077AA4(0, w) & 0xFFFF, &buf1, &buf2, (short)t, 1);
            if (D_800E27EC & 3)
                return 0;
            gte_SetRotMatrix(D_800BCFA4[0]);
            gte_SetTransMatrix(D_800BCFA4[0]);
            func_800CFB7C(a1 + 8, 0xF0, &v);
            v.x += H(a1, 0);
            v.y += H(a1, 2);
            v.z += H(a1, 4);
            func_800D004C(&v, 0xC8, 0xC8, 5, 0, 0x1000, 0x1000, &buf2, 0, t / 2, 1);
            break;
        case 1:
            s4 = func_80077DC4(SH(a1, 0x12) << 7) * 2 / 3;
            t = func_80077CF4(SH(a1, 0x12) << 7) / 64 + 0x40;
            D_800F3368.a70 = D_800E2850[D_800E11E8[0]];
            D_800F3368.a6C = 2;
            D_800F3368.a6E = 0;
            D_800F3368.a68 = 0x10;
            D_800F3368.a6A = 1;
            D_800F3368.a76 = 0x10;
            D_800F3368.a78 = 0x10;
            TEX(w);
            func_800CEE20(a1, 0, s4, s4, D_800F3368.a6A * (H(a1, 0x12) & 1) + 0xDC, func_80077AA4(0x30, w) & 0xFFFF, 1, t, &buf1);
            break;
        case 2:
            D_800F3368.a70 = D_800E2850[D_800E11E8[0]];
            D_800F3368.a6C = 2;
            D_800F3368.a6E = 0;
            D_800F3368.a68 = 0x20;
            D_800F3368.a6A = 2;
            D_800F3368.a76 = 0x20;
            D_800F3368.a78 = 0x20;
            TEX(w);
            func_800CEE20(a1, 0, 0x1000, 0x1000, D_800F3368.a6A * (SH(a1, 0x12) / 2) + 0xC0, func_80077AA4(0, w) & 0xFFFF, 1, 0x80, &buf2);
            break;
        }
        break;
    }
    return 0;
}
