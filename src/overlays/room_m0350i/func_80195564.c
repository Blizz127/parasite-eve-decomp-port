/* room_m0350i — func_80195564, blob offset 0x657C, 0x138 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl9 2026-09-28).
 * k == 4 sprite family (func_801937B4 lever set: four/u/o pins, us local; no K3 needed — (short) low-half read). */

extern int D_800E27EC;
extern unsigned short D_800F336C;
extern short D_800F336A;
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern int D_800966EC[];
extern char D_8019A3D0[];
extern unsigned char D_8019A4EC[];
extern unsigned short func_80077AA4();
extern void func_800CEE20();

int func_80195564(int a0, short *a1)
{
    register int u asm("$5");
    int t;
    int m;
    register int o asm("$3");
    unsigned short us;
    int k;
    register int four asm("$4");

    if (a0 == 1) {
        if (D_800E27EC >= 8) {
            return 1;
        }
    } else if (a0 == 2) {
        four = 4;
        m = D_800F336C;
        o = m << 1;
        u = *(unsigned short *)((char *)D_800E1204 + o);
        if (m == four && D_800F3428 != 0) {
            u += 8;
        } else {
            u += 4;
        }
        us = func_80077AA4(0, u);
        t = D_800E27EC - 1;
        func_800CEE20(a1, 0, a1[3], a1[3], D_800F336A * D_8019A4EC[t / 2] + 0x80, us, 1,
                      (short)*(int *)((char *)D_800966EC + ((t << 9) & 0x3E00)) >> 5, D_8019A3D0);
    }
    return 0;
}
