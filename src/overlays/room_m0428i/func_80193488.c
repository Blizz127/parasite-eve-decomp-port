/* room_m0428i — func_80193488, blob offset 0x44A0, 0x168 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl13 2026-09-28).
 * k==4 sprite: short c (sext at use), o = k << 1 + asm volatile + pinned four $3, separate us local. */

extern int D_800E27EC;
extern unsigned short D_800F336C;
extern short D_800F336A;
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern int D_800966EC[];
extern unsigned short func_80077AA4();
extern void func_800CEE20();

int func_80193488(int a0, short *a1)
{
    short c;
    int k;
    int o;
    int v;
    int t;
    unsigned short us;
    register int four asm("$3");

    if (a0 == 1) {
        if (D_800E27EC >= 0x10) {
            return 1;
        }
        a1[1] -= a1[3];
        a1[3]++;
    } else if (a0 == 2) {
        c = a1[4] + *(int *)((char *)D_800966EC + (((D_800E27EC - 1) << 8) & 0x3F00));
        k = D_800F336C;
        o = k << 1;
        asm volatile("");
        four = 4;
        v = *(unsigned short *)((char *)D_800E1204 + o);
        if (k == four && D_800F3428 != 0) {
            v += 6;
        } else {
            v += 2;
        }
        us = func_80077AA4(0, v);
        t = D_800E27EC - 1;
        func_800CEE20(a1, 0, c, c, D_800F336A * ((t >> 1) & 7), us, 1,
                      (short)*(int *)((char *)D_800966EC + ((t << 9) & 0x3E00)) >> 5, 0);
    }
    return 0;
}
