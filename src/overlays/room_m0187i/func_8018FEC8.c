/* room_m0187i (PE.IMG room m0187i chunk 2, VRAM 0x8018EFE8)
 * func_8018FEC8 — blob offset 0xee0, 0xe0 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0186i func_80190D3C; C re-targeted by symbol address
 * (docs/evidence/room_m0187i-ports-2026-09-23/REPORT.md). */

void func_8018FEC8(int a0, unsigned char *q, short *p)
{
    short *e = p;
    short t;
    short u;

    if (p[11]++ >= 16) {
        p[11] = 15;
    }
    t = p[9];
    if (t > 8) {
        p[9] = t - 8;
    }
    u = p[10];
    if (u > 20) {
        p[10] = u - 20;
    }
    e[0] += (e[4] * e[10]) >> 16;
    e[2] += (e[6] * e[10]) >> 16;
    e[8] -= 100;
    if (*(short *)(q + 2) >= 21) {
        q[1] = 2;
    }
}
