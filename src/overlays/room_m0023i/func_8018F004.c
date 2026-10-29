/* room_m0023i (PE.IMG room m0023i chunk 2, VRAM 0x8018EFE8)
 * func_8018F004 — blob offset 0x1C, 0x3C4 bytes. Default profile era_o2_g0.
 * Eve room particle callback: cmd 1 advances the particle by its velocity (two
 * phases, done at D_800E27EC >= 4 / >= 8), cmd 2 renders it (phase 0: a
 * func_800D2370 sprite plus a func_800CEE20 glow; phase 1: one glow scaled by
 * func_80077DC4). The texture base is D_800E1204[D_800F336C], +4 for entry 4
 * while D_800F3428 is set.
 * Levers:
 *   - phase-0 statement order col, k, x, y, z, then the table read, then s;
 *   - the first table read is wrapped in `do { } while (0)`: the block boundary
 *     keeps cc1's sched1 from pulling the D_800E27EC load in front of it, so
 *     the index lands in $v0 (without it, 8 words differ);
 *   - k is an int (an unsigned short local adds an andi), reloaded for the
 *     second lookup, with the b stores ordered x, y, z, pad;
 *   - the glow size is (D_800F336A << 1) * (D_800E27EC - 1), not
 *     D_800F336A * 2 * ..., which cc1 reassociates;
 *   - phase 1 keeps the func_80077DC4() / 32 result in its own variable (s2),
 *     which puts the divide temp in $a0 so its sra lands in the bne delay slot.
 * inline GTE/asm: uses Psy-Q inline_c.h-shaped GTE (COP2) asm macros — count separately
 * from plain C.
 * Hand-written in this repo (khasinski's vendored tree has this body as asm only). */

#define gte_SetRotMatrix(r0) __asm__ volatile ( \
    "lw $12,0(%0)\n\tlw $13,4(%0)\n\tctc2 $12,$0\n\tctc2 $13,$1\n\t" \
    "lw $12,8(%0)\n\tlw $13,12(%0)\n\tlw $14,16(%0)\n\t" \
    "ctc2 $12,$2\n\tctc2 $13,$3\n\tctc2 $14,$4" : : "r"(r0) : "$12", "$13", "$14")
#define gte_SetTransMatrix(r0) __asm__ volatile ( \
    "lw $12,20(%0)\n\tlw $13,24(%0)\n\tctc2 $12,$5\n\tlw $14,28(%0)\n\t" \
    "ctc2 $13,$6\n\tctc2 $14,$7" : : "r"(r0) : "$12", "$13", "$14")
typedef struct { short vx, vy, vz, pad; } SVECTOR;
typedef struct { short m[3][3]; int t[3]; } MATRIX;
typedef struct { short x, y, z, flag, vx, vy, vz; } Particle;

extern MATRIX *D_800BCFA4[];
extern int D_800E27EC;
extern unsigned short D_800E1204[];
extern unsigned short D_800F336C;
extern short D_800F336A;
extern int D_800F3428;
extern char D_80190758[];
extern int func_80077AA4();
extern int func_80077DC4();
extern void func_800D2370();
extern void func_800CEE20();

int func_8018F004(int cmd, Particle *p)
{
    SVECTOR a;
    SVECTOR b;
    int col;
    int t;
    int s;
    int s2;
    int k;

    switch (cmd) {
    case 1:
        switch (p->flag) {
        case 0:
            p->x += p->vx;
            p->y += p->vy;
            p->z += p->vz;
            if (D_800E27EC >= 4) {
                return 1;
            }
        case 1:
            p->x += p->vx;
            p->y += p->vy;
            p->z += p->vz;
            if (D_800E27EC >= 8) {
                return 1;
            }
            break;
        default:
            return 0;
        }
        break;
    case 2:
        gte_SetRotMatrix(D_800BCFA4[0]);
        gte_SetTransMatrix(D_800BCFA4[0]);
        switch (p->flag) {
        case 0:
            col = 0x808080;
            k = D_800F336C;
            a.vx = p->x;
            a.vy = p->y;
            a.vz = p->z;
            do { t = D_800E1204[k]; } while (0);
            s = (D_800E27EC - 1) * 16;
            if (k == 4 && D_800F3428 != 0) {
                t += 4;
            }
            func_800D2370(D_80190758, p, 0x3C0, 0x64, 0, s, 0xFF, 0xF,
                          func_80077AA4(0, t) & 0xFFFF, 0, &col, 0xBE, 1);
            k = D_800F336C;
            b.vx = p->x;
            b.vy = p->y;
            b.vz = p->z;
            b.pad = 1;
            t = D_800E1204[k];
            if (k == 4 && D_800F3428 != 0) {
                t += 4;
            }
            func_800CEE20(D_80190758, &b, 0x1000, 0x1000,
                          (D_800F336A << 1) * (D_800E27EC - 1) + 0x60,
                          func_80077AA4(0x20, t) & 0xFFFF, 3, 0x80, 0);
            break;
        case 1:
            b.vx = 0;
            b.vy = 0;
            b.pad = 0;
            b.vz = D_800E27EC << 6;
            s2 = func_80077DC4((D_800E27EC - 1) << 7) / 32;
            t = D_800E1204[D_800F336C];
            if (D_800F336C == 4 && D_800F3428 != 0) {
                t += 4;
            }
            func_800CEE20(p, &b, 0x1000, 0x1000, D_800F336A * D_800E27EC + 0x40,
                          func_80077AA4(0x10, t) & 0xFFFF, 1, s2, 0);
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
