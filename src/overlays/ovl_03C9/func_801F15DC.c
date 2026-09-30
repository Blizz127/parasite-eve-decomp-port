/* ovl_03C9 (PE.IMG handler-module overlay, VRAM 0x801ED7F8)
 * func_801F15DC — blob offset 0x3DE4, 0x1B4 bytes. Profile era_o2_g0 (default).
 * Levers: (1) `register int four asm("$3")` compare constant (see func_801F1070);
 * (2) the signed /32 is written out on `cy` (cy = f(t); if (cy < 0) cy += 31; c = cy >> 5):
 * reusing cy gives the temp retail's $a1 and makes c's sra non-birthing for sched1, so it
 * lands in the lhu 0xC load-delay slot. Evidence: docs/evidence/ovl8-lane-2026-09-28/REPORT.md */
typedef struct { short x, y, w, h; } RECT;
typedef struct {
    unsigned char pad[0x8];
    short x, y, w, h;
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
extern unsigned char D_801F1F34;
int func_801F15DC(int mode, Fx *o)
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
        size = func_80077CF4(t) + 0x1000;
        cy = func_80077DC4(t);
        if (cy < 0) cy += 31;
        c = cy >> 5;
        k = D_800F336C;
        r.x = o->x;
        r.y = o->y;
        r.w = o->w;
        r.w = o->step * 50;
        r.h = 1;
        cy = D_800E1204[k];
        four = 4;
        if (k == four && D_800F3428 != 0) {
            cy += 7;
        } else {
            cy += 3;
        }
        func_800CEE20(o, &r, size, size, 0x44, func_80077AA4(0, cy), 1, c, &D_801F1F34);
        break;
    }
    return 0;
}
