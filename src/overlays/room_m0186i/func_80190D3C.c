/* room_m0186i (PE.IMG room m0186i chunk 2, VRAM 0x8018EFE8)
 * func_80190D3C — blob offset 0x1D54, 0xE0 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the room VMA (docs/evidence/room-lane-2026-09-23/REPORT.md).
 * Particle step: +0x16 capped at 15, two fades, spread x/z by +0x14, drop +0x10; done after frame 21. */

void func_80190D3C(int a0, unsigned char *q, short *p)
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
