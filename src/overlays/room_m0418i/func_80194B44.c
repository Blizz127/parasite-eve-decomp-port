/* room_m0418i — func_80194B44, blob offset 0x5B5C, 0xC0 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl 2026-09-27).
 * decay/brighten step; lever: statement order 0x22 before 0x1C, int k for signed slti */

void func_80194B44(void *a0, unsigned char *a1, unsigned char *a2)
{
    unsigned char *p = a2;
    short n;
    int k;

    a2[0x22] += 0x10;
    *(short *)(a2 + 0x1C) -= *(short *)(a2 + 0x1E);
    *(short *)(a2 + 0x1E) += 0xB;
    *(short *)(a2 + 0x20) = n = *(short *)(a2 + 0x20) + 8;
    if (n > 0x80) {
        *(short *)(a2 + 0x20) = 0x80;
    }
    if (*(short *)(a2 + 0x1C) < 0) {
        *(short *)(a2 + 0x1C) = 0;
    }
    if (a2[0x22] > 200) {
        a2[0x22] = 200;
    }
    k = p[0x22] >> 4;
    if (k > 3) {
        *(short *)(p + 0x14) += 200;
    }
    if (*(short *)(a1 + 2) == 0x78) {
        a1[1] = 2;
    }
}
