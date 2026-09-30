/* ovl_03D2 (PE.IMG title/boot overlay, VRAM 0x8018EFF0)
 * func_801930D8 — blob offset 0x40E8, 0x94 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the overlay VMA (docs/evidence/ovl_03D2-func_801930D8/REPORT.md).
 * Fade-in integrator: f20 steps to 0x54, f1C += f24*8 below 0x100 (stop at 0/0x100), derive the f06/f0A layout shorts. */

typedef struct Task {
    struct Task *next;     /* 0x00 */
    short f04;
    short f06;             /* 0x06 */
    short f08;
    short f0A;             /* 0x0A */
    void *f0C;             /* 0x0C handler */
    int f10;
    void *f14;             /* 0x14 */
    int f18;
    int f1C;               /* 0x1C */
    int f20;               /* 0x20 */
    int f24;               /* 0x24 */
    int f28;
    int f2C;               /* 0x2C id */
    int f30;               /* 0x30 */
} Task;
void func_801930D8(Task *t)
{
    int a;
    int v;
    int d;
    int inc;
    int r;
    a = t->f20;
    inc = 0;
    if (a != 0) {
        inc = a < 0x54;
    }
    a += inc;
    t->f20 = a;
    v = t->f1C;
    if (v < 0x100) {
        v += t->f24 * 8;
    }
    t->f1C = v;
    if (v == 0 || v == 0x100) {
        t->f24 = 0;
    }
    t->f06 = (0x3C - t->f20 < 0) ? 0x8C : t->f20 + 0x50;
    d = 0x54 - t->f20;
    r = 0x18;
    if (d >= 0x18) {
        r = d;
    }
    t->f0A = r;
}
