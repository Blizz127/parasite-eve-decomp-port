/* room_m0419i (PE.IMG room m0419i chunk 2, VRAM 0x8018EFE8)
 * func_8018FB88 — blob offset 0xba0, 0xa4 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0075i func_8018FB44; C re-targeted by symbol address
 * (docs/evidence/room_m0419i-ports-2026-09-23/REPORT.md). */

void func_8018FB88(int a0, unsigned char *q, unsigned char *p)
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
