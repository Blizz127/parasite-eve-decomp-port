/* room_m0350i — func_80193BCC, blob offset 0x4BE4, 0x1A0 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl11 2026-09-28).
 * Emitter controller with 2-slot seed loop; s/i pinned to $16/$17; case 1 falls to the shared return 0 with break. */

extern unsigned char *D_800F33E0;
extern unsigned char *D_800F32D0;
extern char D_8019A778[];
extern struct { short a68, a6A, a6C, a6E, a70, a72, a74, a76, a78; } D_800F3368;
extern unsigned short D_800E11FA;
extern unsigned short D_800E2850[];
extern void func_80193A80();
extern int func_800CE560();
extern void **func_800CE610();

#define P(o, x) (*(void **)((char *)(o) + (x)))
#define H(o, x) (*(unsigned short *)((char *)(o) + (x)))

int func_80193BCC(int a0)
{
    void **e;
    unsigned char *p;
    register char *s asm("$16");
    register int i asm("$17");
    int a;
    int b;

    switch (a0) {
    case 0:
        return func_800CE560(P(D_800F33E0, 8), 4, 2, func_80193A80);
    case 1:
        p = P(D_800F32D0, 8);
        if (p[0xE] != 0xB) {
            return 0;
        }
        a = H(p, 0x16);
        b = H(p, 0x1A);
        if (a >= 5 && b < 5) {
            s = D_8019A778;
            for (i = 0; i < 2; i++) {
                e = func_800CE610(P(D_800F33E0, 8));
                if (e == 0) {
                    break;
                }
                *e = s;
                s += 8;
            }
            return 0;
        }
        if (a >= 0x29) {
            return 2;
        }
        break;
    case 2:
        D_800F3368.a68 = 0x20;
        D_800F3368.a6A = 2;
        D_800F3368.a76 = 0x20;
        D_800F3368.a78 = 0x20;
        D_800F3368.a76 = 0x20;
        D_800F3368.a78 = 0x20;
        D_800F3368.a70 = D_800E2850[D_800E11FA];
        D_800F3368.a6C = 3;
        D_800F3368.a6E = 1;
        D_800F3368.a72 = 0;
        D_800F3368.a74 = 0;
        break;
    }
    return 0;
}
