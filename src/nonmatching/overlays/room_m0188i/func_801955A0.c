/* NON_MATCHING: functionally equivalent to retail room_m0188i 0x65B8..0x6A2C
 * (func_801955A0).  Written from the retail asm, not byte-matched; native port
 * differential-tested by tools/analysis/e21_diff.py.
 * Particle callback of the func_80195A14 splash pool.  q[0..2] position,
 * q[4..6] velocity, q[8] kind (0 droplet, 1 spray puff), q[9] age.  Mode 1
 * moves it (bouncing off the floor), finished at 0x10 / 0x18 frames; mode 2
 * draws it. */
extern unsigned char D_8018F1D4[8];
extern unsigned char D_80195F78[];
extern int D_800E27EC;
extern short D_800942EC;
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern unsigned short D_800F336C;
extern short D_800F336A, D_800F3368, D_800F3376, D_800F3378;
extern int func_80077DC4(int a);
extern int func_80077CF4(int a);
extern int func_80077AA4(int a0, int a1);
extern void func_800CF3AC(void *table, void *work, int frame);
extern void func_800CEE20(short *pos, void *ang, int sx, int sy, int tex, int clut,
                          int abr, int bright, void *extra);

#define BLEND(w, b) (D_800F3368 = (w), D_800F336A = (b), D_800F3376 = (w), D_800F3378 = (w))
#define CLUT(a0, c) (c = D_800E1204[D_800F336C],                      \
                     (D_800F336C == 4 && D_800F3428) ? c += 4 : 0,    \
                     func_80077AA4(a0, c) & 0xFFFF)

int func_801955A0(int mode, short *q)
{
    short fr[8];        /* sp+0x28 angle, sp+0x30 work */
    int i, c, s0, s2, kind;

    for (i = 0; i < 8; i++) ((unsigned char *)fr)[i] = D_8018F1D4[i];
    kind = q[8];
    if (mode == 1) {
        if (kind == 0) {
            q[0] = q[0] + q[4];
            q[9] = q[9] + 1;
            q[1] = q[1] + q[5];
            q[2] = q[2] + q[6];
            q[4] = (q[4] * 127) / 128;
            q[6] = (q[6] * 127) / 128;
            q[5] = q[5] + 5;
            if (q[1] >= D_800942EC)
                q[5] = -q[5];
            return q[9] < 0x10 ? 0 : 1;
        }
        if (kind == 1) {
            q[0] = q[0] + q[4];
            q[9] = q[9] + 1;
            q[1] = q[1] + q[5];
            q[2] = q[2] + q[6];
            q[4] = (q[4] * 255) / 256;
            q[6] = (q[6] * 255) / 256;
            q[5] = q[5] + 2;
            if (q[1] >= D_800942EC)
                q[5] = -q[5];
            return q[9] < 0x18 ? 0 : 1;
        }
        return 0;
    }
    if (mode != 2)
        return 0;
    if (kind == 0) {
        s2 = func_80077DC4(q[9] << 6) / 32;
        s0 = func_80077DC4(q[9] << 6) / 2;
        BLEND(0x20, 2);
        i = CLUT(0x20, c);
        func_800CEE20(q, 0, s0, s0, 0xA0, i, 1, s2, 0);
        return 0;
    }
    if (kind == 1) {
        s0 = func_80077CF4((q[9] << 10) / 24) * 2 + 0x2000;
        func_80077DC4((q[9] * 1204) / 24);
        BLEND(0x20, 2);
        fr[2] = (short)(D_800E27EC << 5);
        func_800CF3AC(D_80195F78, &fr[4], q[9]);
        i = CLUT(0x10, c);
        func_800CEE20(q, fr, s0, s0, D_800F336A * (q[9] / 4) + 0x20, i, 1, 0xA0, &fr[4]);
        return 0;
    }
    return 0;
}
