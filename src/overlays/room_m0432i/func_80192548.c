/* room_m0432i — func_80192548, blob offset 0x3560, 0x5A8 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (2026-10-08, written from the retail asm).
 * Particle task started by the effect callback func_80192AF0 (pool of
 * 0x14-byte particles, kind at +0x10, frame at +0x12). Mode 1 steps a
 * particle: kind 0 grows for 24 frames, kind 1 drifts with 127/128 damping
 * and bounces off the floor (D_800942EC) for 16 frames, kind 2 grows for 8.
 * Mode 2 draws it: kind 0 a pulsing sprite through func_800CFB7C /
 * func_800CF3AC / func_800D2B58, kind 1 a rotating sprite through
 * func_800CEE20, kind 2 a ring through func_800D0E88 (camera RT first).
 * Matching notes: `p[9]++` first in kind 1; the floor and the four
 * D_800F33xx sprite globals declared as arrays (keeps the stores before the
 * loads, as retail); the colour pointer `cp` held from entry; `s` shared by
 * kind-1 and kind-2 draws; the products reassigned to the temp `g`; the
 * matrix_slot barrier pair only in the kind-2 draw (kind 0 keeps the
 * trailing "r" barrier only); mode 1 falls out of the outer switch. */
typedef struct { short vx, vy, vz, pad; } SVECTOR;
typedef struct { unsigned char r, g, b, cd; } CVECTOR;

#define gte_SetRotMatrix(r0) __asm__ volatile(                       \
    "lw $12, 0(%0);" "lw $13, 4(%0);" "ctc2 $12, $0;" "ctc2 $13, $1;"   \
    "lw $12, 8(%0);" "lw $13, 12(%0);" "lw $14, 16(%0);"                \
    "ctc2 $12, $2;" "ctc2 $13, $3;" "ctc2 $14, $4;"                     \
    "lw $12, 20(%0);" "lw $13, 24(%0);" "ctc2 $12, $5;"                 \
    "lw $14, 28(%0);" "ctc2 $13, $6;" "ctc2 $14, $7"                    \
    : : "r"(r0) : "$12", "$13", "$14")

extern CVECTOR D_8018F1CC;
extern int *D_800BCFA4;
extern short D_800942EC[];
extern unsigned short D_800E1204[];
extern short D_800F3368[];
extern short D_800F336A[];
extern unsigned short D_800F336C;
extern short D_800F3376[];
extern short D_800F3378[];
extern int D_800F3428;
extern SVECTOR D_801994EC;
extern int D_80199168;
extern int func_80077AA4();
extern int func_80077CF4();
extern int func_80077DC4();
extern void func_800CEE20();
extern void func_800CF3AC();
extern void func_800CFB7C();
extern void func_800D0E88();
extern void func_800D2B58();

int func_80192548(int mode, short *p)
{
    SVECTOR pos;
    SVECTOR ang;
    SVECTOR out;
    CVECTOR c = D_8018F1CC;
    int **matrix_slot;
    int **matrix_slot2;
    CVECTOR *cp;
    int s;
    int r;
    int g;
    int k;
    int clut;

    cp = &c;
    switch (mode) {
    case 1:
        switch (p[8]) {
        case 0:
            p[9]++;
            p[4] += 4;
            p[5] += 0x18;
            if (p[9] >= 0x18) return 1;
            break;
        case 1:
            p[9]++;
            p[0] += p[4];
            p[1] += p[5];
            p[2] += p[6];
            p[4] = p[4] * 127 / 128;
            p[6] = p[6] * 127 / 128;
            p[5]++;
            if (p[1] >= D_800942EC[0]) p[5] *= -1;
            if (p[9] >= 0x10) return 1;
            break;
        case 2:
            p[9]++;
            p[4] += 2;
            if (p[9] >= 8) return 1;
            break;
        }
        break;
    case 2:
        switch (p[8]) {
        case 0:
            g = (func_80077CF4((p[9] << 10) / 24) + 0x1000) / 2;
            g = p[3] * g / 4096;
            func_800CFB7C(p + 4, (short)g, &pos);
            pos.vx += D_801994EC.vx;
            pos.vy += D_801994EC.vy;
            pos.vz += D_801994EC.vz;
            matrix_slot = &D_800BCFA4;
            gte_SetRotMatrix(*matrix_slot);
            asm volatile("" : : "r"(matrix_slot));
            func_800CF3AC(&D_80199168, &out, p[9]);
            func_800D2B58(&D_801994EC, &pos, &out, 0, 0x80, 0, 1);
            return 0;
        case 1:
            r = func_80077CF4(p[9] << 6) + 0x800;
            k = func_80077DC4(p[9] * 0x4B4 / 16) / 128 + 0x28;
            D_800F3368[0] = 0x20;
            D_800F336A[0] = 2;
            D_800F3376[0] = 0x20;
            D_800F3378[0] = 0x20;
            ang.vx = 0;
            ang.vy = 0;
            ang.vz = p[9] * 32;
            ang.pad = 0;
            clut = D_800E1204[D_800F336C];
            if (D_800F336C == 4 && D_800F3428 != 0) clut += 4;
            func_800CEE20(p, &ang, r, r, D_800F336A[0] * (p[9] / 2) + 0x80,
                          func_80077AA4(0, clut) & 0xFFFF, 1, k, 0);
            return 0;
        case 2:
            k = func_80077DC4(p[9] << 7) / 32;
            g = func_80077CF4(p[9] << 7) / 4 + 0xC00;
            g = p[3] * g / 4096;
            s = g;
            r = func_80077DC4(p[9] << 7) * 240 / 4096;
            matrix_slot2 = &D_800BCFA4;
            asm volatile("" : "=r"(matrix_slot2) : "0"(matrix_slot2));
            gte_SetRotMatrix(*matrix_slot2);
            asm volatile("" : : "r"(matrix_slot2));
            func_800D0E88(&D_801994EC, p + 4, s, r, cp, 0, 0, (short)k, 1);
            return 0;
        }
        return 0;
    }
    return 0;
}
