/* room_m0418i — func_80191348, blob offset 0x2360, 0x90 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl 2026-09-27).
 * sine-driven drift (func_80077DC4), decaying +0xA counter; lever: copy-pointer p=a2 for final test, short n test */

extern int func_80077DC4();

void func_80191348(void *a0, unsigned char *a1, unsigned char *a2)
{
    unsigned char *p = a2;
    int v;
    short n;

    v = func_80077DC4(*(short *)(a1 + 2) << 5) >> 6;
    *(short *)(a2 + 8) += v;
    *(short *)(a2 + 0xA) = n = *(short *)(a2 + 0xA) - 4;
    a2[0xC]++;
    if (n < 0) {
        *(short *)(a2 + 0xA) = 0;
    }
    if (*(short *)(p + 0xA) == 0) {
        a1[1] = 2;
    }
}
