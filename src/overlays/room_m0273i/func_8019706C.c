/* room_m0273i — func_8019706C, blob offset 0x8084, 0x1C4 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl11 2026-09-28).
 * Queued-vector flush controller over a SoA state struct (x/y/z arrays + count + flag share one base). */

extern unsigned char *D_800F33E0;
extern struct { unsigned short x[12]; unsigned short y[12]; unsigned short z[12]; short n; unsigned char pad[0x13]; unsigned char f5D; } D_8019AF0C;
extern struct { short a68, a6A, a6C, a6E, a70, a72, a74, a76, a78; } D_800F3368;
extern unsigned short D_800E11EA;
extern unsigned short D_800E2850[];
extern void func_80196F2C();
extern int func_800CE560();
extern short *func_800CE610();

int func_8019706C(int a0)
{
    short *e;
    int i;

    switch (a0) {
    case 0:
        return func_800CE560(((void **)D_800F33E0)[2], 8, 0x28, func_80196F2C);
    case 1:
        if (D_8019AF0C.f5D != 0) {
            return 2;
        }
        for (i = 0; i < D_8019AF0C.n; i++) {
            e = func_800CE610(((void **)D_800F33E0)[2]);
            if (e == 0) {
                break;
            }
            e[0] = D_8019AF0C.x[i];
            e[1] = D_8019AF0C.y[i] - 0x80;
            e[2] = D_8019AF0C.z[i];
            e[3] = 0;
        }
        D_8019AF0C.n = 0;
        return 0;
    case 2:
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
