/* NON_MATCHING: functionally equivalent to retail room_m0188i 0x6204..0x65B8
 * (func_801951EC).  Written from the retail asm, not byte-matched; native port
 * differential-tested by tools/analysis/e21_diff.py.
 * Battle effect main (m0188i enemy ground burst): mode 0 places it on the
 * effect owner, flips the owner's resource state 1 -> 2 and starts a
 * 36-particle pool (func_8019479C, life 22); mode 1 every 4th frame, for d[4]
 * bursts, spawns one floor flash, one column and eight debris particles at a
 * random point 1500..2899 out from the owner; mode 2 sets the blend depth. */
typedef struct {
    short vx, vy, vz, pad;
} SVec;

extern int *D_800F32D0;
extern int *D_800F33E0;
extern unsigned char *D_800E2368;
extern int D_800E27EC;
extern short D_800942EC;
extern short D_800F3374;
extern void func_800CE870(int who, int a1, short *out);
extern void func_800CE9D4(int pool, int a1, short *dir);
extern int func_80071A54(void);
extern void func_800CFB7C(short *dir, int dist, short *out);
extern short *func_800CE610(int pool);
extern int func_800CE560(int pool, int n, int life, void *fn);
extern void func_8019479C();

int func_801951EC(int mode, short *d)
{
    short fr[8];        /* sp+0x10: direction, sp+0x18: point */
    short *p;
    int *o;
    int i, r;

    if (mode == 0) {
        d[4] = 6;
        func_800CE870(D_800F32D0[2], 0, d);
        if (D_800E2368[0xD] && (o = (int *)D_800F32D0[2]) != 0 && (o = (int *)o[0]) != 0 &&
            *(unsigned char *)o[6] == 1)
            *(unsigned char *)o[6] = 2;
        return func_800CE560(D_800F33E0[2], 0x24, 0x16, func_8019479C);
    }
    if (mode == 2) {
        D_800F3374 = 8;
        return 0;
    }
    if (mode != 1)
        return 0;
    if ((D_800E27EC & 3) == 0) {
        if (d[4] <= 0)
            return 2;
        func_800CE9D4(D_800F32D0[2], 0, &fr[0]);
        fr[2] = 0;
        fr[0] = 0;
        fr[1] = fr[1] - 0x100 + (func_80071A54() & 0x1FF);
        r = func_80071A54();
        func_800CFB7C(&fr[0], (short)(r % 1400 + 0x5DC), &fr[4]);
        fr[4] = fr[4] + d[0];
        fr[5] = fr[5] + d[1];
        fr[5] = D_800942EC;
        fr[6] = fr[6] + d[2];
        p = func_800CE610(D_800F33E0[2]);
        if (p) {
            p[4] = fr[4];
            p[5] = fr[5];
            p[16] = 0;
            p[17] = 0;
            p[6] = fr[6];
        }
        p = func_800CE610(D_800F33E0[2]);
        if (p) {
            p[0] = fr[4];
            p[1] = fr[5];
            p[16] = 1;
            p[17] = 0;
            p[2] = fr[6];
        }
        for (i = 0; i < 8; i++) {
            p = func_800CE610(D_800F33E0[2]);
            if (p) {
                p[0] = fr[4];
                p[1] = fr[5];
                p[2] = fr[6];
                r = func_80071A54();
                p[4] = r % 128 - 0x40;
                r = func_80071A54();
                p[5] = -(r % 128);
                r = func_80071A54();
                p[6] = r % 128 - 0x40;
                p[16] = 2;
                p[17] = 0;
            }
        }
        d[4] = d[4] - 1;
    }
    return d[4] > 0 ? 0 : 2;
}
