/* NON_MATCHING: functionally equivalent to retail room_m0156i 0x3220..0x3A00
 * (func_80192208; the same bytes run in m0380i).  Written from the retail asm,
 * not byte-matched; native port differential-tested by tools/analysis/e21_diff.py.
 * Effect callback: mode 0 plays two sounds, anchors the effect at the source
 * position and starts the particle pool with func_80191B94; mode 1 sprays debris
 * particles (two bursts on frame 1, then a trickle up to frame 32); mode 2 sets
 * the sprite parameters. */
extern int *D_800F33E0;
extern int D_800B0E64;
extern int D_800E27EC;
extern short D_800942EC;
extern unsigned short D_800E11EA;
extern unsigned short D_800E2850[];
extern short D_800F3368, D_800F336A, D_800F336C, D_800F336E, D_800F3370;
extern short D_800F3372, D_800F3374, D_800F3376, D_800F3378;
extern int func_800D3FD8(void);
extern void func_8006DF50(int a0, int a1, int a2, int a3, int a4);
extern int func_800CE560(int pool, int n, int life, void *fn);
extern short *func_800CE610(int pool);
extern int func_80071A54(void);
extern int func_80077DC4(int a);
extern int func_80077CF4(int a);
extern void func_80191B94();

/* One particle at p, thrown out at a random angle with speed spread*rand8/256.
 * The vy/vz ranges and the [8] flag differ per burst. */
#define SPAWN(ym, yo, vy, zm, zo, flag)                                   \
    do {                                                                  \
        q = func_800CE610(D_800F33E0[2]);                                 \
        if (q != 0) {                                                     \
            q[0] = p[0];                                                  \
            q[1] = p[1];                                                  \
            q[2] = p[2];                                                  \
            a = func_80071A54();                                          \
            s = spread * (func_80071A54() & 0xFF) / 256;                  \
            q[0] += func_80077DC4(a) * s / 4096;                          \
            q[2] += func_80077CF4(a) * s / 4096;                          \
            q[3] = func_80071A54();                                       \
            r = func_80071A54();                                          \
            q[4] = r % (ym) - (yo);                                       \
            r = func_80071A54();                                          \
            q[5] = -(r % (vy));                                           \
            r = func_80071A54();                                          \
            q[6] = r % (zm) - (zo);                                       \
            q[8] = (flag) ? (func_80071A54() & 1) + 1 : 0;                \
            q[9] = 0;                                                     \
        }                                                                 \
    } while (0)

int func_80192208(int mode, short *p, short *src)
{
    int spread;
    int i;
    short *q;
    int a, s, r;

    switch (mode) {
    case 0:
        if (D_800B0E64) {
            func_8006DF50(D_800B0E64, 0x5FA, func_800D3FD8(), 0x80, 0x7F);
            if (D_800B0E64) {
                func_8006DF50(D_800B0E64, 0x5FB, 0x80, 0x80, 0x7F);
            }
        }
        p[0] = src[0];
        p[1] = src[1];
        p[2] = src[2];
        p[1] = D_800942EC;
        return func_800CE560(D_800F33E0[2], 20, 60, func_80191B94);
    case 1:
        spread = *(int *)(src + 4);
        if (D_800E27EC == 1) {
            for (i = 0; i < 20; i++) {
                SPAWN(64, 32, 44, 64, 32, 0);
            }
            for (i = 0; i < 16; i++) {
                SPAWN(128, 64, 80, 128, 64, 1);
            }
        }
        if (D_800E27EC < 33) {
            if (D_800E27EC & 1) {
                SPAWN(70, 35, 44, 70, 35, 0);
            }
            SPAWN(80, 40, 49, 80, 40, 1);
        }
        return D_800E27EC < 2 ? 0 : 2;
    case 2:
        D_800F3368 = 0x20;
        D_800F336A = 2;
        D_800F3376 = 0x20;
        D_800F3378 = 0x20;
        D_800F336C = 3;
        D_800F336E = 0;
        D_800F3372 = 0;
        D_800F3374 = 8;
        D_800F3370 = D_800E2850[D_800E11EA];
        return 0;
    }
    return 0;
}
