/* room_m0429i — func_8019A054, blob offset 0xB06C, 0xBD8 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (2026-10-08, written from the retail asm).
 * Effect callback (called through func_800D413C after func_80199684). Mode 0
 * opens a 20-entry particle pool with callback D_80199C28. Mode 1 counts 34
 * frames; on frame 15 it bursts 16 pairs of particles from the stored
 * position (a ring of 16 directions plus random scatter); returns 2 when
 * done. Mode 2 loads the camera rotation into the GTE, takes two bone points
 * of the current actor and sets the sprite globals; frames 0-13 draw two
 * pulsing double flashes at the bone points, frames 14-34 store the first
 * point on the floor and draw fading flashes, a ring (func_800D0728) and a
 * floor flash, then restore the 32x32 sprite globals.
 * Matching notes: sprite globals D_800F3368.. as one array (retail keeps the
 * array base in $s2 across the header and the frame 14-34 branch); the
 * first two header stores and the index read are volatile, the 3 is loaded
 * into a variable before the table read, and two memory fences keep the
 * header order (all checked against the retail schedule); the GTE matrix
 * load follows the vendor template (room_m0432i func_80192AF0); the first
 * zero store of mode 0 goes through an in-struct view (gcc 2.7 alias rule);
 * one variable serves as the angle (mode 1) and the 14-34 frame count. */
typedef struct { short vx, vy, vz, pad; } SVECTOR;
typedef struct { unsigned char r, g, b, cd; } CVECTOR;

#define gte_SetRotMatrix(r0) __asm__ volatile(                       \
    "lw $12, 0(%0);" "lw $13, 4(%0);" "ctc2 $12, $0;" "ctc2 $13, $1;"   \
    "lw $12, 8(%0);" "lw $13, 12(%0);" "lw $14, 16(%0);"                \
    "ctc2 $12, $2;" "ctc2 $13, $3;" "ctc2 $14, $4;"                     \
    "lw $12, 20(%0);" "lw $13, 24(%0);" "ctc2 $12, $5;"                 \
    "lw $14, 28(%0);" "ctc2 $13, $6;" "ctc2 $14, $7"                    \
    : : "r"(r0) : "$12", "$13", "$14")

extern int *D_800F32D0;
extern int *D_800F33E0;
extern int *D_800BCFA4;
extern short D_800942EC[];
extern unsigned short D_800F3368[];
extern short D_800F336E;
extern unsigned short D_800F3370;
extern short D_800F3372;
extern short D_800F3374;
extern int D_800F3428;
extern volatile unsigned short D_800E11EA;
extern unsigned short D_800E1204[];
extern unsigned short D_800E2850[];
extern SVECTOR D_8018F1D4;
extern SVECTOR D_8018F264;
extern CVECTOR D_8018F26C;
extern int D_80199C28;
extern int func_80071A54();
extern int func_80077AA4();
extern int func_80077CF4();
extern int func_80077DC4();
extern int func_800CE560();
extern int func_800CE610();
extern void func_800CE8F0();
extern void func_800CEE20();
extern void func_800D0728();

typedef struct { short v[100]; } EFF;

int func_8019A054(int mode, short *p)
{
    SVECTOR a;
    SVECTOR b;
    SVECTOR sv0 = D_8018F1D4;
    SVECTOR sv1 = D_8018F264;
    CVECTOR col = D_8018F26C;
    SVECTOR pos;
    SVECTOR rot;
    short *q;
    int i;
    int size;
    int alpha;
    int k;
    int clut;
    int **matrix_slot;

    switch (mode) {
    case 0:
        p[2] = 6;
        p[3] = 18;
        ((EFF *)p)->v[0] = 0;
        p[1] = 0;
        return func_800CE560(D_800F33E0[2], 20, 32, &D_80199C28);
    case 1:
        p[1]++;
        if (p[1] == 15) {
            for (i = 0; i < 16; i++) {
                k = i << 8;
                q = (short *)func_800CE610(D_800F33E0[2]);
                if (q != 0) {
                    q[0] = p[4];
                    q[1] = p[5];
                    q[2] = p[6];
                    q[0] += (func_80071A54() & 0xFF) - 128;
                    q[2] += (func_80071A54() & 0xFF) - 128;
                    q[4] = func_80077DC4(k) * 60 / 4096;
                    q[6] = func_80077CF4(k) * 60 / 4096;
                    q[5] = -(func_80071A54() & 0xF) - 8;
                    q[8] = 0;
                    q[9] = 0;
                }
                q = (short *)func_800CE610(D_800F33E0[2]);
                if (q != 0) {
                    q[0] = p[4];
                    q[1] = p[5];
                    q[2] = p[6];
                    q[4] = func_80071A54() % 49 - 24;
                    q[5] = func_80071A54() % 128 - 64;
                    q[6] = func_80071A54() % 49 - 24;
                    q[8] = 1;
                    q[9] = 0;
                }
            }
        }
        if (p[1] >= 34)
            return 2;
        break;
    case 2:
        matrix_slot = &D_800BCFA4;
        asm volatile("" : "=r"(matrix_slot) : "0"(matrix_slot));
        gte_SetRotMatrix(*matrix_slot);
        asm volatile("" : : "r"(matrix_slot));
        func_800CE8F0(D_800F32D0[2], p[2], &sv1, &a);
        func_800CE8F0(D_800F32D0[2], p[3], &sv1, &b);
    {
        int value;
        int index;
        int three;
        *(volatile unsigned short *)&D_800F3368[0] = 64;
        index = D_800E11EA;
        *(volatile unsigned short *)&D_800F3368[1] = 4;
        three = 3;
        D_800F3368[7] = 64;
        D_800F3368[8] = 64;
        value = D_800E2850[index];
        asm volatile("" : : "r"(value) : "memory");
        D_800F3368[2] = three;
        D_800F336E = 0;
        D_800F3372 = 0;
        D_800F3374 = 32;
        D_800F3370 = value;
        asm volatile("" : : : "memory");
    }
        if (p[1] < 14) {
            size = func_80077CF4(p[1] << 5) / 2 + 2048;
            if (p[1] < 4)
                alpha = p[1] << 5;
            else
                alpha = (p[1] & 1) * 64 + 128;
            D_800F3368[0] = 32; D_800F3368[1] = 2; D_800F3368[7] = 32; D_800F3368[8] = 32;
            clut = D_800E1204[D_800F3368[2]];
            if (D_800F3368[2] == 4 && D_800F3428 != 0)
                clut += 4;
            func_800CEE20(&a, 0, size * 2 / 3, size * 2 / 3, 152, func_80077AA4(16, clut) & 0xFFFF, 1, alpha, 0);
            D_800F3368[0] = 64; D_800F3368[1] = 4; D_800F3368[7] = 64; D_800F3368[8] = 64;
            clut = D_800E1204[D_800F3368[2]];
            if (D_800F3368[2] == 4 && D_800F3428 != 0)
                clut += 4;
            func_800CEE20(&a, 0, size, size, 8, func_80077AA4(16, clut) & 0xFFFF, 3, alpha, 0);
            D_800F3368[0] = 32; D_800F3368[1] = 2; D_800F3368[7] = 32; D_800F3368[8] = 32;
            clut = D_800E1204[D_800F3368[2]];
            if (D_800F3368[2] == 4 && D_800F3428 != 0)
                clut += 4;
            func_800CEE20(&b, 0, size * 2 / 3, size * 2 / 3, 152, func_80077AA4(16, clut) & 0xFFFF, 1, alpha, 0);
            D_800F3368[0] = 64; D_800F3368[1] = 4; D_800F3368[7] = 64; D_800F3368[8] = 64;
            clut = D_800E1204[D_800F3368[2]];
            if (D_800F3368[2] == 4 && D_800F3428 != 0)
                clut += 4;
            func_800CEE20(&b, 0, size, size, 8, func_80077AA4(16, clut) & 0xFFFF, 3, alpha, 0);
        } else if (p[1] < 35) {
            if (p[1] == 14) {
                p[4] = a.vx;
                p[5] = a.vy;
                p[6] = a.vz;
                p[5] = D_800942EC[0];
            }
            k = p[1] - 14;
            size = func_80077CF4((k << 10) / 20) + 4096;
            D_800F3368[0] = 32; D_800F3368[1] = 2; D_800F3368[7] = 32; D_800F3368[8] = 32;
            clut = D_800E1204[D_800F3368[2]];
            alpha = 128 - (k << 7) / 20;
            if (D_800F3368[2] == 4 && D_800F3428 != 0)
                clut += 4;
            func_800CEE20(&a, 0, size * 2 / 3, size * 2 / 3, 152, func_80077AA4(16, clut) & 0xFFFF, 1, alpha, 0);
            D_800F3368[0] = 64; D_800F3368[1] = 4; D_800F3368[7] = 64; D_800F3368[8] = 64;
            clut = D_800E1204[D_800F3368[2]];
            if (D_800F3368[2] == 4 && D_800F3428 != 0)
                clut += 4;
            func_800CEE20(&a, 0, size, size, 8, func_80077AA4(16, clut) & 0xFFFF, 3, alpha, 0);
            D_800F3368[0] = 32; D_800F3368[1] = 2; D_800F3368[7] = 32; D_800F3368[8] = 32;
            clut = D_800E1204[D_800F3368[2]];
            if (D_800F3368[2] == 4 && D_800F3428 != 0)
                clut += 4;
            func_800CEE20(&b, 0, size * 2 / 3, size * 2 / 3, 152, func_80077AA4(16, clut) & 0xFFFF, 1, alpha, 0);
            D_800F3368[0] = 64; D_800F3368[1] = 4; D_800F3368[7] = 64; D_800F3368[8] = 64;
            clut = D_800E1204[D_800F3368[2]];
            if (D_800F3368[2] == 4 && D_800F3428 != 0)
                clut += 4;
            func_800CEE20(&b, 0, size, size, 8, func_80077AA4(16, clut) & 0xFFFF, 3, alpha, 0);
            func_800D0728(p + 4, 800, 400, 20, &sv0, size, size, &col, 0, alpha / 2, 1);
            size = func_80077CF4(k * 1248 / 20 + 800) + 2048;
            rot.vx = 1024;
            rot.vy = 0;
            rot.vz = 0;
            rot.pad = 1;
            pos.vx = p[4];
            pos.vz = p[6];
            pos.vy = D_800942EC[0];
            clut = D_800E1204[D_800F3368[2]];
            if (D_800F3368[2] == 4 && D_800F3428 != 0)
                clut += 4;
            func_800CEE20(&pos, &rot, size, size, 8, func_80077AA4(16, clut) & 0xFFFF, 1, alpha * 2 / 3, 0);
        }
        D_800F3368[0] = 32;
        D_800F3368[1] = 2;
        D_800F3368[7] = 32;
        D_800F3368[8] = 32;
        D_800F3372 = 0;
        break;
    }
    return 0;
}
