/* room_m0418i — func_8018FDA8, blob offset 0xDC0, 0x134 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl7 2026-09-27).
 * 30-slot countdown/trigger loop over a2 byte/short arrays; per-array B()/H() index macros; loop-body one=1/e=8 locals give retail's hoisted $t2/$t3 constants in order. */

#define B(off) (a2 + i)[off]
#define H(off) *(short *)(a2 + i * 2 + (off))

void func_8018FDA8(unsigned char *a0, unsigned char *a1, unsigned char *a2)
{
    unsigned int i;
    int f;
    int e;
    int one;

    for (i = 0; i < 30; i++) {
        one = 1;
        e = 8;
        if (B(0x23A) != 0) {
            if (--B(0x23A) == 0) {
                B(0x21C) = one;
            }
        }
        if (B(0x21C) == 1) {
            H(0x168) -= 0x10;
            B(0x1E0)++;
            f = 0;
            if (B(0x1A4) == 0x44) {
                H(0xF0) += 0xC8;
                if (B(0x1A4) == 0x44) {
                    { int t = B(0x1E0); f = t == e; }
                }
            }
            if (B(0x1A4) == 0x4C && B(0x1E0) == e) {
                f = 1;
            }
            if (B(0x1A4) == 0x58 && B(0x1E0) == 0x10) {
                f = 1;
            }
            if (f == 1) {
                B(0x21C) = 0;
                if (--a2[0x258] == 0) {
                    a1[1] = 2;
                }
            }
        }
    }
}
