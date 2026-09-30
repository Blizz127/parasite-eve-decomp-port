/* room_m0350i — func_801928D4, blob offset 0x38EC, 0x208 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl11 2026-09-28).
 * Random-jitter 2-slot particle seeder with a1 wave counter (first try, r local for D_800F33E0). */

typedef struct { short x, y, z, w; } SV;
typedef struct { short h0, h2, h4, h6; SV *w8; int wC; } PT;
extern unsigned char *D_800F33E0;
extern int D_800E27EC;
extern SV D_8019A778[];
extern int D_8019A414[];
extern struct { short a68, a6A, a6C, a6E, a70, a72, a74, a76, a78; } D_800F3368;
extern unsigned short D_800E11EA;
extern unsigned short D_800E2850[];
extern void func_801927A4();
extern int func_800CE560();
extern PT *func_800CE610();
extern int func_80052B2C();

int func_801928D4(int a0, short *a1)
{
    PT *e;
    unsigned char *r;
    int i;
    short c;

    switch (a0) {
    case 0:
        r = D_800F33E0;
        a1[0] = 0;
        a1[1] = 0;
        return func_800CE560(((void **)r)[2], 0x10, 0x12, func_801927A4);
    case 1:
        if (D_800E27EC >= 0x11) {
            return 2;
        }
        for (i = 0; i < 2; i++) {
            e = func_800CE610(((void **)D_800F33E0)[2]);
            if (e == 0) {
                break;
            }
            e->h0 = func_80052B2C() << 4;
            e->h2 = func_80052B2C() << 4;
            e->h6 = 1;
            e->h4 = 0;
            e->w8 = &D_8019A778[i];
            e->wC = D_8019A414[(a1[1] + i) & 7];
        }
        c = a1[1];
        a1[1] = c + 1;
        a1[0] = 4 - c;
        if (a1[0] > 0) {
            return 0;
        }
        a1[0] = 1;
        break;
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
