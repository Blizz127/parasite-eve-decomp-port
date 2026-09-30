/* room_m0273i — func_8019A068, blob offset 0xB080, 0x138 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl11 2026-09-28).
 * Two-case room event handler; empty case 2 reproduces the slti/beqz tree. */

extern int *D_800F33E0;
extern int *D_800F32D0;
extern unsigned char D_8019AE68;
extern int D_800E27EC;
extern void func_80194B5C();
extern int func_800CE560();
extern short *func_800CE610();

int func_8019A068(int a0)
{
    short *p;
    int *q;
    int t;

    switch (a0) {
    case 0:
        D_8019AE68 = 0;
        return func_800CE560(((int **)D_800F33E0)[2], 8, 5, func_80194B5C);
    case 2:
        break;
    case 1:
        if (D_8019AE68 != 0) {
            return 2;
        }
        t = ((unsigned char *)((int **)D_800F32D0)[2])[0xE];
        if (t != 0x10 && t != 8) {
            D_8019AE68 = 1;
            return 2;
        }
        if (D_800E27EC & 3) {
            return 0;
        }
        p = func_800CE610(((int **)D_800F33E0)[2]);
        if (p != 0) {
            q = *(int **)((char *)((int **)D_800F32D0)[2] + 0x238);
            p[0] = q[0x594 / 4];
            p[1] = q[0x598 / 4];
            p[2] = q[0x59C / 4];
        }
        break;
    }
    return 0;
}
