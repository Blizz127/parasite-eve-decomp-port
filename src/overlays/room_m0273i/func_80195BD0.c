/* room_m0273i — func_80195BD0, blob offset 0x6BE8, 0x240 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl11 2026-09-28).
 * Dual emitter queue flush (SoA state struct D_8019AEB8, second CE610 slot gets the particle pointer). */

extern unsigned char *D_800F33E0;
extern struct { unsigned short x[8]; unsigned short y[8]; unsigned short z[8]; short n; unsigned char pad[0xE]; unsigned char f; } D_8019AEB8;
extern int D_8019AE7C;
extern struct { short a68, a6A, a6C, a6E, a70, a72, a74, a76, a78; } D_800F3368;
extern unsigned short D_800E11EA;
extern unsigned short D_800E2850[];
extern void func_80195984();
extern void func_8019A290();
extern int func_800CE560();
extern int func_800CE5AC();
extern void func_800CE688();
extern void func_800CE78C();
extern short *func_800CE610();

int func_80195BD0(int a0)
{
    short *e;
    short **g;
    int r;
    int i;

    switch (a0) {
    case 0:
        r = func_800CE560(((void **)D_800F33E0)[2], 0xC, 0xA, func_80195984);
        asm volatile("");
        return r + func_800CE5AC(&D_8019AE7C, r, 4, 0xA, func_8019A290);
    case 1:
        if (D_8019AEB8.f != 0) {
            func_800CE688(D_8019AE7C);
            return 2;
        }
        for (i = 0; i < D_8019AEB8.n; i++) {
            e = func_800CE610(((void **)D_800F33E0)[2]);
            if (e == 0) {
                break;
            }
            g = (short **)func_800CE610(D_8019AE7C);
            e[0] = D_8019AEB8.x[i];
            e[1] = D_8019AEB8.y[i] - 0x80;
            e[2] = D_8019AEB8.z[i];
            e[4] = 0;
            e[3] = 0x80;
            *g = e;
        }
        D_8019AEB8.n = 0;
        func_800CE688(D_8019AE7C);
        break;
    case 2:
        func_800CE78C(D_8019AE7C);
        D_800F3368.a68 = 0x20;
        D_800F3368.a6A = 2;
        D_800F3368.a76 = 0x20;
        D_800F3368.a78 = 0x20;
        D_800F3368.a76 = 0x20;
        D_800F3368.a78 = 0x20;
        D_800F3368.a70 = D_800E2850[D_800E11EA];
        D_800F3368.a6C = 3;
        D_800F3368.a6E = 0;
        D_800F3368.a72 = 0;
        D_800F3368.a74 = 0;
        break;
    }
    return 0;
}
