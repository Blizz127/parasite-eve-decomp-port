/* room_m0350i — func_8019360C, blob offset 0x4624, 0x1A8 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl11 2026-09-28).
 * 2-slot pointer seed + func_8006DCE4 SE call; fresh r local for the second player-pointer read keeps p in $a0. */

typedef struct { short x, y, z, w; } SV;
extern unsigned char *D_800F33E0;
extern unsigned char *D_800F32D0;
extern SV D_8019A778[];
extern struct { short a68, a6A, a6C, a6E, a70, a72, a74, a76, a78; } D_800F3368;
extern void func_80192E4C();
extern int func_800CE560();
extern void **func_800CE610();
extern void func_8006DCE4();

int func_8019360C(int a0)
{
    void **e;
    unsigned char *p;
    unsigned char *q;
    unsigned char *r;
    register SV *s asm("$16");
    register int i asm("$17");
    int a;
    int b;

    switch (a0) {
    case 0:
        return func_800CE560(((void **)D_800F33E0)[2], 8, 2, func_80192E4C);
    case 1:
        p = ((unsigned char **)D_800F32D0)[2];
        if (p[0xE] != 0xB) {
            return 0;
        }
        a = *(unsigned short *)(p + 0x16);
        if (a >= 0x32) {
            return 2;
        }
        b = *(unsigned short *)(p + 0x1A);
        if (a < 8) {
            return 0;
        }
        if (b < 8) {
            for (i = 0, s = D_8019A778; i < 2; i++) {
                e = func_800CE610(((void **)D_800F33E0)[2]);
                if (e == 0) {
                    break;
                }
                e[0] = s;
                ((short *)e)[2] = 0;
                s++;
            }
            r = ((unsigned char **)D_800F32D0)[2];
            q = *(unsigned char **)(r + 0x238);
            func_8006DCE4(0x5C6, ((void **)*(void **)r)[2], *(short *)(q + 0x14), *(short *)(q + 0x18), *(short *)(q + 0x1C));
            return 0;
        }
        break;
    case 2:
        D_800F3368.a68 = 0x20;
        D_800F3368.a6A = 2;
        D_800F3368.a76 = 0x20;
        D_800F3368.a78 = 0x20;
        D_800F3368.a76 = 0x20;
        D_800F3368.a78 = 0x20;
        D_800F3368.a72 = 0;
        D_800F3368.a74 = 0;
        break;
    }
    return 0;
}
