/* room_m0430i (PE.IMG room m0430i chunk 2, VRAM 0x8018EFE8)
 * func_8018F750 — blob offset 0x768, 0xF9C bytes. Profile
 * room_m0430i_dispatch_8018F00C_expand_div (-O2 -G0, header jump table fold,
 * MASPSX_EXPAND_DIV); LINK_EXACT at the room VMA (2026-10-08, written from
 * the retail asm). Was the m0430i story 0x140 CPU_BOUNDARY stop. */

/* 3-mode effect callback (task callback func_8018F028): arm (copy the
 * anchor position out of the room object, load model 0xC5543704 into
 * D_8019085C, spawn the paired sprite), per-frame (7 states: pick a random
 * heading, ease out, hold, ease to the slot target D_80190860, spark burst
 * and 12-spark ring, floor-hit test that can flag the actor, fade), and draw
 * (GTE matrix, two sprite modes, then the lit model and glow).
 * Matching notes: profile room_m0430i_dispatch_8018F00C_expand_div (header
 * jump table + retail's divide check); D_800F3368 is a struct only where a
 * store must stay after a table load (the scheduler treats a fixed struct
 * field and a varying element as aliasing); D_800B0E64 is read as an array
 * element for the same reason; `t` doubles as the draw flag so the 1024
 * scale constant wins $17; matrix_slot stays live after the second
 * gte_SetRotMatrix so reload picks $9. */

#define gte_SetRotMatrix(r0) __asm__ volatile(                       \
    "lw $12, 0(%0);" "lw $13, 4(%0);" "ctc2 $12, $0;" "ctc2 $13, $1;"   \
    "lw $12, 8(%0);" "lw $13, 12(%0);" "lw $14, 16(%0);"                \
    "ctc2 $12, $2;" "ctc2 $13, $3;" "ctc2 $14, $4;"                     \
    "lw $12, 20(%0);" "lw $13, 24(%0);" "ctc2 $12, $5;"                 \
    "lw $14, 28(%0);" "ctc2 $13, $6;" "ctc2 $14, $7"                    \
    : : "r"(r0) : "$12", "$13", "$14")

typedef struct { short vx, vy, vz, pad; } SVECTOR;
typedef struct { unsigned char r, g, b, cd; } CVECTOR;
typedef struct { int vx, vy, vz, pad; } VECTOR;
typedef struct { short m[3][3]; int t[3]; } MATRIX;
typedef struct { short x0, x2, x4, x6, x8, xA, xC, xE; int x10; int x14; } Slot;
extern CVECTOR D_8018EFFC;
extern CVECTOR D_8018F000;
extern CVECTOR D_8018F004;
extern CVECTOR D_8018F008;
extern int D_80190800;
extern void *D_8019085C;
extern void *D_800B0E64[];
extern unsigned short D_800942EC;
extern unsigned char **D_8009D254;
extern int *D_800BCFA4;
extern unsigned short D_800E11E4[];
extern unsigned short D_800E1204[];
extern unsigned short D_800E2850[];
extern unsigned char *D_800E2368;
extern int D_800E27EC;
extern unsigned char *D_800F32D0;
extern unsigned char *D_800F33E0;
typedef struct { short f0; short f2; unsigned short f4; short f6; unsigned short f8; short fa; short fc; short fe; short f10; } SPRP;
extern SPRP D_800F3368;
extern short D_800F336A;
extern unsigned short D_800F336C;
extern short D_800F336E;
extern unsigned short D_800F3370;
extern short D_800F3372;
extern short D_800F3374;
extern short D_800F3376;
extern short D_800F3378;
extern int D_800F3428;
extern void func_8018F028();
extern void *func_8006E498(void *, unsigned int);
extern void func_800C6D5C(void *, int, int);
extern void func_800D1384();
extern int func_800CE560();
extern short *func_800CE610();
extern void func_800CE870();
extern int func_80071A54();
extern int func_800D3FD8();
extern void func_8006DCE4(int, int, int, int, int);
extern int func_80077CF4(int);
extern int func_80077DC4(int);
extern void func_800783E4();
extern void func_800CFB7C(void *, int, void *);
extern void func_800CFAA8(void *, void *, void *);
extern int func_800C6B90(void *, int);
extern void func_800D2B58();
extern int func_80077A64(int, int, int, int);
extern int func_80077AA4(int, int);
extern void func_800C6EC0(int, int);
extern void func_800C6ED8(int);
extern void func_80079754(void *, MATRIX *);
extern void func_80078CC4(MATRIX *, VECTOR *);
extern void func_800C6EF8(void *);
extern void func_800C6FA0(void *, int);
extern void func_800C71E4(void *, MATRIX *);
extern void func_800C6F4C(void *);
extern void func_800CEE20();

int func_8018F750(int mode, short *p, Slot *d)
{
    SVECTOR rot;
    SVECTOR ang;
    CVECTOR col0;
    CVECTOR col1;
    CVECTOR col2;
    CVECTOR col3;
    MATRIX mat;
    VECTOR scale;
    short *e;
    unsigned char *q;
    unsigned char *chan;
    int *w;
    int **matrix_slot;
    CVECTOR *col;
    int t;
    int u;
    int i;
    int k;
    int sc;
    int clut;
    int pal;
    int value;

    col0 = D_8018EFFC;
    col1 = D_8018F000;
    col2 = D_8018F004;
    col3 = D_8018F008;
    switch (mode) {
    case 0:
        p[36] = 0;
        p[37] = 0;
        p[38] = 128;
        d->x14 = 0;
        p[12] = *(short *)(*(unsigned char **)(D_800F32D0 + 8) + 616);
        p[13] = *(short *)(*(unsigned char **)(D_800F32D0 + 8) + 618);
        p[14] = *(short *)(*(unsigned char **)(D_800F32D0 + 8) + 620);
        p[16] = p[12];
        p[17] = p[13];
        p[18] = p[14];
        p[0] = p[12];
        p[1] = p[13];
        p[2] = p[14];
        D_8019085C = func_8006E498(D_800B0E64[0], 0xC5543704);
        func_800C6D5C(D_8019085C, 0, 0);
        func_800D1384(p, p, 1006, &col0, &col1, 128, p + 42, 1);
        return func_800CE560(*(void **)(D_800F33E0 + 8), 20, 32, func_8018F028);
    case 1:
        switch (p[36]) {
        case 0:
            d->x14 = 0;
            func_800CE870(D_8009D254, 0, &rot);
            rot.vx = (func_80071A54() & 0x7FF) - 0x400;
            rot.vy = D_800942EC - 600 - (func_80071A54() & 0x1FF);
            rot.vz = (func_80071A54() & 0x7FF) - 0x400;
            p[12] = rot.vx;
            p[13] = rot.vy;
            p[14] = rot.vz;
            func_800CE870(D_8009D254, 1, p + 8);
            p[40] = (func_80071A54() & 0xF) + 6;
            p[37] = 0;
            p[36] = 1;
            break;
        case 1:
            if (++p[37] == 1 && (func_80071A54() & 1)) {
                func_8006DCE4(0x5FF, func_800D3FD8(), p[0], p[1], p[2]);
            }
            t = func_80077CF4((p[37] << 10) / p[40]);
            func_800783E4(p + 16, p + 12, 0x1000 - t, t, p);
            func_800CE870(D_8009D254, 1, p + 8);
            if (p[37] >= p[40]) {
                p[36] = 2;
                p[37] = 0;
            }
            break;
        case 2:
            p[37]++;
            func_800CE870(D_8009D254, 1, p + 8);
            if (p[37] >= 4) {
                p[36] = 0;
                p[37] = 0;
                p[16] = p[0];
                p[17] = p[1];
                p[18] = p[2];
                if (d->x14) {
                    d->x14 = 0;
                    p[36] = 3;
                    p[12] = d->x8;
                    p[13] = d->xA;
                    p[14] = d->xC;
                    p[20] = d->x0;
                    p[21] = d->x2;
                    p[22] = d->x4;
                    p[40] = d->x10;
                    p[24] = p[8];
                    p[25] = p[9];
                    p[26] = p[10];
                }
            }
            break;
        case 3:
            if (++p[37] == 1) {
                func_8006DCE4(0x5FF, func_800D3FD8(), p[0], p[1], p[2]);
            }
            t = func_80077CF4((p[37] << 10) / p[40]);
            u = 0x1000 - t;
            func_800783E4(p + 16, p + 12, u, t, p);
            func_800783E4(p + 24, p + 20, u, t, p + 8);
            if (p[37] >= p[40]) {
                p[36] = 4;
                p[37] = 0;
                p[16] = p[0];
                p[17] = p[1];
                p[18] = p[2];
            }
            break;
        case 4:
            if (++p[37] == 1) {
                e = func_800CE610(*(void **)(D_800F33E0 + 8));
                if (e) {
                    e[4] = p[4];
                    e[5] = p[5];
                    e[6] = p[6];
                    func_800CFB7C(p + 4, 140, e);
                    e[0] += p[0];
                    e[1] += p[1];
                    e[2] += p[2];
                    e[8] = 0;
                    e[9] = 0;
                }
                func_8006DCE4(0x600, 128, p[0], p[1], p[2]);
            }
            if (p[37] >= 4) {
                p[36] = 5;
                p[37] = 0;
                e = func_800CE610(*(void **)(D_800F33E0 + 8));
                if (e) {
                    e[0] = p[8];
                    e[1] = p[9];
                    e[2] = p[10];
                    e[4] = p[4];
                    e[5] = p[5];
                    e[6] = p[6];
                    e[8] = 1;
                    e[9] = 0;
                }
                for (i = 0; i < 12; i++) {
                    e = func_800CE610(*(void **)(D_800F33E0 + 8));
                    if (e) {
                        e[0] = p[8];
                        e[1] = p[9];
                        e[2] = p[10];
                        e[4] = func_80071A54() % 70 - 35;
                        e[5] = -(func_80071A54() % 70);
                        e[6] = func_80071A54() % 70 - 35;
                        e[8] = 2;
                        e[9] = 0;
                    }
                }
            }
            break;
        case 5:
            p[37]++;
            if (func_800C6B90(p + 8, 66) && D_80190800 == 0) {
                q = **(unsigned char ***)(D_800F32D0 + 8);
                (q + ((*(unsigned int *)q >> 17) & 0x70))[28] = 2;
                if (D_800E2368[0xD]) {
                    chan = D_800F32D0;
                    if ((***(int ***)(chan + 8) & 0x3F000000) == 0x01000000) {
                        *(int *)(*D_8009D254 + 0x4C) |= 0x4000;
                        w = **(int ***)(chan + 8);
                        *w = (*w & 0xC0FFFFFF) | 0x2F000000;
                        w = **(int ***)(chan + 8);
                        *w |= 0x80000000;
                    }
                }
                D_80190800 = 116;
            }
            if (p[37] >= 8) {
                p[36] = 0;
                p[37] = 0;
            }
            break;
        case 6:
            p[37]++;
            p[38] = 96 - p[37] * 3;
            if (p[37] >= 32) return 1;
            break;
        }
        if (p[36] != 6) {
            p[38] = func_80077CF4(D_800E27EC << 5) / 128 + 128;
        }
        func_800CFAA8(p, p + 8, p + 4);
        func_800CFB7C(p + 4, 140, p + 28);
        p[28] += p[0];
        p[29] += p[1];
        p[30] += p[2];
        func_800CFB7C(p + 4, -140, p + 32);
        p[32] += p[0];
        p[33] += p[1];
        p[34] += p[2];
        if (*(int *)(**(unsigned char ***)(D_800F32D0 + 8) + 16) < 500000 && p[36] != 6) {
            p[36] = 6;
            p[37] = 0;
        }
        if (D_80190800 > 0) {
            D_80190800--;
        } else {
            D_80190800 = 0;
        }
        break;
    case 2:
        matrix_slot = &D_800BCFA4;
        asm volatile("" : "=r"(matrix_slot) : "0"(matrix_slot));
        gte_SetRotMatrix(*matrix_slot);
        D_800F3368.f0 = 32;
        D_800F336A = 2;
        D_800F3376 = 32;
        D_800F3378 = 32;
        value = D_800E2850[D_800E11E4[3]];
        asm volatile("" : : : "memory");
        D_800F336C = 3;
        D_800F336E = 0;
        D_800F3372 = 0;
        D_800F3374 = 4;
        D_800F3370 = value;
        gte_SetRotMatrix(*matrix_slot);
        asm volatile("" : : "r"(matrix_slot));
        col = (D_800E27EC & 1) ? &col2 : &col3;
        switch (p[36]) {
        case 4:
            t = p[37] << 10;
            func_800783E4(p + 28, p + 8, 0x1000 - t, t, &rot);
            func_800D2B58(p + 28, &rot, col, col, 128, 128, 1);
            break;
        case 5:
            k = func_80077DC4(p[37] << 7) / 32;
            func_800D2B58(p + 28, p + 8, col, col, k / 2, k, 1);
            break;
        }
        if (p[38] != 0) {
            t = 1;
            if (p[38] >= 96) t = 255;
            value = D_800E2850[D_800E11E4[11]];
            D_800F3368.f4 = 3;
            D_800F3368.f6 = 1;
            D_800F3370 = value;
            clut = (D_800E2850[D_800E11E4[11]] | func_80077A64(0, t, 0, 0)) & 0xFFFF;
            sc = 0x400;
            func_800C6EC0(clut, func_80077AA4(0, D_800E1204[D_800F336C] + ((D_800F336C == 4 && D_800F3428) ? 6 : 2)) & 0xFFFF);
            if (t != 255) func_800C6ED8(1); else func_800C6ED8(0);
            ang.vx = p[4];
            ang.vy = p[5];
            ang.vz = p[6];
            ang.vx += 0x800;
            ang.vz = func_80077CF4(D_800E27EC * 96) / 8 + 0x800;
            func_80079754(p + 4, &mat);
            mat.t[0] = p[0];
            mat.t[1] = p[1];
            mat.t[2] = p[2];
            scale.vx = sc;
            scale.vy = sc;
            scale.vz = sc;
            func_80078CC4(&mat, &scale);
            func_800C6EF8(D_8019085C);
            func_800C6FA0(D_8019085C, (unsigned short)p[38]);
            func_800C71E4(D_8019085C, &mat);
            func_800C6F4C(D_8019085C);
            func_800D1384(p + 28, p + 32, 6, &col0, &col1, p[38], p + 42, 1);
            D_800F3368.fc = 51;
            k = p[38];
            if (D_800E27EC & 1) k = k * 3 / 4;
            value = D_800E2850[D_800E11E4[11]];
            D_800F3368.f4 = 3;
            D_800F3368.f6 = 1;
            D_800F3370 = value;
            func_800CEE20(p, 0, 0x2000, 0x2000, 6, func_80077AA4(0, D_800E1204[D_800F3368.f4] + 5) & 0xFFFF, 3, k, &col0);
        }
        D_800F3374 = 11;
        break;
    }
    return 0;
}
