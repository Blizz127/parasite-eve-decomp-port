/* ovl_0700 (PE.IMG map-exit overlay, VRAM 0x8018EFF0)
 * func_80190254 — blob offset 0x1264, 0x25C bytes. Profile era_o2_g0_expand_div;
 * LINK_EXACT at the overlay VMA (docs/evidence/ovl_0700-func_80190254/REPORT.md).
 * func_8018FFF4 side test extended with a distance gate: a side that fails
 * the sign test still passes when |coord| / D_8019CBC8 (or D_8019CC00) is below
 * (short)(n * 8 + 30). Levers: `a, b` declaration order + `b = 0; a = 0;`
 * init order, `$5` pins for n/lim (keeps n in $a1, lim reuses it), a `$2`
 * temp for n * 8 (lets reorg duplicate `sll v0,a1,3` into a delay slot) and
 * for the second (short)lim; MASPSX_EXPAND_DIV for the div break guards. */

extern int D_8019CBB0[3];
extern int D_8019CBD0[3];
extern int D_8019CB48;
extern int D_8019CB4C;
extern int D_8019CBC8;
extern int D_8019CC00;
extern int D_8019CC04;
extern int D_8019CC0C;
int func_80190254(int *p, int n)
{
    int x, y;
    int a, b;
    register int lim asm("$5");
    register int t asm("$2");
    register int nn asm("$5") = n;
    int q;
    x = D_8019CBB0[0] * p[0] + D_8019CBB0[1] * p[1] + D_8019CBB0[2] * p[2] + D_8019CB48;
    y = D_8019CBD0[0] * p[0] + D_8019CBD0[1] * p[1] + D_8019CBD0[2] * p[2] + D_8019CB4C;
    b = 0;
    a = 0;
    if (D_8019CC04 > 0 && x >= 0) a = 1;
    if (D_8019CC04 < 0 && x <= 0) a = 1;
    if (D_8019CC0C > 0 && y >= 0) b = 1;
    if (D_8019CC0C < 0 && y <= 0) b = 1;
    t = nn * 8;
    lim = t + 30;
    if (a == 0) {
        if (x >= 0) {
            q = x / D_8019CBC8;
        } else {
            q = -x / D_8019CBC8;
        }
        if (q < (short)lim) a = 1;
    }
    if (b == 0) {
        if (y >= 0) {
            q = y / D_8019CC00;
        } else {
            q = -y / D_8019CC00;
        }
        t = (short)lim;
        if (q < t) b = 1;
    }
    return a & b;
}
