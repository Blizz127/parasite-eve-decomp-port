/* room_m0350i — func_80192C34, blob offset 0x3C4C, 0x218 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl11 2026-09-28).
 * Fading 2-slot emitter controller: += / -= 0x1000 approach clamp, D_800F33E0 read into q before the a1 init stores, table value hoisted to t. */

typedef struct { short x, y, z, w; } SV;
extern unsigned char *D_800F33E0;
extern unsigned char *D_800F32D0;
extern int D_800E27EC;
extern SV D_8019A778[];
extern struct { short a68, a6A, a6C, a6E, a70, a72, a74, a76, a78; } D_800F3368;
extern unsigned short D_800E11E8;
extern unsigned short D_800E2850[];
extern void func_80192ADC();
extern int func_800CE560();
extern void **func_800CE610();

int func_80192C34(int a0, short *a1)
{
    void **e;
    register SV *s asm("$16");
    register int i asm("$17");
    int a;
    int b;
    int t;
    unsigned char *q;

    switch (a0) {
    case 0:
        q = D_800F33E0;
        a1[0] = 0x4000;
        a1[1] = 0;
        return func_800CE560(((void **)q)[2], 8, 6, func_80192ADC);
    case 1:
        if (a1[0] == 0) {
            if (a1[1] <= 0) {
                return 2;
            }
        } else if (*(unsigned short *)(((unsigned char **)D_800F32D0)[2] + 0x16) >= 0x29) {
            a1[0] = 0;
        }
        if (D_800E27EC & 7) {
            return 0;
        }
        b = a1[1];
        a = a1[0];
        if (b < a) {
            a1[1] += 0x1000;
        } else if (a < b) {
            a1[1] -= 0x1000;
        }
        if (a1[1] != 0) {
            for (i = 0, s = D_8019A778; i < 2; i++) {
                e = func_800CE610(((void **)D_800F33E0)[2]);
                if (e == 0) {
                    break;
                }
                e[0] = s;
                ((short *)e)[2] = a1[1];
                s++;
            }
        }
        break;
    case 2:
        t = D_800E2850[D_800E11E8];
        D_800F3368.a6C = 2;
        D_800F3368.a6E = 0;
        D_800F3368.a68 = 0x10;
        D_800F3368.a6A = 1;
        D_800F3368.a76 = 0x10;
        D_800F3368.a78 = 0x10;
        D_800F3368.a76 = 0x10;
        D_800F3368.a78 = 0x10;
        D_800F3368.a72 = 0;
        D_800F3368.a74 = 0;
        D_800F3368.a70 = t;
        break;
    }
    return 0;
}
