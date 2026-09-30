/* room_m0273i — func_80196F2C, blob offset 0x7F44, 0x140 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl9 2026-09-28).
 * k == 4 family, li-after variant: first draft (t after the table read, int us = (unsigned short) cast). */

extern int D_800E27EC;
extern unsigned short D_800F336C;
extern short D_800F336A;
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern int D_800966EC[];
extern short D_8019AD68[];
extern unsigned short func_80077AA4();
extern void func_800CEE20();

int func_80196F2C(int a0, short *a1)
{
    int u;
    int t;
    int us;

    if (a0 == 1) {
        if (D_800E27EC >= 0x10) {
            return 1;
        }
        a1[1] -= 6;
    } else if (a0 == 2) {
        u = D_800E1204[D_800F336C];
        t = D_800E27EC - 1;
        if (D_800F336C == 4 && D_800F3428 != 0) {
            u += 4;
        }
        us = (unsigned short)func_80077AA4(0x40, u);
        func_800CEE20(a1, 0, 0x1000, 0x1000, D_800F336A * D_8019AD68[t / 4] + 0x2C, us, 1,
                      (short)*(int *)((char *)D_800966EC + ((t << 9) & 0x3E00)) >> 5, 0);
    }
    return 0;
}
