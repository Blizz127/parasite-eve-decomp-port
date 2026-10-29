/* room_m0256i — func_80192844, blob offset 0x385C, 0xC68 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (2026-10-08, written from the retail asm),
 * profile room_m0256i_dispatch_8018F1DC_8018F1F4 (both switches fold onto
 * the retail jump tables).
 * Particle callback spawned by func_801934AC: mode 1 integrates five particle
 * kinds (kinds 0/1 spawn kinds 2-4 and can trigger the room event), mode 2
 * loads the camera rotation into the GTE and draws the particle.
 * p[] layout: x,y,z, flag, vx,vy,vz, (unused), kind, frame. */

#define gte_SetRotMatrix(r0) __asm__ volatile(                       \
    "lw $12, 0(%0);" "lw $13, 4(%0);" "ctc2 $12, $0;" "ctc2 $13, $1;"   \
    "lw $12, 8(%0);" "lw $13, 12(%0);" "lw $14, 16(%0);"                \
    "ctc2 $12, $2;" "ctc2 $13, $3;" "ctc2 $14, $4;"                     \
    "lw $12, 20(%0);" "lw $13, 24(%0);" "ctc2 $12, $5;"                 \
    "lw $14, 28(%0);" "ctc2 $13, $6;" "ctc2 $14, $7"                    \
    : : "r"(r0) : "$12", "$13", "$14")

typedef struct { short vx, vy, vz, pad; } SVECTOR;
typedef struct { unsigned char r, g, b, cd; } CVECTOR;
extern SVECTOR D_8018F1CC;
extern CVECTOR D_8018F1D4;
extern short D_800942EC[];
extern short D_800F3368;
extern short D_800F336A;
extern unsigned short D_800F336C;
extern short D_800F3376;
extern short D_800F3378;
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern int *D_800BCFA4;
extern unsigned char *D_800E2368;
extern unsigned char *D_800F32D0;
extern unsigned char *D_800F33E0;
extern unsigned char **D_8009D254;
extern int D_8009D248;
extern unsigned short D_8009D1CC;
extern int func_80071A54();
extern int func_80077CF4(int);
extern int func_80077DC4(int);
extern unsigned short func_80077AA4(int, int);
extern short *func_800CE610();
extern int func_800C6B90(void *, int);
extern int func_8001CAB0(int, int, int, int);
extern void func_800CEE20(void *, void *, int, int, int, int, int, int, void *);
extern void func_800D004C(void *, int, int, int, int, int, int, void *, int, int, int);
extern void func_800D1DEC(void *, void *, int, int);

int func_80192844(int mode, short *p)
{
    SVECTOR sv;
    CVECTOR col;
    short *e;
    unsigned char *chan;
    int *w;
    int **matrix_slot;
    int palette;
    unsigned short clut;
    int a;
    int b;

    sv = D_8018F1CC;
    col = D_8018F1D4;
    switch (mode) {
    case 1:
        switch (p[8]) {
        case 0:
            p[9]++;
            p[0] += p[4];
            p[1] += p[5];
            p[2] += p[6];
            p[0] += (func_80071A54() & 0x1F) - 0x10;
            p[1] += (func_80071A54() & 0x1F) - 0x10;
            p[2] += (func_80071A54() & 0x1F) - 0x10;
            if (p[1] >= D_800942EC[0] - 0x10 || p[9] >= 0x18) {
                p[8] = 1;
                p[1] = D_800942EC[0] - 0x10;
                p[5] = 0;
            }
            if (p[9] & 1) {
                if (!(func_80071A54() & 7)) {
                    e = func_800CE610(*(void **)(D_800F33E0 + 8));
                    if (e) {
                        e[0] = p[0];
                        e[1] = p[1];
                        e[2] = p[2];
                        e[4] = p[4] / 2 + (func_80071A54() & 0x1F) - 0x10;
                        e[5] = p[5] / 2 + (func_80071A54() & 0x1F) - 0x10;
                        e[6] = p[6] / 2 + (func_80071A54() & 0x1F) - 0x10;
                        e[8] = 2;
                        e[9] = 0;
                    }
                }
                if ((func_80071A54() & 7) == 1) {
                    e = func_800CE610(*(void **)(D_800F33E0 + 8));
                    if (e) {
                        e[0] = p[0];
                        e[1] = p[1];
                        e[2] = p[2];
                        e[4] = (func_80071A54() & 0x1F) - 0x10;
                        e[5] = (func_80071A54() & 0x1F) - 0x10;
                        e[6] = (func_80071A54() & 0x1F) - 0x10;
                        e[8] = 3;
                        e[9] = 0;
                    }
                }
                if ((func_80071A54() & 0xF) == 2) {
                    e = func_800CE610(*(void **)(D_800F33E0 + 8));
                    if (e) {
                        e[0] = p[0];
                        e[1] = p[1];
                        e[2] = p[2];
                        e[4] = (func_80071A54() & 0x1F) - 0x10;
                        e[5] = (func_80071A54() & 0x1F) - 0x18;
                        e[6] = (func_80071A54() & 0x1F) - 0x10;
                        e[8] = 4;
                        e[9] = 0;
                    }
                }
            }
            if (func_800C6B90(p, 0x19A) && p[3] && D_800E2368[0xD]) {
                chan = D_800F32D0;
                if ((***(int ***)(chan + 8) & 0x3F000000) == 0x01000000) {
                    *(int *)(*D_8009D254 + 0x4C) |= 0x4000;
                    w = **(int ***)(chan + 8);
                    *w = (*w & 0xC0FFFFFF) | 0x19000000;
                    w = **(int ***)(chan + 8);
                    *w |= 0x80000000;
                }
            }
            if (!func_8001CAB0(p[0] << 16, p[2] << 16, D_8009D248, D_8009D1CC)) return 1;
            if (p[9] >= 0x20) return 1;
            break;
        case 1:
            p[9]++;
            p[0] += p[4];
            p[1] += p[5];
            p[2] += p[6];
            p[0] += (func_80071A54() & 0x1F) - 0x10;
            p[2] += (func_80071A54() & 0x1F) - 0x10;
            if (func_800C6B90(p, 0x26C) && p[3] && D_800E2368[0xD]) {
                chan = D_800F32D0;
                if ((***(int ***)(chan + 8) & 0x3F000000) == 0x01000000) {
                    *(int *)(*D_8009D254 + 0x4C) |= 0x4000;
                    w = **(int ***)(chan + 8);
                    *w = (*w & 0xC0FFFFFF) | 0x19000000;
                    w = **(int ***)(chan + 8);
                    *w |= 0x80000000;
                }
            }
            if (!func_8001CAB0(p[0] << 16, p[2] << 16, D_8009D248, D_8009D1CC)) return 1;
            if (p[9] >= 0x20) return 1;
            break;
        case 2:
            p[9]++;
            p[0] += p[4];
            p[1] += p[5];
            p[2] += p[6];
            p[4] = p[4] * 127 / 128;
            p[6] = p[6] * 127 / 128;
            p[5] -= 2;
            if (p[1] >= D_800942EC[0]) p[5] *= -1;
            if (p[9] >= 16) return 1;
            break;
        case 3:
            p[9]++;
            p[0] += p[4];
            p[1] += p[5];
            p[2] += p[6];
            p[4] = p[4] * 511 / 512;
            p[6] = p[6] * 511 / 512;
            p[5] -= 3;
            if (p[1] >= D_800942EC[0]) p[5] *= -1;
            if (p[9] >= 16) return 1;
            break;
        case 4:
            p[9]++;
            p[0] += p[4];
            p[1] += p[5];
            p[2] += p[6];
            p[4] = p[4] * 31 / 32;
            p[6] = p[6] * 31 / 32;
            p[5] -= 3;
            if (p[1] >= D_800942EC[0]) p[5] *= -1;
            if (p[9] >= 16) return 1;
            break;
        }
        break;
    case 2:
        matrix_slot = &D_800BCFA4;
        asm volatile("" : "=r"(matrix_slot) : "0"(matrix_slot));
        gte_SetRotMatrix(*matrix_slot);
        asm volatile("" : : "r"(matrix_slot));
        switch (p[8]) {
        case 0:
            a = (func_80077CF4(p[9] << 5) + 0x1200) * 2;
            b = (func_80077CF4(p[9] << 5) + 0xE00) * 2;
            palette = D_800E1204[D_800F336C];
            if (D_800F336C == 4 && D_800F3428 != 0) palette += 4;
            func_800CEE20(p, 0, a, b, D_800F336A * (p[9] / 4), func_80077AA4(0, palette), 1, 0x72, 0);
            if (!(func_80071A54() & 3)) {
                func_800D004C(p, 0xFA, 0xFA, 5, 0, a, a, &col, 0,
                              func_80077DC4(p[9] << 5) / 32, 3);
            }
            break;
        case 1:
            a = (func_80077CF4(p[9] << 5) + 0x1200) * 2;
            palette = D_800E1204[D_800F336C];
            if (D_800F336C == 4 && D_800F3428 != 0) palette += 4;
            func_800CEE20(p, &sv, a, a, D_800F336A * (p[9] / 4) + 0x20, func_80077AA4(0x10, palette), 1, 0x80, 0);
            break;
        case 2:
            a = func_80077CF4(p[9] << 6) / 2 + 0x1200;
            palette = D_800E1204[D_800F336C];
            if (D_800F336C == 4 && D_800F3428 != 0) palette += 4;
            clut = func_80077AA4(0x10, palette);
            func_800CEE20(p, 0, a, a, D_800F336A * (p[9] / 2), clut, 1, 0x60, 0);
            break;
        case 3:
            a = func_80077CF4(p[9] << 6) / 2 + 0x1000;
            palette = D_800E1204[D_800F336C];
            if (D_800F336C == 4 && D_800F3428 != 0) palette += 4;
            clut = func_80077AA4(0x20, palette);
            func_800CEE20(p, 0, a, a, D_800F336A * (p[9] / 2) + 0x40, clut, 2, 0x50, 0);
            break;
        case 4:
            a = func_80077DC4(p[9] << 6) / 32 + 0x40;
            D_800F3368 = 0x20;
            D_800F336A = 2;
            D_800F3376 = 0x20;
            D_800F3378 = 0x20;
            func_800D1DEC(p, &col, a, 1);
            break;
        }
        break;
    }
    return 0;
}
