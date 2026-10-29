/* room_m0430i (PE.IMG room m0430i chunk 2, VRAM 0x8018EFE8)
 * func_8018F028 — blob offset 0x40, 0x728 bytes. Default profile
 * (era_o2_g0, -O2 -G0); LINK_EXACT at the room VMA (2026-10-08, written from
 * the retail asm). Was the m0430i story 0x90/0x140 CPU_BOUNDARY stop. */

/* Particle task callback (spawned by func_8018F750). mode 1 advances the
 * particle (fade counter p[9]; type 2 also drifts with drag and a floor
 * bounce against D_800942EC); mode 2 draws it: type 0 is a spinning sprite
 * plus a trail burst, type 1 two glow sprites (sprite params D_800F3368),
 * type 2 a single GTE-projected flash.
 * Matching notes (gcc 2.7.2 scheduler/allocator effects):
 *  - type 1 points `sp`/`ix` at D_800F3368 / D_800E11E4[3] and reads
 *    D_800F3368.f2 / D_800E11E4[11] through the symbols, so CSE's
 *    related-value lookup rewrites them as 2($16) / 16($21); the absolute
 *    stores go to the separately named halves (D_800F336A etc.);
 *  - the do {} while (0) loop notes are scheduling barriers (and stop
 *    combine folding the pointers back to absolute addresses);
 *  - the rot stores are written *(short *)&rot.f so they are not
 *    MEM_IN_STRUCT, which lets the p[9] load schedule above them;
 *  - the empty asm on `x` gives it a second set, so sched1 does not treat
 *    the D_800E120A load as a birthing insn and sink it;
 *  - `ix` is pinned to $21 (retail's choice; the 3 constant takes $22). */

#define gte_SetRotMatrix(r0) __asm__ volatile(                       \
    "lw $12, 0(%0);" "lw $13, 4(%0);" "ctc2 $12, $0;" "ctc2 $13, $1;"   \
    "lw $12, 8(%0);" "lw $13, 12(%0);" "lw $14, 16(%0);"                \
    "ctc2 $12, $2;" "ctc2 $13, $3;" "ctc2 $14, $4;"                     \
    "lw $12, 20(%0);" "lw $13, 24(%0);" "ctc2 $12, $5;"                 \
    "lw $14, 28(%0);" "ctc2 $13, $6;" "ctc2 $14, $7"                    \
    : : "r"(r0) : "$12", "$13", "$14")

typedef struct { short vx, vy, vz, pad; } SVECTOR;
typedef struct { unsigned char r, g, b, cd; } CVECTOR;
extern CVECTOR D_8018EFF4;
extern CVECTOR D_8018EFF8;
extern SVECTOR D_80190804;
extern short D_800942EC[];
extern int *D_800BCFA4;
extern unsigned short D_800E11E4[];
extern unsigned short D_800E1204[];
extern unsigned short D_800E120A;
extern unsigned short D_800E2850[];
typedef struct { short f0; short f2; unsigned short f4; short f6; unsigned short f8; short fa; short fc; short fe; short f10; } SPRP;
extern SPRP D_800F3368;
extern short D_800F336A;
extern unsigned short D_800F336C;
extern short D_800F336E;
extern unsigned short D_800F3370;
extern short D_800F3376;
extern short D_800F3378;
extern int D_800F3428;
extern int func_80077CF4(int);
extern int func_80077DC4(int);
extern int func_80077AA4(int, int);
extern void func_800CF3AC(void *, void *, int);
extern void func_800CEE20();
extern void func_800D2370();
extern void func_800D2104(void *, void *, int, int);

int func_8018F028(int mode, short *p)
{
    SVECTOR rot;
    SVECTOR pos;
    CVECTOR col0;
    CVECTOR col1;
    int **matrix_slot;
    int **ms2;
    int s;
    int a;
    int b;
    int c;
    int d, e, x;
    SPRP *sp;
    register unsigned short *ix asm("$21");
    unsigned short *tb;
    int k;
    int value;
    int v1;
    int i0;

    col0 = D_8018EFF4;
    col1 = D_8018EFF8;
    switch (mode) {
    case 1:
        switch (p[8]) {
        case 0:
            if (++p[9] >= 16) return 1;
            break;
        case 1:
            if (++p[9] >= 16) return 1;
            break;
        case 2:
            p[9]++;
            p[0] += p[4];
            p[1] += p[5];
            p[2] += p[6];
            p[4] = (p[4] * 59) / 60;
            p[6] = (p[6] * 59) / 60;
            p[5] = p[5] + 1;
            if (p[1] >= D_800942EC[0]) p[5] *= -1;
            if (p[9] >= 20) return 1;
            break;
        }
        break;
    case 2:
        switch (b = p[8]) {
        case 0:
            D_800F3368.f0 = 32;
            D_800F3368.f2 = 2;
            D_800F3368.fe = 32;
            D_800F3368.f10 = 32;
            value = D_800E2850[D_800E11E4[11]];
            D_800F3368.f4 = 3;
            D_800F3368.f6 = 1;
            D_800F3368.f8 = value;
            a = p[9] << 6;
            b = func_80077CF4(a);
            rot.vx = p[4];
            rot.vy = p[5];
            rot.vz = p[6];
            rot.pad = 1;
            matrix_slot = &D_800BCFA4;
            asm volatile("" : "=r"(matrix_slot) : "0"(matrix_slot));
            gte_SetRotMatrix(*matrix_slot);
            asm volatile("" : : "r"(matrix_slot));
            func_800CF3AC(&D_80190804, &pos, p[9]);
            func_800CEE20(p, &rot, b * 3, b * 3, 4,
                          func_80077AA4(0, D_800E1204[D_800F336C] + ((D_800F336C == 4 && D_800F3428) ? 8 : 4)) & 0xFFFF,
                          1, 128, &pos);
            if (p[9] < 8) {
                int t;
                asm volatile("" : "=r"(t) : "0"(a));
                k = 64;
                b = (p[9] / 2) << 5;
                c = func_80077DC4(t) / 32;
                func_800CF3AC(&D_80190804, &pos, p[9] * 2);
                func_800D2370(p, &rot, 500, 360, b, k, 31, 31,
                              func_80077AA4(0, D_800E1204[D_800F336C] + ((D_800F336C == 4 && D_800F3428) ? 7 : 3)) & 0xFFFF,
                              &col0, &col0, (short)c, 1);
            }
            break;
        case 1:
            do { } while (0);
            *(short *)&rot.vx = 0x400;
            sp = &D_800F3368;
            d = p[9];
            *(short *)&rot.vy = 0;
            e = p[9];
            *(short *)&rot.pad = 1;
            ix = &D_800E11E4[3];
            sp->f0 = 32;
            D_800F336A = 2;
            D_800F3376 = 32;
            x = D_800E120A;
            D_800F3378 = 32;
            do { } while (0);
            i0 = ix[0];
            D_800F336C = 3;
            asm("" : "=r"(x) : "0"(x));
            *(short *)&rot.vz = e << 5;
            D_800F336E = 0;
            v1 = D_800E2850[i0];
            D_800F3370 = v1;
            a = d << 6;
            func_800CEE20(p, 0, 0x2000, 0x2000, D_800F3368.f2 * (p[9] / 2) + 96,
                          func_80077AA4(0, x) & 0xFFFF, 1, 128, 0);
            b = func_80077DC4(a) / 2 + 0x800;
            c = func_80077CF4(d << 7) / 32;
            tb = D_800E2850;
            v1 = tb[D_800E11E4[11]];
            D_800F3368.f4 = 3;
            D_800F3368.f6 = 1;
            D_800F3368.f8 = v1;
            func_800CEE20(p, 0, b * 4, b * 4, 6, func_80077AA4(0, D_800E120A + 5) & 0xFFFF, 3, c, &col1);
            break;
        case 2:
            ms2 = &D_800BCFA4;
            asm volatile("" : "=r"(ms2) : "0"(ms2));
            gte_SetRotMatrix(*ms2);
            asm volatile("" : : "r"(ms2));
            func_800CF3AC(&D_80190804, &pos, (p[9] << 4) / 20);
            func_800D2104(p, &pos, 128, 1);
            break;
        }
        break;
    }
    return 0;
}
