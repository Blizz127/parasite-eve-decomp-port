/* ovl_03C9 (PE.IMG handler-module overlay, VRAM 0x801ED7F8)
 * func_801F0BA0 — blob offset 0x33A8, 0x190 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the overlay VMA (docs/evidence/ovl_03C9-func_801F0BA0/REPORT.md).
 * Effect handler (mode, obj): mode 1 counts step to 16 once wait is 0 (returns 1 when done); mode 2 draws the step-scaled sprite through func_800CEE20 with a rcos-style size (func_80077CF4 + 0x800), a func_800CF3AC-built colour buffer, and the CLUT row from D_800E1204[D_800F336C] (+4 for area 4 when D_800F3428). Levers: switch with break (not return 0) keeps the branchy compare; 'o->step * 32' (not << 5) keeps retail's lh; int CLUT row avoids an andi 0xFFFF. */

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
int func_801F0BA0(int mode, Fx *o)
{
    int t;
    int size;
    int cy;
    RECT r;
    unsigned char buf[8];
    switch (mode) {
    case 1:
        if (o->wait != 0) break;
        if (++o->step < 16) break;
        return 1;
    case 2:
        if (o->wait != 0) return 0;
        t = o->step << 6;
        size = func_80077CF4(t) + 0x800;
        func_80077DC4(t);
        r.x = 0;
        r.y = 0;
        r.w = o->step * 32;
        r.h = 0;
        func_800CF3AC(&D_801F1D5C, buf, o->step << 1);
        D_800F3368 = 0x20;
        D_800F336A = 2;
        D_800F3376 = 0x20;
        D_800F3378 = 0x20;
        cy = D_800E1204[D_800F336C];
        if (D_800F336C == 4 && D_800F3428 != 0) {
            cy += 4;
        }
        func_800CEE20(o, &r, size, size, 0x82, func_80077AA4(0x30, cy), 1, 0x80, buf);
        break;
    }
    return 0;
}
