/* room_m0012i — func_8018F004, blob offset 0x1C, 0x208 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl 2026-09-27).
 * event callback: jitter / spawn func_800CEE20 with sine-scaled size (func_80077DC4), 8-room body */

typedef struct { short x, y, z, pad; int w; } T;
extern int D_800E27EC;
extern unsigned short D_800F336C;
extern short D_800F336A;
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern int func_80071A54();
extern int func_80077DC4();
extern int func_80077AA4();
extern void func_800CEE20();

int func_8018F004(int a0, T *a1)
{
    int s0;
    int s2;
    int n;
    unsigned short u;

    switch (a0) {
    case 1:
        a1->y -= 2;
        a1->x += (func_80071A54() & 7) - 3;
        a1->z += (func_80071A54() & 7) - 3;
        if (D_800E27EC >= 0xE) {
            return 1;
        }
        break;
    case 2:
        s2 = func_80077DC4((D_800E27EC << 10) / 14) / 50;
        s0 = ((func_80077DC4((D_800E27EC << 10) / 14) / 2 + 0x800) * a1->w) / 4096 + 0x400;
        u = D_800F336C;
        n = D_800E1204[u];
        if (u == 4 && D_800F3428 != 0) {
            n += 4;
        }
        func_800CEE20(a1, 0, s0, s0, D_800F336A * ((D_800E27EC << 3) / 14) + 0xE0,
                      func_80077AA4(0x40, n) & 0xFFFF, 1, s2, 0);
        break;
    default:
        return 0;
    }
    return 0;
}
