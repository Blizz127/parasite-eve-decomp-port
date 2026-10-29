/* NON_MATCHING: functionally equivalent to retail room_m0273i 0x9700..0x9B34
 * (func_801986E8).  Written from the retail asm, not byte-matched; native port
 * differential-tested by tools/analysis/e21_diff.py.
 * Slime (m0273i) swing-attack spray effect: mode 0 allocates the pools
 * (func_801981A4 / func_80198060 callbacks); mode 1, during the swing
 * (animation 0xF crossing frame 4), launches a spray particle from each of the
 * two joint points in D_8019AF74 aimed at the player, plays SE 0x5D2 at their
 * midpoint and steps the pool; mode 2 sets the two blend/palette stages.
 * Returns 2 once the swing is over (D_8019AFA2). */
typedef struct {
    short vx, vy, vz, pad;
} SVec;
typedef struct {
    short m[3][3];
    int t[3];
} Mat;

extern int *D_800F32D0;
extern int *D_800F33E0;
extern unsigned char *D_8009D254;
extern int D_8019AF70;
extern unsigned char D_8019AFA2;
extern short D_8019AF74[], D_8019AF96, D_8019AF98;
extern unsigned char D_8019AE10[];
extern unsigned short D_800E11E8, D_800E11EA, D_800E2850[];
extern short D_800F3368, D_800F336A, D_800F336C, D_800F336E, D_800F3370, D_800F3372, D_800F3374;
extern volatile short D_800F3376, D_800F3378;
extern int func_801981A4();
extern int func_80198060();
extern int func_800CE560(void *pool, int size, int count, int (*cb)());
extern int func_800CE5AC(void *slot, int base, int a2, int a3, int (*cb)());
extern unsigned char *func_800CE610(void *pool);
extern void func_800CE688(int pool);
extern void func_800CE78C(int pool);
extern int func_8005186C(int sq);
extern int func_80079FB4(int dy, int dx);
extern void func_80079754(SVec *rot, Mat *m);
extern void func_80078C34(Mat *m, void *in, void *out);
extern void func_8006DCE4(int se, int src, int x, int y, int z);

int func_801986E8(int mode)
{
    SVec a, r;
    Mat m;
    unsigned char *w, *p;
    int i, dx, dz, dy, d, x, y, z, s0;

    if (mode == 0) {
        s0 = func_800CE560((void *)D_800F33E0[2], 0x14, 4, func_801981A4);
        return s0 + func_800CE5AC(&D_8019AF70, s0, 8, 0x14, func_80198060);
    }
    if (mode == 2) {
        D_800F3368 = 0x10;
        D_800F336A = 1;
        D_800F3376 = 0x10;
        D_800F3378 = 0x10;
        D_800F3376 = 0x10;
        D_800F3378 = 0x10;
        D_800F336C = 2;
        D_800F336E = 0;
        D_800F3372 = 0;
        D_800F3374 = 0;
        D_800F3370 = D_800E2850[D_800E11E8];
        func_800CE78C(D_8019AF70);
        D_800F3368 = 0x20;
        D_800F336A = 2;
        D_800F3376 = 0x20;
        D_800F3378 = 0x20;
        D_800F336C = 3;
        D_800F336E = 0;
        D_800F3370 = D_800E2850[D_800E11EA];
        return 0;
    }
    if (mode != 1)
        return 0;
    if (D_8019AFA2)
        return 2;
    w = (unsigned char *)D_800F32D0[2];
    if (w[0xE] == 0xF && D_8019AF96 >= 4 && D_8019AF98 < 4) {
        for (i = 0; i < 2; i++) {
            p = func_800CE610((void *)D_800F33E0[2]);
            if (!p)
                break;
            p[0x10] = i;
            p[0x12] = 0;
            ((short *)p)[0] = D_8019AF74[i * 4 + 0];
            ((short *)p)[1] = D_8019AF74[i * 4 + 1];
            ((short *)p)[2] = D_8019AF74[i * 4 + 2];
            a.vx = *(short *)(D_8009D254 + 0x2A);
            a.vy = *(short *)(D_8009D254 + 0x2E);
            a.vz = *(short *)(D_8009D254 + 0x32);
            dx = a.vx - ((short *)p)[0];
            dz = a.vz - ((short *)p)[2];
            dy = a.vy - ((short *)p)[1];
            d = func_8005186C(dx * dx + dz * dz);
            r.vx = (func_80079FB4(dy, d) & 0xFFF) - 0x30;
            r.vy = *(unsigned short *)((unsigned char *)D_800F32D0[2] + 0x3A);
            r.vz = 0;
            if (r.vx < 0x100)
                r.vx = 0x100;
            if (r.vx > 0x280)
                r.vx = 0x280;
            func_80079754(&r, &m);
            func_80078C34(&m, D_8019AE10, p + 8);
        }
        x = (short)((D_8019AF74[0] + D_8019AF74[4]) / 2);
        y = (short)((D_8019AF74[1] + D_8019AF74[5]) / 2);
        z = (short)((D_8019AF74[2] + D_8019AF74[6]) / 2);
        w = (unsigned char *)D_800F32D0[2];
        func_8006DCE4(0x5D2, *(int *)(*(int *)w + 8), x, y, z);
    }
    func_800CE688(D_8019AF70);
    return 0;
}
