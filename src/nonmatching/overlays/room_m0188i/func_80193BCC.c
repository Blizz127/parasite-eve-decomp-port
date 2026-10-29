/* NON_MATCHING: functionally equivalent to retail room_m0188i 0x4BE4..0x55A0
 * (func_80193BCC; spimdisasm marks it handwritten for its inline cop2 loads).
 * Written from the retail asm, not byte-matched; native port differential-tested
 * by tools/analysis/e21_diff.py.
 * Particle callback of the m0188i enemy's joint glow effect.  q[8] kind
 * (0 joint glow, 1 mouth burst), q[9] age, q[10] joint pair.  Mode 1 ages it
 * (finished at 0x18 / 8 frames); mode 2 draws flashes and rings at the
 * enemy's joints (func_800CE8F0 on the effect owner), kind 1 also a floor
 * ring and a func_800D0728 shockwave. */
typedef struct {
    short vx, vy, vz, pad;
} SVec;
typedef struct {
    int m[8];
} Mtx;

extern unsigned char D_8018F1E8[8];
extern unsigned char D_8018F1F0[8];
extern unsigned char D_8018F1F8[8];
extern unsigned char D_80195EE0[];
extern int *D_800F32D0;
extern short D_800942EC;
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern unsigned short D_800F336C;
extern short D_800F336A, D_800F3368, D_800F3376, D_800F3378;
extern Mtx *D_800BCFA4;
extern void func_800CE8F0(int pool, int idx, void *local, SVec *out);
extern int func_80077DC4(int a);
extern int func_80077CF4(int a);
extern int func_80077AA4(int a0, int a1);
extern void func_800CF3AC(void *table, void *work, int frame);
extern void func_800CEE20(SVec *pos, void *ang, int sx, int sy, int tex, int clut,
                          int abr, int bright, void *extra);
extern void func_800D0728(SVec *pos, int inner, int outer, int segments, int a4, int sx,
                          int sy, int a7, void *work, int bright, int abr);

#define CLUT(a0, c) (c = D_800E1204[D_800F336C],                      \
                     (D_800F336C == 4 && D_800F3428) ? c += 4 : 0,    \
                     func_80077AA4(a0, c) & 0xFFFF)
#define BLEND(w, b) (D_800F3368 = (w), D_800F336A = (b), D_800F3376 = (w), D_800F3378 = (w))
/* small additive flash at P while young */
#define FLASH(q, P, c, i) ((q)[9] < 4                                                     \
        ? (BLEND(0x10, 1), i = CLUT(0x40, c),                                             \
           func_800CEE20(P, 0, 0x2000, 0x2000, D_800F336A * ((q)[9] & 3) + 0xA3, i, 1, 0x80, 0), 0) \
        : 0)

int func_80193BCC(int mode, short *q)
{
    short fr[32];               /* sp+0x30..0x70, retail frame order */
    SVec *P = (SVec *)&fr[0], *Q = (SVec *)&fr[4], *R = (SVec *)&fr[24], *ang = (SVec *)&fr[28];
    unsigned char *a = (unsigned char *)&fr[8], *b = (unsigned char *)&fr[12];
    unsigned char *cc = (unsigned char *)&fr[16], *w = (unsigned char *)&fr[20];
    int i, c, s0, s3, kind, sub;

    for (i = 0; i < 8; i++) a[i] = D_8018F1E8[i];
    for (i = 0; i < 8; i++) b[i] = D_8018F1F0[i];
    for (i = 0; i < 8; i++) cc[i] = D_8018F1F8[i];
    kind = q[8];
    if (mode == 1) {
        if (kind == 0) {
            q[9] = q[9] + 1;
            return q[9] < 0x18 ? 0 : 1;
        }
        if (kind == 1) {
            q[9] = q[9] + 1;
            return q[9] < 8 ? 0 : 1;
        }
        return 0;
    }
    if (mode != 2)
        return 0;
    if (kind == 0) {
        sub = q[10];
        if (sub < 7)
            func_800CE8F0(D_800F32D0[2], sub + 1, a, P);
        else
            func_800CE8F0(D_800F32D0[2], sub + 9, cc, P);
        FLASH(q, P, c, i);
        BLEND(0x20, 2);
        s0 = func_80077DC4((q[9] << 10) / 24) / 32;
        s3 = func_80077CF4((q[9] << 10) / 24) + 0x800;
        i = CLUT(0x60, c);
        func_800CEE20(P, 0, s3, s3, 0xE, i, 1, s0, 0);
        if (sub < 7)
            func_800CE8F0(D_800F32D0[2], q[10] + 1, b, P);
        else
            func_800CE8F0(D_800F32D0[2], q[10] + 0xD, cc, P);
        FLASH(q, P, c, i);
        BLEND(0x20, 2);
        i = CLUT(0x60, c);
        func_800CEE20(P, 0, s3, s3, 0xE, i, 1, s0, 0);
        return 0;
    }
    if (kind == 1) {
        func_800CE8F0(D_800F32D0[2], 0xB, cc, P);
        func_800CE8F0(D_800F32D0[2], 0xF, cc, Q);
        BLEND(0x10, 1);
        P->vx = (P->vx + Q->vx) / 2;
        P->vy = (P->vy + Q->vy) / 2;
        P->vz = (P->vz + Q->vz) / 2;
        i = CLUT(0x50, c);
        func_800CEE20(P, 0, 0x2000, 0x2000, D_800F336A * q[9] + 0xA7, i, 1, 0x80, 0);
        ang->vx = 0x400;
        ang->vy = 0;
        ang->vz = 0;
        ang->pad = 1;
        R->vx = P->vx;
        R->vz = P->vz;
        R->vy = D_800942EC;
        i = CLUT(0x50, c);
        func_800CEE20(R, ang, 0x1000, 0x1000, D_800F336A * q[9] + 0xA7, i, 3, 0x80, 0);
        s3 = func_80077DC4(q[9] << 7);
        func_800CF3AC(D_80195EE0, w, q[9]);
        {
            Mtx *m = D_800BCFA4;
            asm volatile("lw $12,0(%0)\n\t" "lw $13,4(%0)\n\t" "ctc2 $12,$0\n\t" "ctc2 $13,$1\n\t" "lw $12,8(%0)\n\t" "lw $13,12(%0)\n\t" "lw $14,16(%0)\n\t" "ctc2 $12,$2\n\t" "ctc2 $13,$3\n\t" "ctc2 $14,$4" : : "r"( m ) : "$12", "$13", "$14") ;
            asm volatile("lw $12,20(%0)\n\t" "lw $13,24(%0)\n\t" "ctc2 $12,$5\n\t" "lw $14,28(%0)\n\t" "ctc2 $13,$6\n\t" "ctc2 $14,$7" : : "r"( m ) : "$12", "$13", "$14") ;
        }
        func_800D0728(P, 0x15E, 0x2BC, 0x18, 0, s3, s3, 0, w, 0x80, 1);
        return 0;
    }
    return 0;
}
