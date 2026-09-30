/* room_m0350i — func_80192ADC, blob offset 0x3AF4, 0x158 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl9 2026-09-28).
 * k == 4 sprite family, li-after variant (scaled by a1[2]): first draft. */

extern int D_800E27EC;
extern unsigned short D_800F336C;
extern short D_800F336A;
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern int D_800966EC[];
extern char D_8019A3D0[];
extern unsigned short func_80077AA4();
extern void func_800CEE20();

int func_80192ADC(int a0, void **a1)
{
    int u;
    int s;
    unsigned short us;

    if (a0 == 1) {
        if (D_800E27EC >= 0x10) {
            return 1;
        }
    } else if (a0 == 2) {
        s = *(short *)((char *)D_800966EC + (((D_800E27EC - 1) << 8) & 0x3F00)) * ((short *)a1)[2] / 4096;
        u = D_800E1204[D_800F336C];
        if (D_800F336C == 4 && D_800F3428 != 0) {
            u += 4;
        }
        us = func_80077AA4(0x20, u);
        func_800CEE20(*a1, 0, s, s, D_800F336A + 0xD8, us, 1,
                      (short)*(int *)((char *)D_800966EC + (((D_800E27EC - 1) << 9) & 0x3E00)) >> 5, D_8019A3D0);
    }
    return 0;
}
