/* room_m0273i — func_80193B5C, blob offset 0x4B74, 0x15C bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl11 2026-09-28).
 * Two-pass func_800D0728 emitter over short/int tables (short loop counter, shared v[4] block). */

extern int D_800E27EC;
extern short D_8019AC18[];
extern int D_8019AC0C[];
extern void func_800D0728();

int func_80193B5C(int a0, short **a1)
{
    short v[4];
    short i;

    if (a0 == 1) {
        if (D_800E27EC >= 0x20) {
            return 1;
        }
    } else if (a0 == 2) {
        v[0] = 0x400;
        v[2] = 0;
        v[3] = 1;
        v[1] = D_800E27EC << 7;
        for (i = 0; i < 2; i++) {
            func_800D0728(*a1, D_8019AC18[i], D_8019AC18[i + 1], 0x10, v, (*a1)[4], (*a1)[4],
                          &D_8019AC0C[i], &D_8019AC0C[i + 1], (*a1)[6], 1);
        }
    }
    return 0;
}
