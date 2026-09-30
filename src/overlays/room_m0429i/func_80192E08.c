/* room_m0429i — func_80192E08, blob offset 0x3E20, 0x194 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl9 2026-09-28).
 * Two-state sprite tick (fade step / draw with sin-cos scale). First draft: switch(a0) with case 1/2 + break (a compare chain inverts the dispatch), int u for the D_800E1204 entry (unsigned short adds andi 0xFFFF). */

typedef struct { short x, y, w, h; } RECT;
extern RECT D_8018F1CC;
extern int D_8019B668;
extern unsigned short D_800F336C;
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern int func_80077CF4();
extern int func_80077DC4();
extern unsigned short func_80077AA4();
extern void func_800CEE20();

int func_80192E08(int a0, char *a1)
{
    RECT r = D_8018F1CC;
    int c;
    int s;
    int t;
    int u;

    switch (a0) {
    case 1:
        if (*(short *)(a1 + 8) != 0) {
            return 0;
        }
        *(short *)(a1 + 0xA) += 1;
        *(short *)(a1 + 2) -= 0x80;
        if (*(short *)(a1 + 0xA) >= 8) {
            return 1;
        }
        break;
    case 2:
        if (*(short *)(a1 + 8) != 0) {
            return 0;
        }
        r.w = 0x400;
        t = *(short *)(a1 + 0xA) << 7;
        c = func_80077CF4(t);
        s = D_8019B668 * func_80077DC4(t) / 4096;
        u = D_800E1204[D_800F336C];
        if (D_800F336C == 4 && D_800F3428 != 0) {
            u += 4;
        }
        func_800CEE20(a1, &r, 0x1000, c, 0x88, func_80077AA4(0x90, u), 1, s, 0);
        break;
    }
    return 0;
}
