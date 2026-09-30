/* room_m0075i (PE.IMG room m0075i chunk 2, VRAM 0x8018EFE8)
 * func_8018FB44 — blob offset 0xB5C, 0xA4 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the room VMA (docs/evidence/room-lane-2026-09-23/REPORT.md).
 * Effect step keyed on q+2 windows (1-7, 21-49, 51+); done at 58. */

void func_8018FB44(int a0, unsigned char *q, unsigned char *p)
{
    unsigned char *e = p;
    short t;

    if ((unsigned short)(*(unsigned short *)(q + 2) - 1) < 7) {
        *(short *)(p + 0x12) += 16;
    }
    if ((unsigned short)(*(unsigned short *)(q + 2) - 21) < 29) {
        *(short *)(p + 0x10) += 40;
    }
    if (*(short *)(q + 2) >= 51) {
        t = *(short *)(e + 0x12);
        if (t > 16) {
            *(short *)(e + 0x12) = t - 16;
        }
    }
    if (*(short *)(q + 2) == 58) {
        q[1] = 2;
    }
}
