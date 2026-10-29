/* room_m0089i — func_801924F8, blob offset 0x3510, 0x538 bytes. Default profile era_o2_g0.
 * Room trail-effect callback over a 6-point history (p[0] head .. p[5] tail).
 * cmd 1: phase 0 shifts the history down one slot, re-seeds the head from a2 and steps it
 *   (rise by +0x36, swirl by rsin/rcos of +0x34), state 1 after 16 ticks; phase 1 just
 *   re-seeds the head, done after 8 ticks.
 * cmd 2: phase 0 draws the 5 linked segments with func_800D2B58 (skipping zero points);
 *   phase 1 draws one func_800CEE20 glow.
 * LINK_EXACT (chk.sh, default -O2 -G0).
 * Levers: e.vz via a block-local int (keeps lh, not lhu); four pinned $4.
 *   Phase 1: one local k, pinned $3, carries the rsin result, its in-place `k / 32`
 *   (written out as the round-toward-zero `if (k < 0) k += 31; s = k >> 5;`, so the
 *   addiu happens in $v1), and the D_800E1204 byte index (k = j << 1). A separate j
 *   holds D_800F336C ($v0). This gives retail's move v1,v0 and the $v1/$v0 split.
 *   Without the pin it lands in $a1 (7 mismatches); with `/ 32` it gets 55.
 * inline GTE/asm: uses Psy-Q inline_c.h-shaped GTE (COP2) asm macros. */

#define gte_SetRotMatrix(r0) __asm__ volatile ( \
    "lw $12,0(%0)\n\tlw $13,4(%0)\n\tctc2 $12,$0\n\tctc2 $13,$1\n\t" \
    "lw $12,8(%0)\n\tlw $13,12(%0)\n\tlw $14,16(%0)\n\t" \
    "ctc2 $12,$2\n\tctc2 $13,$3\n\tctc2 $14,$4" : : "r"(r0) : "$12", "$13", "$14")
#define gte_SetTransMatrix(r0) __asm__ volatile ( \
    "lw $12,20(%0)\n\tlw $13,24(%0)\n\tctc2 $12,$5\n\tlw $14,28(%0)\n\t" \
    "ctc2 $13,$6\n\tctc2 $14,$7" : : "r"(r0) : "$12", "$13", "$14")
typedef struct { short vx, vy, vz, pad; } SVECTOR;
typedef struct { short m[3][3]; int t[3]; } MATRIX;
typedef struct { unsigned char a, b, c, d; } S4;
typedef struct {
    SVECTOR p[6];
    short cnt;   /* 0x30 */
    short state; /* 0x32 */
    short ang;   /* 0x34 */
    short spd;   /* 0x36 */
} Trail;

extern S4 D_8018F1CC;
extern S4 D_8018F1D0;
extern MATRIX *D_800BCFA4[];
extern unsigned short D_800E1204[];
extern unsigned short D_800F336C;
extern int D_800F3428;
extern int func_80077AA4();
extern int func_80077CF4();
extern int func_80077DC4();
extern void func_800D2B58();
extern void func_800CEE20();

int func_801924F8(int cmd, Trail *o, short *v)
{
    SVECTOR e;
    S4 c1;
    S4 c2;
    int s;
    register int k asm("$3");
    int t;

    c1 = D_8018F1CC;
    c2 = D_8018F1D0;
    switch (cmd) {
    case 1:
        switch (o->state) {
        case 0:
            o->cnt++;
            o->p[5].vx = o->p[4].vx;
            o->p[5].vy = o->p[4].vy;
            o->p[5].vz = o->p[4].vz;
            o->p[4].vx = o->p[3].vx;
            o->p[4].vy = o->p[3].vy;
            o->p[4].vz = o->p[3].vz;
            o->p[3].vx = o->p[2].vx;
            o->p[3].vy = o->p[2].vy;
            o->p[3].vz = o->p[2].vz;
            o->p[2].vx = o->p[1].vx;
            o->p[2].vy = o->p[1].vy;
            o->p[2].vz = o->p[1].vz;
            o->p[1].vx = o->p[0].vx;
            o->p[1].vy = o->p[0].vy;
            o->p[1].vz = o->p[0].vz;
            o->p[0].vx = v[0];
            o->p[0].vy = v[1];
            o->p[0].vz = v[2];
            o->p[0].vy -= o->spd - o->spd * o->cnt / 16;
            s = o->p[0].pad - o->p[0].pad * o->cnt / 16;
            o->p[0].vx += func_80077DC4(o->ang) * s / 4096;
            o->p[0].vz += func_80077CF4(o->ang) * s / 4096;
            o->ang += o->p[1].pad;
            if (o->cnt >= 16) {
                o->state = 1;
                o->cnt = 0;
            }
            break;
        case 1:
            o->cnt++;
            o->p[0].vx = v[0];
            o->p[0].vy = v[1];
            o->p[0].vz = v[2];
            if (o->cnt >= 8) {
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
        switch (o->state) {
        case 0:
            gte_SetRotMatrix(D_800BCFA4[0]);
            gte_SetTransMatrix(D_800BCFA4[0]);
            if (o->p[5].vx | o->p[5].vy | o->p[5].vz) {
                func_800D2B58(&o->p[4], &o->p[5], &c1, &c2, 0x19, 0, 1);
            }
            if (o->p[4].vx | o->p[4].vy | o->p[4].vz) {
                func_800D2B58(&o->p[3], &o->p[4], &c1, &c2, 0x32, 0x19, 1);
            }
            if (o->p[3].vx | o->p[3].vy | o->p[3].vz) {
                func_800D2B58(&o->p[2], &o->p[3], &c1, &c2, 0x4B, 0x32, 1);
            }
            if (o->p[2].vx | o->p[2].vy | o->p[2].vz) {
                func_800D2B58(&o->p[1], &o->p[2], &c1, &c2, 0x64, 0x4B, 1);
            }
            if (o->p[1].vx | o->p[1].vy | o->p[1].vz) {
                func_800D2B58(&o->p[0], &o->p[1], &c1, &c2, 0x80, 0x64, 1);
            }
            break;
        case 1:
            e.vx = 0x400;
            e.vy = 0;
            {
                int z = o->cnt << 7;
                e.vz = z;
            }
            e.pad = 1;
            k = func_80077DC4(o->cnt << 7);
            if (k < 0) {
                k += 31;
            }
            s = k >> 5;
            {
                register int four asm("$4");
                int j;
                j = D_800F336C;
                four = 4;
                k = j << 1;
                t = *(unsigned short *)((char *)D_800E1204 + k);
                k = j;
                if (k == four && D_800F3428 != 0) {
                    t += 8;
                } else {
                    t += 4;
                }
            }
            func_800CEE20(o, &e, 0x1000, 0x1000, 6, func_80077AA4(0, t) & 0xFFFF, 1, s, &c1);
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
