/* room_m0350i — func_801968D8, blob offset 0x78F0, 0x13C bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl9 2026-09-28).
 * k == 4 sprite family, li-after variant (single +4, arg 0x10): first draft. */

extern int D_800E27EC;
extern unsigned short D_800F336C;
extern short D_800F336A;
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern int D_800966EC[];
extern char D_8019A598[];
extern unsigned short func_80077AA4();
extern void func_800CEE20();

int func_801968D8(int a0, short *a1)
{
    int u;
    int t;
    unsigned short us;

    if (a0 == 1) {
        if (D_800E27EC >= 0x10) {
            return 1;
        }
        a1[1] += a1[3];
        a1[3] -= a1[4];
    } else if (a0 == 2) {
        u = D_800E1204[D_800F336C];
        if (D_800F336C == 4 && D_800F3428 != 0) {
            u += 4;
        }
        us = func_80077AA4(0x10, u);
        t = D_800E27EC - 1;
        func_800CEE20(a1, 0, 0x3000, 0x3000, D_800F336A * ((t >> 1) & 7) + 0xC8, us, 1,
                      (short)*(int *)((char *)D_800966EC + ((t << 9) & 0x3E00)) >> 5, D_8019A598);
    }
    return 0;
}
