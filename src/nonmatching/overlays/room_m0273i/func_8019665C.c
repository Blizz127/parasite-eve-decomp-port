/* NON_MATCHING: functionally equivalent to retail room_m0273i 0x7674..0x79F0
 * (func_8019665C; spimdisasm flags its unaligned lwl/lwr copy).
 * Written from the retail asm, not byte-matched; native port differential-tested
 * by tools/analysis/e21_diff.py.
 * Particle callback of the slime drop (m0273i): q[0..2] position, q[3] age
 * (0 falling), q[4..6] ring angle, q[8..10] velocity, q[11] hit latch.
 * Mode 1: a falling drop lands on the floor (logged in the D_8019AF0C/24/3C
 * splat lists, count D_8019AF54) and, within 0x80 of the player, hits once
 * (D_8019AF04 hit point, player flag 0x4000); a landed drop ages out at 8.
 * Mode 2 draws two func_800D0E88 rings while falling, a flare once landed. */
extern int *D_800F32D0;
extern int *D_8009D254;
extern short D_800942EC;
extern unsigned char D_800966EC[];
extern short D_8019AF04[4];
extern short D_8019AF0A, D_8019AF54, D_8019AF62;
extern short D_8019AF0C[], D_8019AF24[], D_8019AF3C[];
extern unsigned char D_8019AB68[], D_8019AD54[], D_8019AD58[], D_8019AD5C[];
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern unsigned short D_800F336C;
extern int func_8005186C(int sq);
extern int func_80077AA4(int a0, int a1);
extern void func_800D0E88(short *pos, short *ang, int a2, int a3, void *s10, void *s14,
                          void *s18, int s1c, int s20);
extern void func_800CEE20(short *pos, void *ang, int sx, int sy, int tex, int clut,
                          int abr, int bright, void *extra);

#define TBL(q, off) (*(short *)(D_800966EC + (((q)[3] << 9) & 0x3E00) + (off)))

int func_8019665C(int mode, short *q)
{
    short ang[4];
    int i, k, c, s0, s3, dx, dz, *x;

    if (mode == 1) {
        if (q[3] != 0) {
            k = q[3];
            q[3] = k + 1;
            return k < 8 ? 0 : 1;
        }
        q[0] = q[0] + q[8];
        q[1] = q[1] + q[9];
        q[2] = q[2] + q[10];
        if (q[1] >= D_800942EC) {
            q[1] = D_800942EC;
            q[3] = 1;
            k = D_8019AF54;
            D_8019AF54 = k + 1;
            D_8019AF0C[k] = q[0];
            D_8019AF24[k] = D_800942EC;
            D_8019AF3C[k] = q[2];
        }
        if (q[11] != 0 || q[1] < D_8019AF62)
            return 0;
        dx = D_8009D254[0x7F] - q[0];
        dz = D_8009D254[0x81] - q[2];
        if (func_8005186C(dx * dx + dz * dz) < 0x80) {
            q[11] = 1;
            for (i = 0; i < 4; i++) D_8019AF04[i] = q[i];
            D_8019AF0A = 1;
            ((int *)D_8009D254[0])[0x13] |= 0x4000;
            x = *(int **)D_800F32D0[2];
            if (x)
                *x |= 0x80000000;
        }
        return 0;
    }
    if (mode != 2)
        return 0;
    s3 = TBL(q, 2) >> 5;
    ang[0] = q[4];
    ang[1] = q[5];
    ang[2] = q[6];
    if (q[3] == 0) {
        for (i = 0; i < 2; i++) {
            func_800D0E88(q, ang, 0x100, 0x10, D_8019AD58, D_8019AD54, D_8019AD54, s3, 1);
            ang[2] = ang[2] + 0x400;
        }
        return 0;
    }
    s0 = TBL(q, 0) * 2 + 0x1000;
    c = D_800E1204[D_800F336C];
    if (D_800F336C == 4 && D_800F3428)
        c += 4;
    c = func_80077AA4(0x30, c) & 0xFFFF;
    func_800CEE20(q, D_8019AB68, s0, s0, 0xDC, c, 1, s3, D_8019AD5C);
    return 0;
}
