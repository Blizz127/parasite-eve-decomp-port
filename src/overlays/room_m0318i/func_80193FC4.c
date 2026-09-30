/* room_m0318i — func_80193FC4, blob offset 0x4FDC, 0x1A0 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl9 2026-09-28).
 * Two-state particle tick (rand jitter / draw). First draft. */

typedef struct { short x, y, z, pad; } SV;
extern int D_800E27EC;
extern char D_801995FC[];
extern unsigned short D_800F336C;
extern short D_800F336A;
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern int func_80071A54();
extern void func_800CF3AC();
extern unsigned short func_80077AA4();
extern void func_800CEE20();

int func_80193FC4(int a0, short *a1)
{
    SV w;
    SV v;
    int s;
    int u;

    switch (a0) {
    case 1:
        a1[0] += (func_80071A54() & 7) - 3;
        a1[2] += (func_80071A54() & 7) - 3;
        a1[1] += func_80071A54() & 3;
        if (D_800E27EC >= 0x20) {
            return 1;
        }
        break;
    case 2:
        v.x = 0;
        v.y = 0;
        v.pad = 0;
        v.z = D_800E27EC * 12;
        func_800CF3AC(D_801995FC, &w, D_800E27EC);
        s = (D_800E27EC << 7) + 0x1000;
        u = D_800E1204[D_800F336C];
        if (D_800F336C == 4 && D_800F3428 != 0) {
            u += 4;
        }
        func_800CEE20(a1, &v, s, s, D_800F336A * (D_800E27EC / 6 + 2) + 0xA0, func_80077AA4(0x30, u), 1, 0x80, &w);
        break;
    }
    return 0;
}
