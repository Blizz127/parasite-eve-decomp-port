/* room_m0350i — func_80197594, blob offset 0x85AC, 0x134 bytes. Flags -O2 -G0 + MASPSX_NARROW_SHIFTED_WORD_LOAD (K3);
 * LINK_EXACT at the target VMA (lane ovl9 2026-09-28).
 * k == 4 sprite family (func_801937B4 lever set: K3 + four/u/o pins, us local). */

extern int D_800E27EC;
extern unsigned short D_800F336C;
extern short D_800F336A;
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern int D_800966EC[];
extern char D_8019A62C[];
extern unsigned short func_80077AA4();
extern void func_800CEE20();

int func_80197594(int a0, short *a1)
{
    register int u asm("$5");
    int t;
    int m;
    register int o asm("$3");
    unsigned short us;
    int k;
    register int four asm("$4");

    if (a0 == 1) {
        if (D_800E27EC >= 0x10) {
            return 1;
        }
        a1[1] -= a1[3];
        a1[3] += 2;
    } else if (a0 == 2) {
        four = 4;
        m = D_800F336C;
        o = m << 1;
        u = *(unsigned short *)((char *)D_800E1204 + o);
        if (m == four && D_800F3428 != 0) {
            u += 6;
        } else {
            u += 2;
        }
        us = func_80077AA4(0, u);
        t = D_800E27EC - 1;
        func_800CEE20(a1, 0, 0x1800, 0x1800, D_800F336A * ((t >> 1) & 7), us, 1,
                      *(int *)((char *)D_800966EC + ((t << 8) & 0x3F00)) >> 21, D_8019A62C);
    }
    return 0;
}
