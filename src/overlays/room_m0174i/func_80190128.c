/* room_m0174i (PE.IMG room m0174i chunk 2, VRAM 0x8018EFE8)
 * func_80190128 — blob offset 0x1140, 0x54 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the room VMA (docs/evidence/room-lane-2026-09-23/REPORT.md).
 * Frame counter at +2 (done after 20) and a -8 fade at +4 down to 8. */

void func_80190128(int a0, unsigned char *q, unsigned char *p)
{
    short t;

    if ((signed char)++p[2] > 20) {
        q[1] = 2;
    }
    t = *(short *)(p + 4);
    if (t > 8) {
        *(short *)(p + 4) = t - 8;
    }
}
