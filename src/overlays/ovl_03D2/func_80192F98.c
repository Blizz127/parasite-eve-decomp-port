/* ovl_03D2 (PE.IMG title/boot overlay, VRAM 0x8018EFF0)
 * func_80192F98 — blob offset 0x3FA8, 0x50 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the overlay VMA (docs/evidence/ovl_03D2-func_80192F98/REPORT.md).
 * Arm task t (f06 = 180, clear f14/f0C/f20) and set task id 7's f1C = 0x100. Same named-id lever as func_801931BC. */

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
void func_80192F98(Task *t)
{
    Task *p;
    int id = 7;
    t->f06 = 180;
    t->f14 = 0;
    t->f0C = 0;
    t->f20 = 0;
    for (p = D_801D1370; p != 0; p = p->next) {
        if (p->f2C == id) {
            break;
        }
    }
    p->f1C = 0x100;
}
