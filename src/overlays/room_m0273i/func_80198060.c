/* room_m0273i — func_80198060, blob offset 0x9078, 0x144 bytes. Flags -O2 -G0 + MASPSX_NARROW_SHIFTED_WORD_LOAD (K3);
 * LINK_EXACT at the target VMA (lane ovl9 2026-09-28).
 * k == 4 family, li-after variant. Levers: asm("" : "=r"(s) : "0"(s)) after the scale read keeps the D_800F336C load below it (27 -> 21), K3 word-load for the final >> 21 read (21 -> 6), register void *a1 asm($16) (retail s0 = a1; 6 -> 0). */

extern int D_800E27EC;
extern unsigned short D_800F336C;
extern short D_800F336A;
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern short D_800966EE[];
extern int D_800966EC[];
extern char D_8019AE00[];
extern unsigned short func_80077AA4();
extern void func_800CEE20();

int func_80198060(int a0, void *a1_)
{
    register void *a1 asm("$16") = a1_;
    int u;
    int s;
    unsigned short us;

    if (a0 == 1) {
        if (D_800E27EC >= 9) {
            return 1;
        }
    } else if (a0 == 2) {
        if (D_800E27EC - 2 < 0) {
            return 0;
        }
        s = *(short *)((char *)D_800966EE + (((D_800E27EC - 1) << 9) & 0x3E00)) * 2;
        asm("" : "=r"(s) : "0"(s));
        u = D_800E1204[D_800F336C];
        if (D_800F336C == 4 && D_800F3428 != 0) {
            u += 4;
        }
        us = func_80077AA4(0x20, u);
        func_800CEE20(a1, 0, (short)s, (short)s, D_800F336A + 0xD8, us, 1,
                      *(int *)((char *)D_800966EC + (((D_800E27EC - 1) << 9) & 0x3E00)) >> 21, D_8019AE00);
    }
    return 0;
}
