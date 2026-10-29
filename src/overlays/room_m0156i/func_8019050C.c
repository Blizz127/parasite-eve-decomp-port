/* room_m0156i - func_8019050C, blob offset 0x1524, 0x3A4 bytes. Flags -O2 -G0;
 * written from the retail asm (2026-10-09).
 * Spark particle for the m0156i roof fight (effect callback, mode/p/def):
 * mode 1 integrates the spark (two drag kinds, 31/32 and 511/512, gravity
 * 1 or 2, bounce on the floor height D_800942EC) and ends after 16 frames;
 * mode 2 draws it as a func_800CEE20 glow (kind 0: fixed size, growing
 * alpha; kind 1: cosine-pulsed size, phase (t << 10) / 24). Was the Day 4 m0156i battle stop
 * CPU_BOUNDARY/REFUSED pc=0x8019050C ra=@800D413C:effect.
 * Matching notes: the matrix_slot barrier keeps the D_800BCFA4 load in $9
 * for the macro (as room_m0159i func_8018F3A4). */

#define gte_SetRotMatrix(r0) __asm__ volatile(                       \
    "lw $12, 0(%0);" "lw $13, 4(%0);" "ctc2 $12, $0;" "ctc2 $13, $1;"   \
    "lw $12, 8(%0);" "lw $13, 12(%0);" "lw $14, 16(%0);"                \
    "ctc2 $12, $2;" "ctc2 $13, $3;" "ctc2 $14, $4;"                     \
    "lw $12, 20(%0);" "lw $13, 24(%0);" "ctc2 $12, $5;"                 \
    "lw $14, 28(%0);" "ctc2 $13, $6;" "ctc2 $14, $7"                    \
    : : "r"(r0) : "$12", "$13", "$14")

extern short D_800942EC[];
extern int *D_800BCFA4;
extern unsigned short D_800E1204[];
extern short D_800F336A;
extern unsigned short D_800F336C;
extern int D_800F3428;
extern int func_80077AA4(int, int);
extern int func_80077CF4(int);
extern void func_800CEE20();

int func_8019050C(int mode, short *p)
{
    int t;
    int clut;
    int **matrix_slot;

    switch (mode) {
    case 1:
        switch (p[8]) {
        case 0:
            p[9]++;
            p[0] += p[4];
            p[1] += p[5];
            p[2] += p[6];
            p[4] = p[4] * 31 / 32;
            p[6] = p[6] * 31 / 32;
            p[5] -= 1;
            if (p[1] >= D_800942EC[0]) {
                p[5] *= -1;
            }
            break;
        case 1:
            p[9]++;
            p[0] += p[4];
            p[1] += p[5];
            p[2] += p[6];
            p[4] = p[4] * 511 / 512;
            p[6] = p[6] * 511 / 512;
            p[5] -= 2;
            if (p[1] >= D_800942EC[0]) {
                p[5] *= -1;
            }
            break;
        default:
            return 0;
        }
        if (p[9] >= 16) {
            return 1;
        }
        break;
    case 2:
        matrix_slot = &D_800BCFA4;
        asm volatile("" : "=r"(matrix_slot) : "0"(matrix_slot));
        gte_SetRotMatrix(*matrix_slot);
        asm volatile("" : : "r"(matrix_slot));
        switch (p[8]) {
        case 0:
            t = p[9];
            if (t >= 8) {
                t += 8;
            }
            clut = D_800E1204[D_800F336C];
            if (D_800F336C == 4 && D_800F3428) {
                clut += 4;
            }
            func_800CEE20(p, 0, 0xC00, 0xC00, D_800F336A * t + 32, func_80077AA4(16, clut) & 0xFFFF, 1, 128, 0);
            break;
        case 1:
            t = func_80077CF4((p[9] << 10) / 24) + 0x1000;
            t /= 2;
            clut = D_800E1204[D_800F336C];
            if (D_800F336C == 4 && D_800F3428) {
                clut += 4;
            }
            func_800CEE20(p, 0, t, t, D_800F336A * (p[9] / 4) + 128, func_80077AA4(48, clut) & 0xFFFF, 1, 96, 0);
            break;
        default:
            return 0;
        }
        break;
    }
    return 0;
}
