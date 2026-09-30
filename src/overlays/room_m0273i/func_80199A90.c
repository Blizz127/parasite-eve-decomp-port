/* room_m0273i — func_80199A90, blob offset 0xAAA8, 0x1C0 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl11 2026-09-28).
 * 2-slot pointer seed controller (func_80193BCC shape), shared D_8019AF74 state struct. */

typedef struct { short x, y, z, w; } SV;
extern unsigned char *D_800F33E0;
extern int D_800E27EC;
extern short D_8019AE88;
extern struct { SV t[4]; short a20, a22, a24, a26, a28, a2A, a2C; unsigned char f2E; } D_8019AF74;
extern struct { short a68, a6A, a6C, a6E, a70, a72, a74, a76, a78; } D_800F3368;
extern unsigned short D_800E11FA;
extern unsigned short D_800E2850[];
extern void func_80199950();
extern int func_800CE560();
extern void **func_800CE610();

int func_80199A90(int a0)
{
    void **e;
    register SV *s asm("$16");
    register int i asm("$17");

    switch (a0) {
    case 0:
        D_8019AE88 = 0;
        return func_800CE560(((void **)D_800F33E0)[2], 4, 6, func_80199950);
    case 1:
        if (D_8019AE88 != 0) {
            return 2;
        }
        if (D_8019AF74.a20 == 0xF && D_8019AF74.a22 >= 0x19) {
            D_8019AE88 = 1;
            return 2;
        }
        if (D_800E27EC & 7) {
            return 0;
        }
        for (i = 0, s = D_8019AF74.t; i < 2; i++) {
            e = func_800CE610(((void **)D_800F33E0)[2]);
            if (e == 0) {
                break;
            }
            *e = s;
            s++;
        }
        return 0;
    case 2:
        D_800F3368.a68 = 0x40;
        D_800F3368.a6A = 4;
        D_800F3368.a76 = 0x40;
        D_800F3368.a78 = 0x40;
        D_800F3368.a76 = 0x40;
        D_800F3368.a78 = 0x40;
        D_800F3368.a70 = D_800E2850[D_800E11FA];
        D_800F3368.a6C = 3;
        D_800F3368.a6E = 1;
        D_800F3368.a72 = 0;
        D_800F3368.a74 = 0x20;
        break;
    }
    return 0;
}
