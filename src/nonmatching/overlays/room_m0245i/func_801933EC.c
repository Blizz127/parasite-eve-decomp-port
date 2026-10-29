/* NON_MATCHING: functionally equivalent to retail room_m0245i 0x4404..0x46A4
 * (func_801933EC; spimdisasm marks it handwritten for its inline cop2 loads).
 * Written from the retail asm, not byte-matched; native port differential-tested
 * by tools/analysis/e21_diff.py.
 * Particle callback of the func_8019368C triceratops beam glow pool: q[0..2]
 * position, q[5] age, q[6..7] drift.  Mode 1 moves it (finished at 16 frames),
 * mode 2 draws a func_800D2370 sprite trail and a func_800CEE20 flare at D_801959AC. */
typedef struct { int m[8]; } Mtx;
extern unsigned char D_8018F1DC[4];
extern short D_801959AC[];
extern int D_800E27EC;
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern unsigned short D_800F336C;
extern short D_800F336A;
extern Mtx *D_800BCFA4;
extern int func_80077DC4(int a);
extern int func_80077AA4(int a0, int a1);
extern void func_800D2370(void *pos, short *q, int a2, int a3, int s10, int s14, int s18,
                          int s1c, int clut, void *c0, void *c1, int scale, int abr);
extern void func_800CEE20(short *pos, void *ang, int sx, int sy, int tex, int clut,
                          int abr, int bright, void *extra);

#define GTE_VIEW(m) do {                                                        \
        m = D_800BCFA4;                                                         \
        asm volatile("lw $12,0(%0)\n\t" "lw $13,4(%0)\n\t" "ctc2 $12,$0\n\t" "ctc2 $13,$1\n\t" "lw $12,8(%0)\n\t" "lw $13,12(%0)\n\t" "lw $14,16(%0)\n\t" "ctc2 $12,$2\n\t" "ctc2 $13,$3\n\t" "ctc2 $14,$4" : : "r"( m ) : "$12", "$13", "$14") ; \
        asm volatile("lw $12,20(%0)\n\t" "lw $13,24(%0)\n\t" "ctc2 $12,$5\n\t" "lw $14,28(%0)\n\t" "ctc2 $13,$6\n\t" "ctc2 $14,$7" : : "r"( m ) : "$12", "$13", "$14") ; \
    } while (0)
#define CLUT(a0, c) (c = D_800E1204[D_800F336C],                      \
                     (D_800F336C == 4 && D_800F3428) ? c += 4 : 0,    \
                     func_80077AA4(a0, c) & 0xFFFF)

int func_801933EC(int mode, short *q)
{
    unsigned char w[4];
    Mtx *m;
    int i, c, s1, s2;

    for (i = 0; i < 4; i++) w[i] = D_8018F1DC[i];
    if (mode == 1) {
        q[5] = q[5] + 1;
        q[1] = q[1] + q[7];
        q[0] = q[0] + q[6];
        q[2] = q[2] + 0x18;
        return q[5] < 0x10 ? 0 : 1;
    }
    if (mode != 2)
        return 0;
    s1 = func_80077DC4(q[5] << 6) / 32;
    func_80077DC4(q[5] << 6);
    s2 = (D_800E27EC & 1) << 7;
    GTE_VIEW(m);
    i = CLUT(0x10, c);
    func_800D2370(D_801959AC, q, 0x1FE, 0x6E, s2, 0, 0x80, 0x20, i, w, w, (short)s1, 1);
    i = CLUT(0x40, c);
    func_800CEE20(D_801959AC, 0, 0x1000, 0x1000, D_800F336A * (q[5] / 2) + 0x90, i, 1, s1 / 2, 0);
    return 0;
}
