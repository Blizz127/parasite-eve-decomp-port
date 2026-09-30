/* ovl_03C9 (PE.IMG handler-module overlay, VRAM 0x801ED7F8)
 * func_801F1070 — blob offset 0x3878, 0x18C bytes. Profile era_o2_g0 (default).
 * Effect handler sibling of func_801F0BA0. Lever: the CLUT compare constant is a
 * hard-register local (register int four asm("$3") = 4) so sched2 can put retail's
 * `li $v1,4` before the D_800E1204 indexed load (a pseudo lands in $v0, which the
 * load's index uses, pinning it after the load). Evidence: docs/evidence/ovl8-lane-2026-09-28/REPORT.md */
typedef struct { short x, y, w, h; } RECT;
typedef struct {
    unsigned char pad[0x10];
    short wait;   /* 0x10 */
    short step;   /* 0x12 */
} Fx;
extern unsigned short D_800F336C;
extern int D_800F3428;
extern unsigned short D_800E1204[];
extern unsigned short func_80077AA4();
extern void func_800CEE20();
extern unsigned char D_801F1D5C;
extern short D_800F3368;
extern short D_800F336A;
extern short D_800F3376;
extern short D_800F3378;
extern int func_80077CF4();
extern int func_80077DC4();
extern void func_800CF3AC();
extern unsigned char D_801F1F30;
int func_801F1070(int mode, Fx *o)
{
    int t;
    int size;
    int c;
    int cy;
    int k;
    register int four asm("$3");
    RECT r;

    switch (mode) {
    case 1:
        if (o->wait != 0) break;
        if (++o->step < 12) break;
        return 1;
    case 2:
        if (o->wait != 0) return 0;
        t = (o->step << 10) / 12;
        size = func_80077DC4(t) + 0x800;
        c = func_80077DC4(t) / 32;
        r.x = 0;
        r.y = 0;
        r.w = o->step * 32;
        r.h = 0;
        k = D_800F336C;
        cy = D_800E1204[k];
        four = 4;
        if (k == four && D_800F3428 != 0) {
            cy += 7;
        } else {
            cy += 3;
        }
        func_800CEE20(o, &r, size, size, 0x42, func_80077AA4(0, cy), 1, c, &D_801F1F30);
        break;
    }
    return 0;
}
