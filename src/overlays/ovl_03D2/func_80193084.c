/* ovl_03D2 (PE.IMG title/boot overlay, VRAM 0x8018EFF0)
 * func_80193084 — blob offset 0x4094, 0x54 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the overlay VMA (docs/evidence/ovl_03D2-func_80193084/REPORT.md).
 * Fade integrator: stop f20 at the [0, 0x100] bounds, then f1C += f20. */

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
void func_80193084(Task *t)
{
    if ((t->f1C <= 0 && t->f20 < 0) || (t->f1C >= 0x100 && t->f20 > 0)) {
        t->f20 = 0;
    }
    t->f1C += t->f20;
}
