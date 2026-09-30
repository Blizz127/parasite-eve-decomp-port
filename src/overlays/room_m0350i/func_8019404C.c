/* room_m0350i — func_8019404C, blob offset 0x5064, 0x1D0 bytes. Flags -O2 -G0 + MASPSX_NARROW_SHIFTED_WORD_LOAD (K3);
 * LINK_EXACT at the target VMA (lane ovl13 2026-09-28).
 * CEE20 + D004C pair (k==4 li-after); pad[2]; *(int)>>21 narrowed by K3; first try. */

extern int D_800E27EC;
extern unsigned short D_800F336C;
extern short D_800F336A;
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern short D_800966EC[];
extern char D_8019A3CC[];
extern char D_8019A3C8[];
extern unsigned short func_80077AA4();
extern void func_800CEE20();
extern void func_800D004C();

int func_8019404C(int a0, short *a1)
{
    int s;
    int u;
    int pad[2];

    if (a0 == 1) {
        if (D_800E27EC >= 8) {
            return 1;
        }
    } else if (a0 == 2) {
        s = *(short *)((char *)D_800966EC + (((D_800E27EC - 1) << 9) & 0x3E00)) * a1[5] / 4096;
        u = D_800E1204[D_800F336C];
        if (D_800F336C == 4 && D_800F3428 != 0) {
            u += 4;
        }
        func_800CEE20(&a1[2], 0, s, s, D_800F336A * 5 + 0xD8, func_80077AA4(0x30, u), 1,
                      *(int *)((char *)D_800966EC + (((D_800E27EC - 1) << 9) & 0x3E00)) >> 21, a1);
        func_800D004C(&a1[2], 0x60, 0x60, 8, 0, s, s, D_8019A3CC, D_8019A3C8,
                      (short)*(int *)((char *)D_800966EC + (((D_800E27EC - 1) << 10) & 0x3C00)) >> 5, 1);
    }
    return 0;
}
