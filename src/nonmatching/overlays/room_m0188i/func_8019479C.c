/* NON_MATCHING: functionally equivalent to retail room_m0188i 0x57B4..0x6204
 * (func_8019479C; spimdisasm marks it handwritten for its inline cop2 loads).
 * Written from the retail asm, not byte-matched; native port differential-tested
 * by tools/analysis/e21_diff.py.
 * Particle callback of the func_801951EC ground-burst pool.  q[0..2] position,
 * q[4..6] velocity, q[8..10] / q[12..14] ribbon control points, q[16] kind
 * (0 ribbon from the enemy's mouth, 1 shockwave flash, 2 debris), q[17] age.
 * Mode 1 steps it (kind 1 flags the player hit, target state 0x21); mode 2
 * draws it (kind 0 as a func_800D3114 ribbon through D_80195FD8). */
typedef struct {
    short vx, vy, vz, pad;
} SVec;
typedef struct {
    int m[8];
} Mtx;

extern unsigned char D_8018F1CC[8];
extern unsigned char D_8018F1DC[8];
extern unsigned char D_80195F00[];
extern unsigned char D_80195FD8[8], D_80195FE0[8], D_80195FE8[8], D_80195FF0[8];
extern int **D_8009D254;
extern int *D_800F32D0;
extern unsigned char *D_800E2368;
extern short D_800942EC;
extern unsigned short D_800E1204[];
extern unsigned short D_800E2850[];
extern unsigned short D_800E11FA, D_800E11EA, D_800E120A;
extern int D_800F3428;
extern unsigned short D_800F336C;
extern short D_800F336A, D_800F3368, D_800F3376, D_800F3378;
extern short D_800F336E, D_800F3370, D_800F3372;
extern Mtx *D_800BCFA4;
extern void func_800CE8F0(int pool, int idx, void *local, short *out);
extern void func_800783E4(short *a, short *b, int wa, int wb, short *out);
extern int func_80071A54(void);
extern int func_800C6B90(short *pos, int r);
extern int func_80077DC4(int a);
extern int func_80077CF4(int a);
extern int func_80077AA4(int a0, int a1);
extern void func_800CF3AC(void *table, void *work, int frame);
extern void func_800CEE20(short *pos, void *ang, int sx, int sy, int tex, int clut,
                          int abr, int bright, void *extra);
extern void func_800D0728(short *pos, int inner, int outer, int segments, void *a4, int sx,
                          int sy, int a7, void *work, int bright, int abr);
extern void func_800D3114(void *pts, int n, int width, int u0, int v0, int du, int dv,
                          int tpage, int clut, int scale, void *col0, void *col1, int abr);

#define BLEND(w, b) (D_800F3368 = (w), D_800F336A = (b), D_800F3376 = (w), D_800F3378 = (w))
/* CLUT row + 2 (+6 on the alternate palette) */
#define CLUT2(c) (c = D_800E1204[D_800F336C],                                  \
                  c += (D_800F336C == 4 && D_800F3428) ? 6 : 2,              \
                  func_80077AA4(0, c) & 0xFFFF)
#define GTE_VIEW(m) do {                                                        \
        m = D_800BCFA4;                                                         \
        asm volatile("lw $12,0(%0)\n\t" "lw $13,4(%0)\n\t" "ctc2 $12,$0\n\t" "ctc2 $13,$1\n\t" "lw $12,8(%0)\n\t" "lw $13,12(%0)\n\t" "lw $14,16(%0)\n\t" "ctc2 $12,$2\n\t" "ctc2 $13,$3\n\t" "ctc2 $14,$4" : : "r"( m ) : "$12", "$13", "$14") ; \
        asm volatile("lw $12,20(%0)\n\t" "lw $13,24(%0)\n\t" "ctc2 $12,$5\n\t" "lw $14,28(%0)\n\t" "ctc2 $13,$6\n\t" "ctc2 $14,$7" : : "r"( m ) : "$12", "$13", "$14") ; \
    } while (0)

int func_8019479C(int mode, short *q)
{
    short fr[28];       /* sp+0x38..0x70, retail frame order */
    short *A = &fr[0], *B = &fr[4], *P1 = &fr[8], *P2 = &fr[12];
    unsigned char *W = (unsigned char *)&fr[16];
    short *R = &fr[20], *ang = &fr[24];
    Mtx *m;
    int *x;
    int i, c, s1, kind;

    for (i = 0; i < 8; i++) ((unsigned char *)A)[i] = D_8018F1CC[i];
    for (i = 0; i < 8; i++) ((unsigned char *)B)[i] = D_8018F1DC[i];
    kind = q[16];
    if (mode == 1) {
        if (kind == 0) {
            q[17] = q[17] + 1;
            func_800CE8F0(D_800F32D0[2], 0xB, B, P1);
            func_800CE8F0(D_800F32D0[2], 0xF, B, P2);
            q[0] = (P1[0] + P2[0]) / 2;
            q[1] = (P1[1] + P2[1]) / 2;
            q[2] = (P1[2] + P2[2]) / 2;
            func_800783E4(q, &q[4], 0xAAB, 0x555, &q[8]);
            func_800783E4(q, &q[4], 0x556, 0xAAA, &q[12]);
            q[12] = q[12] - 0x100 + (func_80071A54() & 0x1FF);
            q[13] = q[13] - 0x100 + (func_80071A54() & 0x1FF);
            q[14] = q[14] - 0x100 + (func_80071A54() & 0x1FF);
            q[8] = q[8] - 0x80 + (func_80071A54() & 0xFF);
            q[9] = q[9] - 0x80 + (func_80071A54() & 0xFF);
            q[10] = q[10] - 0x80 + (func_80071A54() & 0xFF);
            return q[17] < 8 ? 0 : 1;
        }
        if (kind == 1) {
            q[17] = q[17] + 1;
            if (func_800C6B90(q, 0x190) && D_800E2368[0xD]) {
                x = *(int **)D_800F32D0[2];
                if ((*x & 0x3F000000) == 0x1000000) {
                    (*D_8009D254)[0x13] |= 0x4000;
                    *x = (*x & 0xC0FFFFFF) | 0x21000000;
                    *x |= 0x80000000;
                }
            }
            return q[17] < 0x10 ? 0 : 1;
        }
        if (kind == 2) {
            q[17] = q[17] + 1;
            q[0] = q[0] + q[4];
            q[1] = q[1] + q[5];
            q[2] = q[2] + q[6];
            q[4] = (q[4] * 31) / 32;
            q[6] = (q[6] * 31) / 32;
            q[5] = q[5] + 2;
            if (q[1] >= D_800942EC)
                q[5] = -q[5];
            return q[17] < 0x10 ? 0 : 1;
        }
        return 0;
    }
    if (mode != 2)
        return 0;
    if (kind == 0) {
        BLEND(0x20, 2);
        D_800F336C = 3;
        D_800F336E = 1;
        D_800F3372 = 5;
        D_800F3370 = D_800E2850[D_800E11FA];
        i = func_80077AA4(0, D_800E120A + 2) & 0xFFFF;
        func_800CEE20(q, 0, 0x800, 0x800, D_800F336A * q[17] + 0x40, i, 1, 0x80, 0);
        ang[0] = 0x400;
        ang[1] = 0;
        ang[2] = 0;
        ang[3] = 1;
        R[0] = q[0];
        R[1] = D_800942EC;
        R[2] = q[2];
        i = CLUT2(c);
        func_800CEE20(R, ang, 0x800, 0x800, D_800F336A * q[17] + 0x40, i, 3, 0x80, 0);
        for (i = 0; i < 8; i++) D_80195FE8[i] = ((unsigned char *)&q[8])[i];
        for (i = 0; i < 8; i++) D_80195FE0[i] = ((unsigned char *)&q[12])[i];
        for (i = 0; i < 8; i++) D_80195FF0[i] = ((unsigned char *)&q[0])[i];
        for (i = 0; i < 8; i++) D_80195FD8[i] = ((unsigned char *)&q[4])[i];
        GTE_VIEW(m);
        func_800CF3AC(D_80195F00, W, q[17] * 2);
        i = CLUT2(c);
        func_800D3114(D_80195FD8, 3, 0x12C, 0, 0x81, 0x7F, 0x1E, D_800E2850[D_800E11FA], i,
                      0x80, W, W, 1);
        return 0;
    }
    if (kind == 1) {
        BLEND(0x20, 2);
        D_800F336C = 3;
        D_800F336E = 1;
        D_800F3372 = 5;
        D_800F3370 = D_800E2850[D_800E11FA];
        s1 = func_80077CF4(q[17] << 6);
        GTE_VIEW(m);
        func_800CF3AC(D_80195F00, W, q[17]);
        func_800D0728(q, 0x1F4, 0x320, 0x18, A, s1, s1, 0, W, 0x80, 1);
        if (q[17] < 8) {
            i = CLUT2(c);
            func_800CEE20(q, A, 0x2000, 0x2000, D_800F336A * q[17] + 0x40, i, 1, 0x80, 0);
        }
        return 0;
    }
    if (kind == 2) {
        BLEND(0x10, 1);
        D_800F336C = 3;
        D_800F336E = 0;
        D_800F3372 = 0;
        D_800F3370 = D_800E2850[D_800E11EA];
        i = func_80077AA4(0x50, D_800E120A) & 0xFFFF;
        func_800CEE20(q, 0, 0x800, 0x800, D_800F336A * (short)(q[17] / 2) + 0xA7, i, 1, 0x80, 0);
        return 0;
    }
    return 0;
}
