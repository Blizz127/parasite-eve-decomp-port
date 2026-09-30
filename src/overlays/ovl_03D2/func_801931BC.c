/* ovl_03D2 (PE.IMG title/boot overlay, VRAM 0x8018EFF0)
 * func_801931BC — blob offset 0x41CC, 0x44 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the overlay VMA (docs/evidence/ovl_03D2-func_801931BC/REPORT.md).
 * Find the D_801D1370 task with id 2 and install func_80193200 as its handler. The named int id local is load-bearing: it lets ASPSX's delay-slot shape (li in the beqz slot) come out of cc1. */

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
extern Task *D_801D1370;
extern void func_80193200();
void func_801931BC(void)
{
    Task *p;
    int id = 2;
    for (p = D_801D1370; p != 0; p = p->next) {
        if (p->f2C == id) break;
    }
    p->f0C = func_80193200;
}
