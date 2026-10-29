/* NON_MATCHING: functionally equivalent to retail room_m0273i 0x91BC..0x9700
 * (func_801981A4).  Written from the retail asm, not byte-matched; native port
 * differential-tested by tools/analysis/e21_diff.py.
 * Spray-particle callback of the slime swing (m0273i), pool from
 * func_801986E8: q[0..2] position, q[4..6] velocity, byte 0x10 joint, 0x11
 * frame, 0x12 hit latch.  Mode 1 hits the player once within 0x80 (flag
 * 0x4000, 40-frame lockout D_8019AFA0) or splats on the floor; either logs the
 * splat in D_8019AF84 (kind 2 hit / 3 floor), plays SE 0x5FC and retires the
 * particle (1); otherwise it moves and drops a trail point.  Mode 2 draws it. */
typedef struct {
    short vx, vy, vz, pad;
} SVec;

extern int *D_800F32D0;
extern int *D_8009D254;
extern short D_800942EC;
extern int D_8019AF70;
extern short D_8019AFA0;
extern short D_8019AF84[];
extern int D_800E27EC;
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern unsigned short D_800F336C;
extern short D_800F336A;
extern unsigned char D_8019AE04[], D_8019AE08[], D_8019AE0C[], D_8019AB70[];
extern int func_8005186C(int sq);
extern void func_8006DCE4(int se, int src, int x, int y, int z);
extern unsigned char *func_800CE610(int pool);
extern int func_80077AA4(int a0, int a1);
extern void func_800CEE20(SVec *pos, void *a1, int sx, int sy, int a4, int clut, int abr, int bright, void *extra);
extern void func_800D0728(SVec *pos, int a1, int a2, int a3, SVec *ang, int sx, int sy, void *a7, void *a8, int a9, int a10);
extern void func_800D004C(SVec *pos, int a1, int a2, int a3, SVec *ang, int sx, int sy, void *a7, void *a8, int a9, int a10);

#define SPLAT(kind)                                                        \
    do {                                                                   \
        k = (signed char)b[0x10];                                          \
        w = (unsigned char *)D_800F32D0[2];                                \
        D_8019AF84[k * 4 + 0] = q[0];                                      \
        D_8019AF84[k * 4 + 1] = q[1];                                      \
        D_8019AF84[k * 4 + 2] = q[2];                                      \
        D_8019AF84[k * 4 + 3] = (kind);                                    \
        func_8006DCE4(0x5FC, *(int *)(*(int *)w + 8), q[0], q[1], q[2]);   \
    } while (0)

int func_801981A4(int mode, short *q)
{
    unsigned char *b = (unsigned char *)q;
    unsigned char *p;
    SVec pos, ang;
    int dx, dz, r, c, i, t, k, *o;
    unsigned char *w;

    if (mode == 1) {
        if (b[0x12] == 0 && D_8019AFA0 == 0 && q[1] > D_800942EC - 0x240) {
            dx = (short)(D_8009D254[0x7F] - (unsigned short)q[0]);
            dz = (short)(D_8009D254[0x81] - (unsigned short)q[2]);
            if ((short)func_8005186C(dx * dx + dz * dz) < 0x80) {
                b[0x12] = 1;
                ((int *)D_8009D254[0])[0x13] |= 0x4000;
                o = *(int **)D_800F32D0[2];
                if (o)
                    *o |= 0x80000000;
                SPLAT(2);
                D_8019AFA0 = 0x28;
                return 1;
            }
        }
        q[0] = q[0] + q[4];
        q[1] = q[1] + q[5];
        q[2] = q[2] + q[6];
        if (q[1] >= D_800942EC)
            {
            SPLAT(3);
            return 1;
        }
        r = D_800E27EC & 7;
        b[0x11] = r < 4 ? r : r + 0xC;
        p = func_800CE610(D_8019AF70);
        if (!p)
            return 0;
        ((short *)p)[0] = q[0];
        ((short *)p)[1] = q[1];
        ((short *)p)[2] = q[2];
        return 0;
    }
    if (mode != 2)
        return 0;
    pos.vx = q[0];
    pos.vy = q[1];
    pos.vz = q[2];
    c = D_800E1204[D_800F336C];
    if (D_800F336C == 4 && D_800F3428)
        c += 4;
    c = func_80077AA4(0x10, c) & 0xFFFF;
    func_800CEE20(&pos, 0, 0xA00, 0xA00, D_800F336A * b[0x11], c, 1, 0x80, 0);
    ang.vx = D_800E27EC * 3 << 8;
    ang.vy = *(unsigned short *)((unsigned char *)D_800F32D0[2] + 0x3A);
    ang.vz = 0;
    ang.pad = 1;
    for (i = 0; i < 2; i++) {
        func_800D0728(&pos, 0x60, 0x88, 8, &ang, 0x1000, 0x1000, D_8019AE04, D_8019AB70, 0x80, 1);
        t = ang.vx;
        ang.vx = 0;
        ang.vy = t;
    }
    ang.vx = 0;
    ang.vy = 0;
    ang.pad = 0;
    ang.vz = D_800E27EC << 8;
    func_800D004C(&pos, 0x100, 0x100, 8, &ang, 0x1000, 0x1000, D_8019AE08, D_8019AB70, 0x80, 1);
    pos.vy = D_800942EC;
    func_800D004C(&pos, 0xC0, 0xC0, 8, 0, 0x1000, 0x1000, D_8019AE0C, D_8019AB70, 0x80, 1);
    return 0;
}
