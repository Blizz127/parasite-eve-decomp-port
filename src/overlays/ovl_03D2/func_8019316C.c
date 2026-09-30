/* ovl_03D2 (PE.IMG title/boot overlay, VRAM 0x8018EFF0)
 * func_8019316C — blob offset 0x417C, 0x30 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the overlay VMA (docs/evidence/ovl_03D2-func_8019316C/REPORT.md).
 * Fade-out step: f1C -= 0x10 clamped at 0; at 0 clear f24/f20. */

typedef struct Task {
    struct Task *next;     /* 0x00 */
    short f04;
    short f06;             /* 0x06 */
    int f08;
    void *f0C;             /* 0x0C handler */
    int f10;
    int f14;               /* 0x14 */
    int f18;
    int f1C;               /* 0x1C */
    int f20;               /* 0x20 */
    int f24;               /* 0x24 */
    int f28;
    int f2C;               /* 0x2C id */
    int f30;               /* 0x30 */
} Task;
void func_8019316C(Task *t)
{
    int v = t->f1C - 0x10;
    if (v < 0) {
        v = 0;
    }
    t->f1C = v;
    if (v == 0) {
        t->f24 = 0;
        t->f20 = 0;
    }
}
