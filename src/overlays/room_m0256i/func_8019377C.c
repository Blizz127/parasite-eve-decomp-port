/* room_m0256i — func_8019377C, blob offset 0x4794, 0x480 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (2026-10-08, written from the retail asm).
 * Per-particle callback spawned by func_80193BFC: mode 1 integrates the
 * particle (two motion kinds), mode 2 loads the camera rotation into the GTE
 * and draws the sprite.  p[] layout: x,y,z, flag, vx,vy,vz, phase, kind, frame. */

#define gte_SetRotMatrix(r0) __asm__ volatile(                       \
    "lw $12, 0(%0);" "lw $13, 4(%0);" "ctc2 $12, $0;" "ctc2 $13, $1;"   \
    "lw $12, 8(%0);" "lw $13, 12(%0);" "lw $14, 16(%0);"                \
    "ctc2 $12, $2;" "ctc2 $13, $3;" "ctc2 $14, $4;"                     \
    "lw $12, 20(%0);" "lw $13, 24(%0);" "ctc2 $12, $5;"                 \
    "lw $14, 28(%0);" "ctc2 $13, $6;" "ctc2 $14, $7"                    \
    : : "r"(r0) : "$12", "$13", "$14")

extern short D_800942EC[];
extern short D_800F336A;
extern unsigned short D_800F336C;
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern int *D_800BCFA4;
extern int func_80071A54();
extern int func_80077CF4(int);
extern int func_80077DC4(int);
extern unsigned short func_80077AA4(int, int);
extern void func_800CEE20(void *, void *, int, int, int, int, int, int, void *);

int func_8019377C(int mode, short *p)
{
    int **matrix_slot;
    int it;
    short s;
    unsigned short ut;
    int palette;
    unsigned short clut;
    int size;
    int blend;

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
            if (p[1] >= D_800942EC[0]) p[5] *= -1;
            p[0] += (func_80071A54() & 1) * 2 - 1;
            p[2] += (func_80071A54() & 1) * 2 - 1;
            if (p[9] >= 8) return 1;
            break;
        case 1:
            p[9]++;
            p[0] += p[4];
            p[1] += p[5];
            p[2] += p[6];
            p[4] = p[4] * 511 / 512;
            p[6] = p[6] * 511 / 512;
            p[5] -= 2;
            if (p[1] >= D_800942EC[0]) p[5] *= -1;
            p[7] += 0x100;
            p[0] += func_80077CF4(p[7] << 8) / 1024;
            p[2] += func_80077DC4(p[7] << 8) / 1024;
            if (p[9] >= 16) return 1;
            break;
        default:
            return 0;
        }
        break;
    case 2:
        matrix_slot = &D_800BCFA4;
        asm volatile("" : "=r"(matrix_slot) : "0"(matrix_slot));
        gte_SetRotMatrix(*matrix_slot);
        asm volatile("" : : "r"(matrix_slot));
        switch (p[8]) {
        case 0:
            palette = D_800E1204[D_800F336C];
            if (D_800F336C == 4 && D_800F3428 != 0) palette += 4;
            clut = func_80077AA4(0, palette);
            func_800CEE20(p, 0, 0x1800, 0x1800, D_800F336A * p[9],
                          clut, 1, 100, 0);
            break;
        case 1:
            blend = 2;
            size = func_80077CF4((p[9] << 10) / 24) + 0x1000;
            size /= 2;
            if (p[3]) blend = 1;
            palette = D_800E1204[D_800F336C];
            if (D_800F336C == 4 && D_800F3428 != 0) palette += 4;
            clut = func_80077AA4(0x20, palette);
            func_800CEE20(p, 0, size, size, D_800F336A * (p[9] / 2) + 0x40,
                          clut, blend, 0x50, 0);
            break;
        default:
            return 0;
        }
        break;
    default:
        return 0;
    }
    return 0;
}
