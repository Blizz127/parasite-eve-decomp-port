/* room_m0203i (PE.IMG room m0203i chunk 2, VRAM 0x8018EFE8)
 * func_80192F00 — blob offset 0x3F18, 0xDDC bytes. Local profile: -O2 -G0, MASPSX_THREE_WORD_SYMBOL_STORE=1,
 * MASPSX_DISPATCH_FOLD=jtbl_8018F1EC,jtbl_8018F204 (both load-bearing: 2 mismatches without either;
 * no div guards, so no EXPAND_DIV).
 * Shared room effect callback (cmd 1: particle update switch — case 0 falls and spawns
 * func_800CE610 debris, cases 1/3 bounce, 2/4 timers; cmd 2: GTE rot+trans from D_800BCFA4[0]
 * and a 5-state D_800F3368 sprite switch drawn with func_800CEE20 / func_800D004C).
 * Twin (masked-body port): room_m0231i func_80193650 (D_8018F1EC, D_8018F1F0,
 * jtbl_8018F1F8,jtbl_8018F210).
 * Levers (lane s887):
 *   - D_800E11E8 declared as an array and read as D_800E11E8[0]: a MEM_IN_STRUCT load then
 *     true-depends on the `sh 0(s0)` a68 store, which reproduces retail's
 *     lhu/li 16/sh/lhu order in mode-2 cases 1 and 3 (scalar: sched2 hoists li 16);
 *   - bounce negation `{ int n = y; SH(a1,0xA) = -n; }` keeps retail's sll/sra/negu
 *     (a plain -y is narrowed by fold to a bare negu);
 *   - `{ int z = SH(a1,0x12) << 7; v.z = z; }` gives lh (a short expression narrows to lhu);
 *   - mode-2 case 0 order: s2, v.x/v.y, v.z, v.w, init, tex;
 *   - one function-scope `int t` shared by mode-2 cases 2 and 4 (retail homes both in $s3);
 *   - case 3 divisor is *1024/12 (magic 0x2AAAAAAB, sra 1).
 * inline GTE/asm: Psy-Q inline_c.h-shaped GTE (COP2) macros (SetRotMatrix/SetTransMatrix);
 * count separately from plain C.
 * Hand-written in this repo by lane-s887 (khasinski's vendored tree has this body as asm only). */
#define MB() __asm__ volatile("" : : : "memory")
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

extern B4 D_8018F1E0;
extern B4 D_8018F1E4;
extern unsigned char *D_800E2368;
extern void *D_800F32D0;
extern void *D_800F33E0;
extern void **D_8009D254;
extern short D_800942EC;
extern int D_8009D248;
extern unsigned short D_8009D1CC;
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

#define INIT16() \
    D_800F3368.a68 = 0x10; \
    D_800F3368.a6A = 1; \
    D_800F3368.a76 = 0x10; \
    D_800F3368.a78 = 0x10; \
    D_800F3368.a70 = D_800E2850[D_800E11E8[0]]; \
    D_800F3368.a6C = 2; \
    D_800F3368.a6E = 0;

#define TEX(w) \
    w = D_800E1204[D_800F3368.a6C]; \
    if (D_800F3368.a6C == 4 && D_800F3428 != 0) \
        w += 4;

int func_80192F00(int mode, unsigned char *a1, short *a2)
{
    GteShortVector v;
    B4 buf1;
    B4 buf2;
    GteShortVector d;
    GteShortVector ang;
    unsigned char *e;
    void *o;
    int i;
    int s2;
    int w;
    int t;

    { B4 *p1 = &D_8018F1E0; buf1 = *p1; }
    { B4 *p2 = &D_8018F1E4; buf2 = *p2; }

    switch (mode) {
    case 1:
        switch (SH(a1, 0x10)) {
        case 0:
            H(a1, 0x12)++;
            func_800CFB7C(a1 + 8, *a2, &v);
            H(a1, 0) += v.x;
            H(a1, 2) += v.y;
            H(a1, 4) += v.z;
            if (!(H(a1, 0x12) & 1)) {
                e = func_800CE610(P(D_800F33E0, 8));
                if (e != 0) {
                    H(e, 0) = H(a1, 0);
                    H(e, 2) = H(a1, 2);
                    H(e, 4) = H(a1, 4);
                    H(e, 8) = func_80071A54() % 32 - 16;
                    H(e, 0xA) = func_80071A54() % 32 - 16;
                    H(e, 0xC) = func_80071A54() % 32 - 16;
                    SH(e, 0x10) = 1;
                    SH(e, 0x12) = 0;
                }
            }
            if (func_800C6B90(a1, 0x32) != 0 && D_800942EC - 0x202 < SH(a1, 2)) {
                if (D_800E2368[0xD] != 0) {
                    o = D_800F32D0;
                    if ((**(int **)P(o, 8) & 0x3F000000) == 0x01000000) {
                        W(*D_8009D254, 0x4C) |= 0x4000;
                        **(int **)P(o, 8) = (**(int **)P(o, 8) & 0xC0FFFFFF) | 0x19000000;
                        **(int **)P(o, 8) |= 0x80000000;
                    }
                }
                SH(a1, 0x10) = 2;
                SH(a1, 0x12) = 0;
                for (i = 0; i < 6; i++) {
                    e = func_800CE610(P(D_800F33E0, 8));
                    if (e != 0) {
                        H(e, 0) = H(a1, 0);
                        H(e, 2) = H(a1, 2);
                        H(e, 4) = H(a1, 4);
                        H(e, 8) = func_80071A54() % 40 - 20;
                        H(e, 0xA) = func_80071A54() % 40 - 20;
                        H(e, 0xC) = func_80071A54() % 40 - 20;
                        SH(e, 0x10) = 3;
                        SH(e, 0x12) = 0;
                    }
                }
            }
            if (SH(a1, 2) >= D_800942EC) {
                SH(a1, 0x10) = 2;
                SH(a1, 0x12) = 0;
                for (i = 0; i < 6; i++) {
                    e = func_800CE610(P(D_800F33E0, 8));
                    if (e != 0) {
                        H(e, 0) = H(a1, 0);
                        H(e, 2) = H(a1, 2);
                        H(e, 4) = H(a1, 4);
                        H(e, 8) = func_80071A54() % 40 - 20;
                        H(e, 0xA) = func_80071A54() % 40 - 20;
                        H(e, 0xC) = func_80071A54() % 40 - 20;
                        SH(e, 0x10) = 3;
                        SH(e, 0x12) = 0;
                    }
                }
            }
            if (func_8001CAB0(SH(a1, 0) << 16, SH(a1, 4) << 16, D_8009D248, D_8009D1CC) != 0)
                return 0;
            SH(a1, 0x10) = 2;
            SH(a1, 0x12) = 0;
            break;
        case 1:
            H(a1, 0x12)++;
            H(a1, 0) += H(a1, 8);
            H(a1, 2) += H(a1, 0xA);
            H(a1, 4) += H(a1, 0xC);
            SH(a1, 8) = SH(a1, 8) * 63 / 64;
            SH(a1, 0xC) = SH(a1, 0xC) * 63 / 64;
            {
            short y = SH(a1, 0xA) - 1;
            SH(a1, 0xA) = y;
            if (SH(a1, 2) >= D_800942EC) {
                int n = y;
                SH(a1, 0xA) = -n; }
            }
            if (SH(a1, 0x12) >= 6)
                return 1;
            break;
        case 3:
            H(a1, 0x12)++;
            H(a1, 0) += H(a1, 8);
            H(a1, 2) += H(a1, 0xA);
            H(a1, 4) += H(a1, 0xC);
            SH(a1, 8) = SH(a1, 8) * 63 / 64;
            SH(a1, 0xC) = SH(a1, 0xC) * 63 / 64;
            {
            short y = SH(a1, 0xA) + 1;
            SH(a1, 0xA) = y;
            if (SH(a1, 2) >= D_800942EC) {
                int n = y;
                SH(a1, 0xA) = -n; }
            }
            if (SH(a1, 0x12) >= 0xC)
                return 1;
            break;
        case 2:
        case 4:
            H(a1, 0x12)++;
            if (SH(a1, 0x12) >= 0x10)
                return 1;
            break;
        }
        break;
    case 2:
        gte_SetRotMatrix(D_800BCFA4[0]);
        gte_SetTransMatrix(D_800BCFA4[0]);
        switch (SH(a1, 0x10)) {
        case 0:
            s2 = 0x80 - ((H(a1, 0x12) & 1) << 6);
            v.x = 0; v.y = 0;
            { int z = SH(a1, 0x12) << 7; v.z = z; }
            v.w = 0;
            INIT16();
            TEX(w);
            func_800CEE20(a1, &v, 0x1000, 0x1000, 0xD8, func_80077AA4(0x20, w) & 0xFFFF, 1, s2, &buf1);
            TEX(w);
            func_800CEE20(a1, &v, 0x2000, 0x2000, D_800F3368.a6A + 0xD8, func_80077AA4(0x20, w) & 0xFFFF, 1, s2 / 2, &buf2);
            ang.x = 0x400;
            ang.y = 0;
            ang.z = 0;
            ang.w = 1;
            d.x = H(a1, 0);
            d.z = H(a1, 4);
            d.y = D_800942EC;
            TEX(w);
            func_800CEE20(&d, &ang, 0x1000, 0x1000, D_800F3368.a6A + 0xD8, func_80077AA4(0x20, w) & 0xFFFF, 1, s2, &buf2);
            break;
        case 1:
            s2 = func_80077CF4(SH(a1, 0x12) << 7) / 64 + 0x40;
            INIT16();
            TEX(w);
            func_800CEE20(a1, 0, 0x1000, 0x1000, D_800F3368.a6A * SH(a1, 0x12) + 0xE0, func_80077AA4(0x20, w) & 0xFFFF, 1, s2, &buf2);
            break;
        case 2:
        {
            s2 = func_80077DC4(SH(a1, 0x12) << 6) / 32;
            if (H(a1, 0x12) & 1)
                s2 = s2 * 2 / 3;
            D_800F3368.a68 = 0x20;
            D_800F3368.a6A = 2;
            D_800F3368.a76 = 0x20;
            D_800F3368.a78 = 0x20;
            D_800F3368.a70 = D_800E2850[D_800E11EC];
            D_800F3368.a6C = 4;
            D_800F3368.a6E = 0;
            TEX(w);
            func_800CEE20(a1, 0, 0x1800, 0x1800, D_800F3368.a6A * (SH(a1, 0x12) / 2) + 0xC0, func_80077AA4(0, w) & 0xFFFF, 1, s2, &buf1);
            t = func_80077CF4(SH(a1, 0x12) << 6) + 0x800;
            func_800D004C(a1, 0x12C, 0x12C, 5, 0, t, t, &buf2, 0, s2 / 2, 1);
            return 0;
        }
        case 3:
            s2 = func_80077DC4(SH(a1, 0x12) * 1024 / 12) / 32;
            INIT16();
            TEX(w);
            func_800CEE20(a1, 0, 0x1000, 0x1000, D_800F3368.a6A * (SH(a1, 0x12) * 2 / 3) + 0xC8, func_80077AA4(0x20, w) & 0xFFFF, 1, s2, &buf2);
            break;
        case 4:
        {
            t = func_80077CF4(SH(a1, 0x12) << 6) + 0x1000;
            s2 = func_80077DC4(SH(a1, 0x12) << 6) / 32;
            SH(a1, 0xE) = 1;
            SH(a1, 0xC) = SH(a1, 0x12) * 24;
            INIT16();
            TEX(w);
            func_800CEE20(a1, a1 + 8, t, t, D_800F3368.a6A * 2 + 0xFD, func_80077AA4(0x90, w) & 0xFFFF, 1, s2, 0);
            break;
        }
        }
        break;
    }
    return 0;
}
