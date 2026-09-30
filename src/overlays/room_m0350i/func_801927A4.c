/* room_m0350i — func_801927A4, blob offset 0x37BC, 0x130 bytes. Profile era_o2_g0_narrow_shifted_word_load
 * (local; MASPSX_NARROW_SHIFTED_WORD_LOAD=1). LINK_EXACT at the target VMA (lane ovl3 2026-09-27). */

extern int D_800E27EC;
extern unsigned short D_800F336C;
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern int D_800966EC[];
extern unsigned short func_80077AA4();
extern void func_800CEE20();

int func_801927A4(int a0, unsigned char *a1)
{
    int c;
    int k;
    int v;

    if (a0 == 1) {
        if (D_800E27EC >= 0x10) {
            return 1;
        }
    } else {
        if (a0 != 2) {
            return 0;
        }
        k = D_800F336C;
        c = (D_800966EC[((D_800E27EC - 1) << 7) & 0xF80] >> 16) * 4 + 0x800;
        v = D_800E1204[k];
        if (k == 4 && D_800F3428 != 0) {
            v += 4;
        }
        func_800CEE20(*(int *)(a1 + 8), a1, c, c, 0x6E, func_80077AA4(0x30, v), 1,
                      (short)D_800966EC[((D_800E27EC - 1) << 7) & 0xF80] >> 5, a1 + 0xC);
    }
    return 0;
}
