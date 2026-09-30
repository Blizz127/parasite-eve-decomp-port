/* room_m0418i — func_80194D74, blob offset 0x5D8C, 0x5C bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl 2026-09-27).
 * velocity/clamp step on a2 record; sets a1[1]=2 when a1 state 0x3C */

void func_80194D74(void *a0, unsigned char *a1, unsigned char *a2)
{
    short t;

    *(short *)(a2 + 0x10) += *(short *)(a2 + 0x12);
    *(short *)(a2 + 0x12) += 10;
    t = *(short *)(a2 + 0x14);
    if (t >= 9) {
        *(short *)(a2 + 0x14) = t - 8;
    }
    if (*(short *)(a1 + 2) == 0x3C) {
        a1[1] = 2;
    }
}
