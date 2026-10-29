/* NON_MATCHING: functionally equivalent to retail room_m0188i 0x6A2C..0x6D8C
 * (func_80195A14).  Written from the retail asm, not byte-matched; native port
 * differential-tested by tools/analysis/e21_diff.py.
 * Battle effect main (m0188i enemy splash): mode 0 starts a 20-particle pool
 * (func_801955A0, life 32); mode 1, at the effect owner, spawns a spray puff
 * every odd frame below 60, sixteen droplets on frame 48 and a sound on frame
 * 2, and reports done (2) from frame 2 on; mode 2 sets the blend state. */
extern int *D_800F32D0;
extern int *D_800F33E0;
extern int D_800E27EC;
extern unsigned short D_800E2850[];
extern unsigned short D_800E11EA;
extern unsigned short D_800F336C;
extern short D_800F336E, D_800F3370, D_800F3372, D_800F3374;
extern void func_800CE870(int who, int a1, short *out);
extern int func_80071A54(void);
extern short *func_800CE610(int pool);
extern int func_800CE560(int pool, int n, int life, void *fn);
extern int func_800D3FD8(void);
extern void func_800D3F64(int id, int a1);
extern void func_801955A0();

int func_80195A14(int mode, short *d)
{
    short pos[4];       /* sp+0x10 */
    short *p;
    int i, r;

    if (mode == 0)
        return func_800CE560(D_800F33E0[2], 0x14, 0x20, func_801955A0);
    if (mode == 2) {
        D_800F336C = 3;
        D_800F336E = 0;
        D_800F3372 = 0;
        D_800F3374 = 8;
        D_800F3370 = D_800E2850[D_800E11EA];
        return 0;
    }
    if (mode != 1)
        return 0;
    func_800CE870(D_800F32D0[2], 0, pos);
    if ((D_800E27EC & 1) && D_800E27EC < 0x3C) {
        p = func_800CE610(D_800F33E0[2]);
        if (p) {
            p[0] = pos[0];
            p[1] = pos[1];
            p[2] = pos[2];
            r = func_80071A54();
            p[4] = r % 80 - 0x28;
            r = func_80071A54();
            p[5] = -(r % 60);
            r = func_80071A54();
            p[8] = 1;
            p[9] = 0;
            p[6] = r % 80 - 0x28;
        }
    }
    if (D_800E27EC == 0x30) {
        for (i = 0; i < 0x10; i++) {
            p = func_800CE610(D_800F33E0[2]);
            if (p) {
                p[0] = pos[0];
                p[1] = pos[1];
                p[2] = pos[2];
                r = func_80071A54();
                p[4] = r % 192 - 0x60;
                r = func_80071A54();
                p[5] = -(r % 170);
                r = func_80071A54();
                p[8] = 0;
                p[9] = 0;
                p[6] = r % 192 - 0x60;
            }
        }
    }
    if (D_800E27EC == 2)
        func_800D3F64(0x5B4, func_800D3FD8());
    return D_800E27EC < 2 ? 0 : 2;
}
