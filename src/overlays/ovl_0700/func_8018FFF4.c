/* ovl_0700 (PE.IMG map-exit overlay, VRAM 0x8018EFF0)
 * func_8018FFF4 — blob offset 0x1004, 0x130 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the overlay VMA (docs/evidence/ovl_0700-func_8018FFF4/REPORT.md).
 * Side test of an int XYZ point against two rows of the D_8019CBB0 matrix (+D_8019CB48/4C); returns 1 when both signs agree with D_8019CC04 / D_8019CC0C. Declaring 'int b, a' (not 'a, b') is load-bearing: it decides the $a1/$a2 homes (4 words). */

extern int D_8019CBB0[3];
extern int D_8019CBD0[3];
extern int D_8019CB48;
extern int D_8019CB4C;
extern int D_8019CC04;
extern int D_8019CC0C;
int func_8018FFF4(int *p)
{
    int x, y;
    int b, a;
    x = D_8019CBB0[0] * p[0] + D_8019CBB0[1] * p[1] + D_8019CBB0[2] * p[2] + D_8019CB48;
    y = D_8019CBD0[0] * p[0] + D_8019CBD0[1] * p[1] + D_8019CBD0[2] * p[2] + D_8019CB4C;
    a = 0;
    b = 0;
    if (x > 0 && D_8019CC04 > 0) a = 1;
    if (x < 0 && D_8019CC04 < 0) a = 1;
    if (y > 0 && D_8019CC0C > 0) b = 1;
    if (y < 0 && D_8019CC0C < 0) b = 1;
    return a & b;
}
