/* room_m0123i — func_801929FC, blob offset 0x3A14, 0x1E0 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl11 2026-09-28).
 * Timed 3+1 burst emitter controller (every 6 frames below 0x28); jump-threaded t<0x28 test. */

extern unsigned char *D_800F33E0;
extern int D_800E27EC;
extern struct { short a68, a6A, a6C, a6E, a70, a72, a74, a76, a78; } D_800F3368;
extern unsigned short D_800E11EA;
extern unsigned short D_800E2850[];
extern void func_8019251C();
extern int func_800CE560();
extern short *func_800CE610();

int func_801929FC(int a0)
{
    short *e;
    int i;

    switch (a0) {
    case 0:
        return func_800CE560(((void **)D_800F33E0)[2], 8, 0x10, func_8019251C);
    case 1:
        if (D_800E27EC % 6 == 0 && D_800E27EC < 0x28) {
            for (i = 0; i < 3; i++) {
                e = func_800CE610(((void **)D_800F33E0)[2]);
                if (e) {
                    e[0] = 1;
                    e[1] = 0;
                    e[2] = i << 12;
                }
            }
            if (D_800E27EC < 0x19) {
                e = func_800CE610(((void **)D_800F33E0)[2]);
                if (e) {
                    e[0] = 0;
                    e[1] = 0;
                    e[2] = 0;
                }
            }
        }
        if (D_800E27EC >= 0x28) {
            return 2;
        }
        break;
    case 2:
        D_800F3368.a68 = 0x10;
        D_800F3368.a6A = 1;
        D_800F3368.a76 = 0x10;
        D_800F3368.a78 = 0x10;
        D_800F3368.a70 = D_800E2850[D_800E11EA];
        D_800F3368.a6C = 3;
        D_800F3368.a6E = 0;
        D_800F3368.a72 = 0;
        D_800F3368.a74 = 0xC;
        break;
    }
    return 0;
}
