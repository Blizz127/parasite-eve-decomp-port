/* room_m0123i (PE.IMG room m0123i chunk 2, VRAM 0x8018EFE8)
 * func_8019251C — blob offset 0x3534, 0x4E0 bytes. Profile era_o2_g0 (default).
 * Day 3 fight effect callback (a0 = mode, a1 = state shorts): mode 1 ages the
 * effect (state 0: 24-frame grow; state 1: p[2] spins by 0x200 mod 0x3000 for
 * 24 frames); mode 2 draws a triangle of func_800D2B58 streaks between three
 * func_800CE8F0 model points (state 0, shrinking towards a fourth point) or a
 * func_800D1DEC glow travelling round the triangle edges (state 1).
 * The 8/8/4-byte rodata templates sit at the head of the text window
 * (splat labels 0x8018F1CC as a function); copied to the stack by value.
 * inline GTE/asm: uses Psy-Q inline_c.h-shaped GTE (COP2) asm macros — count
 * separately from plain C. Levers: the inner switch ends with `break` (no
 * cross-jumped `return 1` tails), case 1 keeps p[2] in its own temp, and the
 * glow scale goes through `s` so the streak width is a multi-set pseudo
 * (retail's quotient-then-copy into $s0). The second camera load uses the
 * register barrier (as in func_800CF5B0); the first two use the plain global.
 * Hand-matched (no masked twins). */
#define gte_SetRotMatrix(r0) __asm__ volatile ( \
    "lw $12,0(%0)\n\tlw $13,4(%0)\n\tctc2 $12,$0\n\tctc2 $13,$1\n\t" \
    "lw $12,8(%0)\n\tlw $13,12(%0)\n\tlw $14,16(%0)\n\t" \
    "ctc2 $12,$2\n\tctc2 $13,$3\n\tctc2 $14,$4" : : "r"(r0) : "$12", "$13", "$14")
#define gte_SetTransMatrix(r0) __asm__ volatile ( \
    "lw $12,20(%0)\n\tlw $13,24(%0)\n\tctc2 $12,$5\n\tlw $14,28(%0)\n\t" \
    "ctc2 $13,$6\n\tctc2 $14,$7" : : "r"(r0) : "$12", "$13", "$14")

typedef struct { short vx, vy, vz, pad; } SVECTOR;
typedef struct { short m[3][3]; int t[3]; } MATRIX;
typedef struct { unsigned char r, g, b, cd; } CVECTOR;
typedef struct { MATRIX *value; unsigned char reserved[8]; } RenderMatrixSlot;
extern RenderMatrixSlot D_800BCFA4;
extern char *D_800F32D0;
extern SVECTOR D_8018F1CC;
extern SVECTOR D_8018F1D4;
extern CVECTOR D_8018F1DC;
extern int func_80077CF4(int angle);
extern void func_800783E4(void *first, void *second, int first_scale, int second_scale, void *out);
extern int func_800CE8F0();
extern void func_800D1DEC(void *position, void *color, int scale, int flags);
extern void func_800D2B58();

int func_8019251C(int mode, short *p)
{
    SVECTOR v0;
    SVECTOR v1;
    SVECTOR v2;
    SVECTOR v3;
    SVECTOR a = D_8018F1CC;
    SVECTOR b = D_8018F1D4;
    CVECTOR c = D_8018F1DC;
    MATRIX **slot;
    int s;
    int w;
    int t;

    switch (mode) {
    case 1:
        switch (p[0]) {
        case 0:
            p[1]++;
            if (p[1] < 24) {
                break;
            }
            return 1;
        case 1:
            p[1]++;
            p[2] = (short)(p[2] + 0x200) % 0x3000;
            if (p[1] >= 24) {
                return 1;
            }
            break;
        }
        break;
    case 2:
        gte_SetRotMatrix(D_800BCFA4.value);
        gte_SetTransMatrix(D_800BCFA4.value);
        func_800CE8F0(*(void **)(D_800F32D0 + 8), 0x28, &a, &v0);
        func_800CE8F0(*(void **)(D_800F32D0 + 8), 0x22, &a, &v1);
        func_800CE8F0(*(void **)(D_800F32D0 + 8), 0x1D, &b, &v2);
        switch (p[0]) {
        case 0:
            func_800CE8F0(*(void **)(D_800F32D0 + 8), 0x17, &b, &v3);
            w = 0x5000 - func_80077CF4(p[1] * 1024 / 24) * 4;
            func_800783E4(&v3, &v0, 0x1000 - w, w, &v0);
            func_800783E4(&v3, &v1, 0x1000 - w, w, &v1);
            func_800783E4(&v3, &v2, 0x1000 - w, w, &v2);
            s = p[1] * 128 / 24;
            gte_SetRotMatrix(D_800BCFA4.value);
            gte_SetTransMatrix(D_800BCFA4.value);
            func_800D2B58(&v0, &v1, &c, &c, s, s, 1);
            func_800D2B58(&v1, &v2, &c, &c, s, s, 1);
            func_800D2B58(&v2, &v0, &c, &c, s, s, 1);
            return 0;
        case 1:
            t = p[2];
            if (t < 0x1000) {
                w = t;
                func_800783E4(&v0, &v1, 0x1000 - w, w, &v0);
            } else if (t < 0x2000) {
                w = t - 0x1000;
                func_800783E4(&v1, &v2, 0x1000 - w, w, &v0);
            } else {
                w = t - 0x2000;
                func_800783E4(&v2, &v0, 0x1000 - w, w, &v0);
            }
            slot = &D_800BCFA4.value;
            __asm__ volatile ("" : "=r"(slot) : "0"(slot));
            gte_SetRotMatrix(*slot);
            gte_SetTransMatrix(*slot);
            s = func_80077CF4(p[1] << 8) / 32;
            func_800D1DEC(&v0, &c, s, 1);
            break;
        }
        break;
    }
    return 0;
}
