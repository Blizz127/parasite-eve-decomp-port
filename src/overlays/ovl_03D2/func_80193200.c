/* ovl_03D2 (PE.IMG title/boot overlay, VRAM 0x8018EFF0)
 * func_80193200 — blob offset 0x4210, 0x54 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the overlay VMA (docs/evidence/ovl_03D2-func_80193200/REPORT.md).
 * Fade-out handler: f1C -= 0x10; at 0x80 call func_8018FE1C; at 0 set done flag f30. */

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
extern void func_8018FE1C();
void func_80193200(Task *t)
{
    t->f1C -= 0x10;
    if (t->f1C == 0x80) {
        func_8018FE1C();
    }
    if (t->f1C == 0) {
        t->f30 = 1;
    }
}
