/* room_m0034i (PE.IMG room m0034i chunk 2, VRAM 0x8018EFE8)
 * func_8018FEE0 — blob offset 0xef8, 0x54 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0174i func_80190128; C re-targeted by symbol address
 * (docs/evidence/room_m0034i-ports-2026-09-23/REPORT.md). */

void func_8018FEE0(int a0, unsigned char *q, unsigned char *p)
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
