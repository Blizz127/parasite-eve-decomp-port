/* room_m0123i — func_80194F68, blob offset 0x5F80, 0x1AC bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl13 2026-09-28).
 * func_800CF3AC + CEE20 sprite: rodata SV local, (EC << 10) / 24, separate r local for func_80077DC4's result (callee-saved order 14->0). */

typedef struct { short x, y, z, w; } SV;
extern SV D_8018F1F8;
extern int D_800E27EC;
extern unsigned short D_800F336C;
extern short D_800F336A;
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern char D_80195624[];
extern int func_80077DC4();
extern unsigned short func_80077AA4();
extern void func_800CF3AC();
extern void func_800CEE20();

int func_80194F68(int a0, short *a1)
{
    SV w;
    SV c = D_8018F1F8;
    int s;
    int r;
    int u;

    switch (a0) {
    case 1:
        if (a1[4] != 0) {
            return 0;
        }
        a1[5]++;
        a1[1] -= 3;
        if (a1[5] >= 0x18) {
            return 1;
        }
        break;
    case 2:
        if (a1[4] != 0) {
            return 0;
        }
        func_800CF3AC(D_80195624, &w, a1[5]);
        r = func_80077DC4((D_800E27EC << 10) / 24);
        c.z = D_800E27EC << 5;
        u = D_800E1204[D_800F336C];
        s = r / 2 + 0x800;
        if (D_800F336C == 4 && D_800F3428 != 0) {
            u += 4;
        }
        func_800CEE20(a1, &c, s, s, D_800F336A + 0xDC, func_80077AA4(0x30, u), 1, 0x80, &w);
        break;
    default:
        return 0;
    }
    return 0;
}
