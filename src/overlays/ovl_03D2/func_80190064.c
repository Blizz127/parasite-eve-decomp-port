/* ovl_03D2 (PE.IMG title/boot overlay, VRAM 0x8018EFF0)
 * func_80190064 — blob offset 0x1074, 0x5FC bytes. Profile era_o2_g0 (default).
 * Title-menu input handler over the D_801D1370 task list (ids 1..7): start button, cursor
 * brightening/dimming, confirm and up/down moves, then latches the pad word in D_801D11B8.
 * Levers: one `static inline Task *find(int id)` for all twelve list walks (null-checked or
 * not, as retail); `find(7)->f06 > t->f06` operand order; the down-move bound as a ternary
 * condition (retail's beqz-to-body / j-to-skip layout). Evidence: docs/evidence/ovl8-lane-2026-09-28/REPORT.md */
typedef struct Task {
    struct Task *next;     /* 0x00 */
    short f04;
    short f06;             /* 0x06 */
    int f08;
    void *f0C;             /* 0x0C handler */
    void *f10;
    void *f14;             /* 0x14 */
    int f18;
    int f1C;               /* 0x1C */
    int f20;               /* 0x20 */
    int f24;               /* 0x24 */
    int f28;
    int f2C;               /* 0x2C id */
    int f30;               /* 0x30 */
} Task;
extern Task *D_801D1370;
extern int D_801D11B8;
extern int D_801D1380;
extern int func_8005E038();
extern void func_800525EC();
extern void func_8005267C();
extern int func_80042770();
extern int func_8003FFCC();
extern void func_801931BC();
extern void func_801930D8();
extern void func_8018F958();
extern void func_8019316C();

static inline Task *find(int id)
{
    Task *p;
    for (p = D_801D1370; p != 0; p = p->next) {
        if (p->f2C == id) break;
    }
    return p;
}

void func_80190064(void)
{
    int pad;
    Task *t;
    int k;

    pad = func_8005E038();
    if ((pad & 0x800) && !(D_801D11B8 & 0x800)) {
        t = find(2);
        if (t != 0 && t->f1C > 0x80) {
            t = find(1);
            if (t->f24 == 0) {
                t->f24 = 1;
                t->f28 = 8;
                t->f14 = func_801931BC;
                D_801D1380 = 0;
                func_800525EC();
            }
        }
    }
    t = find(5);
    if (t != 0 && t->f0C == 0) {
        if (func_80042770(0) || func_80042770(1)) {
            if (t->f06 < 0xC8) {
                t->f06++;
                find(4)->f20 = 0x10;
                D_801D1380 = 0;
            }
            if (t->f06 == 0xC8 && func_8003FFCC()) {
                t = find(6);
                if (t->f20 == 0) {
                    t->f0C = func_801930D8;
                    t->f10 = func_8018F958;
                    t->f20 = 0x14;
                    t->f24 = 1;
                }
            }
        } else if (t->f06 >= 0xB5) {
            t->f06--;
            if (find(7)->f06 > t->f06) {
                find(7)->f06 = t->f06;
            }
            find(4)->f20 = -0x10;
            D_801D1380 = 0;
        }
        if (func_8003FFCC() == 0) {
            t = find(6);
            if (t->f20 == 0x54) {
                t->f0C = func_8019316C;
            }
            if (t->f1C != 0) {
                t->f24 = -1;
                if (find(7)->f06 == 0x8C) {
                    find(7)->f06 = 0xA0;
                }
            }
        }
        t = find(7);
        if (pad & 0x20) {
            if (t->f06 == 0xA0 || t->f06 == 0xB4 || t->f06 == 0xC8 || t->f06 == 0x8C) {
                D_801D1380 = 0x3E9;
                func_800525EC();
            }
        }
        if ((pad & 0x1000) && !(D_801D11B8 & 0x1000)) {
            k = t->f06;
            if (find(6)->f1C == 0x100 ? k >= 0x8D : k >= 0xA1) {
                D_801D1380 = 0;
                t->f06 -= 0x14;
                func_8005267C();
            }
        }
    skip:
        if ((pad & 0x4000) && !(D_801D11B8 & 0x4000)) {
            k = t->f06;
            if (k <= find(5)->f06 - 0x14) {
                D_801D1380 = 0;
                t->f06 += 0x14;
                func_8005267C();
            }
        }
    }
    D_801D11B8 = pad;
}
