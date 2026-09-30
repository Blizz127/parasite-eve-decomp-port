/* ovl_0700 (PE.IMG map-exit overlay, VRAM 0x8018EFF0)
 * func_80194108 — blob offset 0x5118, 0x9C bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the overlay VMA (docs/evidence/ovl_0700-func_80194108/REPORT.md).
 * Step a signed short level toward zero by 0x10 with clamps, calling func_801941A4. Inner else-order is load-bearing (4 words). */

extern void func_801941A4(unsigned char);
short func_80194108(short v)
{
    if (v > 0) {
        if (v < 0xFF) {
            func_801941A4(v);
            v += 0x10;
        } else {
            func_801941A4(0xFF);
        }
    }
    if (v < 0) {
        if (v >= -0xFE) {
            func_801941A4(v - 1);
            v -= 0x10;
        } else {
            func_801941A4(0);
            v = 0;
        }
    }
    return v;
}
