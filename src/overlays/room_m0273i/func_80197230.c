/* room_m0273i — func_80197230, blob offset 0x8248, 0x130 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl13 2026-09-28).
 * k==4 sprite (li-before): short c (sext at use), pinned four $3 + asm volatile("") after t (park 15w -> 0). */

extern int D_800E27EC;
extern unsigned short D_800F336C;
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern int D_800966EC[];
extern unsigned short func_80077AA4();
extern void func_800CEE20();

int func_80197230(int a0, void *a1)
{
    int t;
    short c;
    int k;
    int u;
    register int four asm("$3");

    if (a0 == 1) {
        if (D_800E27EC >= 8) {
            return 1;
        }
    } else if (a0 == 2) {
        t = D_800E27EC - 1;
        four = 4;
        asm volatile("");
        c = *(int *)((char *)D_800966EC + ((t << 9) & 0x3E00)) + 0x800;
        k = D_800F336C;
        u = D_800E1204[k];
        if (k == four && D_800F3428 != 0) {
            u += 9;
        } else {
            u += 5;
        }
        func_800CEE20(a1, 0, c, c, 0x66, func_80077AA4(0, u), 1,
                      (short)*(int *)((char *)D_800966EC + ((t << 10) & 0x3C00)) >> 5, 0);
    }
    return 0;
}
