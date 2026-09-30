/* room_m0318i — func_801939AC, blob offset 0x49C4, 0x1B4 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl9 2026-09-28).
 * Two-state rising-sprite tick. v.z store before v.pad (retail order; 5 -> 0). */

typedef struct { short x, y, z, pad; } SV;
extern int D_800E27EC;
extern unsigned short D_800F336C;
extern short D_800F336A;
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern int func_80077CF4();
extern unsigned short func_80077AA4();
extern void func_800CEE20();

int func_801939AC(int a0, short *a1)
{
    SV p;
    SV v;
    int s;
    int u;

    switch (a0) {
    case 1:
        a1[1] += a1[3];
        if (D_800E27EC < 0x13) {
            a1[3] -= 1;
        }
        if (D_800E27EC >= 0x18) {
            return 1;
        }
        break;
    case 2:
        p.x = a1[0];
        p.y = a1[1];
        p.z = a1[2];
        v.x = 0;
        v.y = 0;
        v.z = a1[1] + a1[0] + (D_800E27EC << 5);
        v.pad = 0;
        s = func_80077CF4((D_800E27EC << 11) / 24) / 128;
        u = D_800E1204[D_800F336C];
        if (D_800F336C == 4 && D_800F3428 != 0) {
            u += 4;
        }
        func_800CEE20(&p, &v, 0x1000, 0x1000, D_800F336A * (D_800E27EC / 6) + 0x60, func_80077AA4(0x10, u), 1, s, 0);
        break;
    }
    return 0;
}
