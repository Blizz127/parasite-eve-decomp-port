/* room_m0273i — func_80194284, blob offset 0x529C, 0x1EC bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl11 2026-09-28).
 * 2-slot SVECTOR seed with random y jitter (func_80052B2C()<<4), short loop counter. */

typedef struct { short x, y, z, w; } SV;
extern unsigned char *D_800F33E0;
extern unsigned char *D_800F32D0;
extern unsigned char D_8019AE9A;
extern SV D_8019AC20[];
extern struct { short a68, a6A, a6C, a6E, a70, a72, a74, a76, a78; } D_800F3368;
extern unsigned short D_800E11FA;
extern unsigned short D_800E2850[];
extern void func_80194128();
extern int func_800CE560();
extern SV *func_800CE610();
extern int func_80052B2C();

int func_80194284(int a0)
{
    SV *e;
    unsigned char *p;
    short i;
    int k;
    int a;
    unsigned short b;

    switch (a0) {
    case 0:
        return func_800CE560(((void **)D_800F33E0)[2], 8, 4, func_80194128);
    case 1:
        if (D_8019AE9A != 0) {
            return 2;
        }
        p = ((unsigned char **)D_800F32D0)[2];
        if (p[0xE] != 9) {
            return 0;
        }
        a = *(short *)(p + 0x16);
        b = *(unsigned short *)(p + 0x1A);
        if (a > 0 && (short)b <= 0) {
            k = func_80052B2C() << 4;
            for (i = 0; i < 2; i++) {
                e = func_800CE610(((void **)D_800F33E0)[2]);
                if (e == 0) {
                    break;
                }
                *e = D_8019AC20[i];
                e->y += k;
            }
            return 0;
        }
        break;
    case 2:
        D_800F3368.a68 = 0x10;
        D_800F3368.a6A = 1;
        D_800F3368.a76 = 0x10;
        D_800F3368.a78 = 0x10;
        D_800F3368.a76 = 0x10;
        D_800F3368.a78 = 0x40;
        D_800F3368.a70 = D_800E2850[D_800E11FA];
        D_800F3368.a6C = 3;
        D_800F3368.a6E = 1;
        D_800F3368.a72 = 3;
        D_800F3368.a74 = 0;
        break;
    }
    return 0;
}
