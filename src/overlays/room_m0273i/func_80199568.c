/* room_m0273i — func_80199568, blob offset 0xA580, 0x15C bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl9 2026-09-28).
 * k == 4 family, li-after variant. Levers: a1[3]++ post-increment (not += 1: load order flips; 4 -> 0), int us = (unsigned short)func_80077AA4() (andi in the bgez slot; 36 -> 13). */

extern int D_800E27EC;
extern unsigned short D_800F336C;
extern short D_800F336A;
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern int D_800966EC[];
extern unsigned short func_80077AA4();
extern void func_800CEE20();

int func_80199568(int a0, short *a1)
{
    int u;
    int t;
    int us;

    if (a0 == 1) {
        if (D_800E27EC >= 0x24) {
            return 1;
        }
        if (D_800E27EC >= 4) {
            a1[1] -= a1[3];
            a1[3]++;
        }
    } else if (a0 == 2) {
        t = D_800E27EC - 5;
        if (t < 0) {
            return 0;
        }
        u = D_800E1204[D_800F336C];
        if (D_800F336C == 4 && D_800F3428 != 0) {
            u += 4;
        }
        us = (unsigned short)func_80077AA4(0x10, u);
        func_800CEE20(a1, 0, 0x3000, 0x3000, D_800F336A * (t / 4) + 0xC8, us, 1,
                      (short)*(int *)((char *)D_800966EC + ((t << 8) & 0x3F00)) >> 5, 0);
    }
    return 0;
}
