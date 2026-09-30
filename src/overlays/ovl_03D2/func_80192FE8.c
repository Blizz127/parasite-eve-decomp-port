/* ovl_03D2 (PE.IMG title/boot overlay, VRAM 0x8018EFF0)
 * func_80192FE8 — blob offset 0x3FF8, 0x9C bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the overlay VMA (docs/evidence/ovl_03D2-func_80192FE8/REPORT.md).
 * Scroll step: f06 = f20 + f24, f1C = 0x100 - |f24<<4|, advance f24, fire the f14 callback at f28, done flag at 0x10. */

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
void func_80192FE8(Task *t)
{
    int a;
    int c;
    void (*fn)();
    t->f06 = t->f20 + t->f24;
    a = t->f24 << 4;
    a = (a >= 0) ? 0x100 - a : a + 0x100;
    c = t->f24;
    fn = t->f14;
    t->f1C = a;
    c += (c != 0);
    t->f24 = c;
    if (fn != 0 && c == t->f28) {
        fn(t);
    }
    if (t->f24 >= 0x10) {
        t->f30 = 1;
    }
}
