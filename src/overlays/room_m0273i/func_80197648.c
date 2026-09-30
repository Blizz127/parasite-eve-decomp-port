/* room_m0273i — func_80197648, blob offset 0x8660, 0x1B0 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl11 2026-09-28).
 * Emitter controller with short cooldown counter D_8019AE84; struct source vector. */

extern unsigned char *D_800F33E0;
extern unsigned char D_8019AF69;
extern unsigned char D_8019AF6A;
extern short D_8019AE84;
extern struct { unsigned short x, y, z; } D_8019AEFC;
extern struct { short a68, a6A, a6C, a6E, a70, a72, a74, a76, a78; } D_800F3368;
extern unsigned short D_800E11E8;
extern unsigned short D_800E2850[];
extern void func_801974DC();
extern int func_800CE560();
extern short *func_800CE610();

int func_80197648(int a0)
{
    short *e;
    short c;

    switch (a0) {
    case 0:
        D_8019AE84 = 0;
        return func_800CE560(((void **)D_800F33E0)[2], 8, 0xC, func_801974DC);
    case 1:
        if (D_8019AF69 != 0) {
            return 2;
        }
        if (D_8019AF6A != 0) {
            c = D_8019AE84;
            D_8019AE84 = c - 1;
            if (c <= 0) {
                e = func_800CE610(((void **)D_800F33E0)[2]);
                if (e) {
                    e[0] = D_8019AEFC.x;
                    e[1] = D_8019AEFC.y - 0x100;
                    e[2] = D_8019AEFC.z;
                    e[3] = 0;
                    D_8019AE84 = 2;
                }
            }
        }
        break;
    case 2:
        D_800F3368.a68 = 0x10;
        D_800F3368.a6A = 1;
        D_800F3368.a76 = 0x10;
        D_800F3368.a78 = 0x10;
        D_800F3368.a76 = 0x10;
        D_800F3368.a78 = 0x10;
        D_800F3368.a70 = D_800E2850[D_800E11E8];
        D_800F3368.a6C = 2;
        D_800F3368.a6E = 0;
        D_800F3368.a72 = 0;
        D_800F3368.a74 = 0;
        break;
    }
    return 0;
}
