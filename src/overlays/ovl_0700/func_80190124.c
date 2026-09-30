/* ovl_0700 (PE.IMG map-exit overlay, VRAM 0x8018EFF0)
 * func_80190124 — blob offset 0x1134, 0x130 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the overlay VMA (docs/evidence/ovl_0700-func_80190124/REPORT.md).
 * SVECTOR (short XYZ) twin of func_8018FFF4; same 'int b, a' declaration-order lever. */

extern int D_8019CBB0[3];
extern int D_8019CBD0[3];
extern int D_8019CB48;
extern int D_8019CB4C;
extern int D_8019CC04;
extern int D_8019CC0C;
int func_80190124(short *p)
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
