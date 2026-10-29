/* NON_MATCHING: functionally equivalent to retail room_m0188i 0x4318..0x48A8
 * (func_80193300; spimdisasm marks it handwritten for its inline cop2 loads).
 * Written from the retail asm, not byte-matched; native port differential-tested
 * by tools/analysis/e21_diff.py.
 * Particle callback of the m0188i battle effect.  q[0..2] position, q[4..6]
 * velocity, q[8] kind (0 rising puff, 1 falling drop, 2 splash), q[9] age.
 * Mode 1 moves it (x/z velocity decays 31/32 per frame) and reports finished
 * (1) at its age limit; mode 2 draws it (kind 1 as a glow point). */
typedef struct {
    short vx, vy, vz, pad;
} SVec;
typedef struct {
    int m[8];
} Mtx;

extern unsigned char D_8018F1D4[8];
extern unsigned char D_8018F1E4[4];
extern unsigned char D_80195E64[];
extern int D_800E27EC;
extern short D_800942EC;
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern unsigned short D_800F336C;
extern short D_800F336A;
extern Mtx *D_800BCFA4;
extern int func_80077DC4(int a);
extern int func_80077CF4(int a);
extern int func_80077AA4(int a0, int a1);
extern void func_800CF3AC(void *table, void *work, int frame);
extern void func_800D2104(short *pos, unsigned char *color, int scale, int abr);
extern void func_800CEE20(short *pos, void *ang, int sx, int sy, int tex, int clut,
                          int abr, int bright, void *extra);

#define CLUT16(c) (c = D_800E1204[D_800F336C],                       \
                   (D_800F336C == 4 && D_800F3428) ? c += 4 : 0,     \
                   func_80077AA4(0x10, c) & 0xFFFF)

int func_80193300(int mode, short *q)
{
    union {
        unsigned char b[8];
        short h[4];
    } ang;                 /* sp+0x28 */
    unsigned char work[8]; /* sp+0x30 */
    unsigned char color[4];/* sp+0x38 */
    int i, c, s0, s2, kind;

    for (i = 0; i < 8; i++) ang.b[i] = D_8018F1D4[i];
    for (i = 0; i < 4; i++) color[i] = D_8018F1E4[i];
    kind = q[8];
    if (mode == 1) {
        if (kind != 0 && kind != 1 && kind != 2)
            return 0;
        q[9] = q[9] + 1;
        q[0] = q[0] + q[4];
        q[1] = q[1] + q[5];
        q[2] = q[2] + q[6];
        q[4] = (q[4] * 31) / 32;
        q[6] = (q[6] * 31) / 32;
        if (kind == 0) {
            q[5] = q[5] + 2;
            return q[9] < 0x18 ? 0 : 1;
        }
        if (kind == 1) {
            q[5] = q[5] + 3;
            if (q[1] >= D_800942EC)
                q[8] = 2;
            return q[9] < 0x10 ? 0 : 1;
        }
        q[5] = q[5] - 1;
        return q[9] < 0x10 ? 0 : 1;
    }
    if (mode != 2)
        return 0;
    if (kind == 1) {
        Mtx *m = D_800BCFA4;
        asm volatile("lw $12,0(%0)\n\t" "lw $13,4(%0)\n\t" "ctc2 $12,$0\n\t" "ctc2 $13,$1\n\t" "lw $12,8(%0)\n\t" "lw $13,12(%0)\n\t" "lw $14,16(%0)\n\t" "ctc2 $12,$2\n\t" "ctc2 $13,$3\n\t" "ctc2 $14,$4" : : "r"( m ) : "$12", "$13", "$14") ;
        asm volatile("lw $12,20(%0)\n\t" "lw $13,24(%0)\n\t" "ctc2 $12,$5\n\t" "lw $14,28(%0)\n\t" "ctc2 $13,$6\n\t" "ctc2 $14,$7" : : "r"( m ) : "$12", "$13", "$14") ;
        func_800D2104(q, color, 0x80, 0xFF);
        return 0;
    }
    if (kind == 0) {
        s0 = func_80077DC4((D_800E27EC << 10) / 24) + 0x1000;
        ang.h[2] = (short)(D_800E27EC * 50);
        func_800CF3AC(D_80195E64, work, q[9]);
        i = CLUT16(c);
        func_800CEE20(q, ang.b, s0, s0, D_800F336A * (short)(q[9] / 3) + 0x20, i, 1, 0x80, work);
        return 0;
    }
    if (kind == 2) {
        s0 = func_80077CF4((q[9] << 10) / 24) + 0x1000;
        s2 = func_80077DC4((q[9] * 1204) / 24) / 32;
        i = CLUT16(c);
        func_800CEE20(q, 0, 0x1000, s0, D_800F336A * (short)(q[9] / 2) + 0x20, i, 1, s2, 0);
        return 0;
    }
    return 0;
}
