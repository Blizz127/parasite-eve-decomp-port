/* room_m0350i — func_80197A04, blob offset 0x8A1C, 0x194 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl13 2026-09-28).
 * k==4 sprite over a scaled 3-vector ((0x1000 - sin)*2); first try. */

extern int D_800E27EC;
extern unsigned short D_800F336C;
extern short D_800F336A;
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern short D_800966EC[];
extern char *D_800F32D0;
extern char D_8019A634[];
extern unsigned short func_80077AA4();
extern void func_800CEE20();

int func_80197A04(int a0, short *a1)
{
    short v[4];
    int i;
    int c;
    int *b;
    int u;

    if (a0 == 1) {
        if (D_800E27EC >= 8) {
            return 1;
        }
    } else if (a0 == 2) {
        c = (0x1000 - *(short *)((char *)D_800966EC + (((D_800E27EC - 1) << 9) & 0x3E00))) * 2;
        b = (int *)(*(char **)(*(char **)(D_800F32D0 + 8) + 0x238) + 0xF4);
        for (i = 0; i < 3; i++) {
            v[i] = a1[i] * c / 4096 + b[i];
        }
        u = D_800E1204[D_800F336C];
        if (D_800F336C == 4 && D_800F3428 != 0) {
            u += 4;
        }
        func_800CEE20(v, 0, 0x1400, 0x1400, D_800F336A * 2 + 0xD8, func_80077AA4(0x20, u), 1,
                      (short)*(int *)((char *)D_800966EC + (((D_800E27EC - 1) << 9) & 0x3E00)) >> 5, D_8019A634);
    }
    return 0;
}
