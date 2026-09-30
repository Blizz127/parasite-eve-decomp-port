/* room_m0137i — func_8018FD14, blob offset 0xD2C, 0x104 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl 2026-09-27).
 * approach/brake toward e+0xC then func_800C6B90 step; lever: 'v > w' operand order loads v first */

extern unsigned char *func_800C2B50();
extern int func_800C6B90();

void func_8018FD14(void *a0, unsigned char *a1, unsigned char *a2)
{
    unsigned char *e;
    short t;
    short v;
    int w;

    e = func_800C2B50();
    if (*(short *)(e + 0xC) - 0x3C < *(short *)(a1 + 2)) {
        t = *(short *)(a2 + 0xA);
        if (t >= 5) {
            *(short *)(a2 + 0xA) = t - 4;
        }
        if (*(short *)(e + 0xC) - 0x1E < *(short *)(a1 + 2)) {
            v = *(short *)(a2 + 8);
            if (v > *(short *)(e + 0xA) * 6) {
                w = *(short *)(e + 0xA) * 6;
                *(short *)(a2 + 8) = v - w;
            }
        }
    } else {
        *(short *)(a2 + 8) += *(short *)(e + 0xA);
    }
    if (func_800C6B90(a2, *(short *)(a2 + 8) >> 3)) {
        *(short *)(e + 8) = 1;
    }
    if (*(short *)(a1 + 2) == *(short *)(e + 0xC)) {
        a1[1] = 2;
    }
}
