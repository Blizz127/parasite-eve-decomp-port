/* room_m0418i — func_80195210, blob offset 0x6228, 0x148 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl7 2026-09-27).
 * Grow two shorts, then rand()%400-200 x12 and rand()%300-150 x3 jitter tables; exit flag when a1+2 >= 0x24. */

extern int func_80071A54();

void func_80195210(int a0, char *a1, char *a2)
{
    unsigned int i;

    if (*(short *)(a2 + 8) < 0xBB8) {
        *(short *)(a2 + 8) += 0x32;
    }
    if (*(short *)(a2 + 0xA) < 0x80) {
        *(short *)(a2 + 0xA) += 8;
    }
    for (i = 0; i < 12; i++) {
        *(short *)(a2 + i * 2 + 0xC) = func_80071A54() % 400 - 200;
    }
    for (i = 0; i < 3; i++) {
        *(short *)(a2 + i * 2 + 0x24) = func_80071A54() % 300 - 150;
    }
    if (*(short *)(a1 + 2) >= 0x24) {
        a1[1] = 2;
    }
}
