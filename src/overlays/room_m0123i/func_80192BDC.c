/* room_m0123i (PE.IMG room m0123i chunk 2, VRAM 0x8018EFE8)
 * func_80192BDC — blob offset 0x3bf4, 0x330 bytes. Profile era_o2_g0 (default).
 * Particle callback of func_80192F0C's pool (mode 1 = age + spin, done at 48;
 * 2 = draw: before frame 16 a func_800CEE20 sprite + func_800D1DEC glow at the
 * func_800CF844 position, from frame 16 a func_800D2B58 streak towards
 * D_80195690).
 * inline GTE/asm: uses Psy-Q inline_c.h-shaped GTE (COP2) asm macros — count
 * separately from plain C. The camera slot goes through a register barrier
 * (as in func_800CF5B0) to keep retail's address-then-load of D_800BCFA4.
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
extern short D_800F336A;
extern unsigned short D_800F336C;
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern unsigned char D_801954BC[];
extern short D_80195684;
extern SVECTOR D_80195688;
extern SVECTOR D_80195690;
extern int func_80077AA4(int a0, int a1);
extern int func_80077DC4(int a0);
extern void func_800783E4(void *first, void *second, int first_scale, int second_scale, void *out);
extern void func_800CEE20(short *a0, short *a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8);
extern void func_800CF3AC(void *track, CVECTOR *color, int time);
extern void func_800CF844(void *, void *, int, void *, int, int);
extern void func_800D1DEC(void *position, void *color, int scale, int flags);
extern void func_800D2B58();

int func_80192BDC(int mode, short *p)
{
    SVECTOR pos;
    SVECTOR mid;
    CVECTOR col;
    int s;
    int t;
    MATRIX **slot;

    switch (mode) {
    case 1:
        p[0]++;
        p[1] += 16;
        if (p[0] < 48) {
            break;
        }
        return 1;
    case 2:
        s = p[2] * func_80077DC4(p[0] * 1024 / 48) / 4096;
        func_800CF844(&D_80195690, &pos, s / 2, &D_80195688, s, p[1]);
        if (p[0] < 16) {
            func_800CF3AC(D_801954BC, &col, 48 - s * 48 / 1024);
            func_800CEE20((short *)&pos, 0, 0x1000, 0x1000, D_800F336A * 2 + 0xD8,
                          func_80077AA4(32, D_800E1204[D_800F336C] +
                                        ((D_800F336C == 4 && D_800F3428) ? 4 : 0)) & 0xFFFF,
                          3, D_80195684, (int)&col);
            slot = &D_800BCFA4.value;
            __asm__ volatile ("" : "=r"(slot) : "0"(slot));
            gte_SetRotMatrix(*slot);
            gte_SetTransMatrix(*slot);
            func_800D1DEC(&pos, &col, D_80195684, 1);
        }
        if (p[0] >= 16) {
            slot = &D_800BCFA4.value;
            __asm__ volatile ("" : "=r"(slot) : "0"(slot));
            gte_SetRotMatrix(*slot);
            gte_SetTransMatrix(*slot);
            func_800CF3AC(D_801954BC, &col, 48 - s * 48 / 1024);
            t = func_80077DC4((p[0] - 16) * 32);
            func_800783E4(&pos, &D_80195690, t, 0x1000 - t, &mid);
            func_800D2B58(&mid, &pos, &col, 0, D_80195684, 0, 1);
        }
        break;
    }
    return 0;
}
