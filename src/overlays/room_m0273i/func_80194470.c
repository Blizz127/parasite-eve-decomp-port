/* room_m0273i — func_80194470, blob offset 0x5488, 0x138 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl13 2026-09-28).
 * k==4 sprite (li-before, k loaded early): t copied to r through an asm output barrier (retail addu s0,v1), pinned four $3 + asm volatile("") before the D_800E1204 read. */

extern int D_800E27EC;
extern unsigned short D_800F336C;
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern short D_800966EC[];
extern unsigned short func_80077AA4();
extern void func_800CEE20();

int func_80194470(int a0, short *a1)
{
    int t;
    int r;
    int s;
    int k;
    int u;
    register int four asm("$3");

    if (a0 == 1) {
        if (D_800E27EC >= 9) {
            return 1;
        }
    } else if (a0 == 2) {
        t = D_800E27EC - 1;
        k = D_800F336C;
        asm("" : "=r"(r) : "0"(t));
        s = *(short *)((char *)D_800966EC + ((t << 9) & 0x3E00)) * 2 + 0x1000;
        four = 4;
        asm volatile("");
        u = D_800E1204[k];
        if (k == four && D_800F3428 != 0) {
            u += 9;
        } else {
            u += 5;
        }
        func_800CEE20(a1, 0, (short)s, (short)s, 0x66, func_80077AA4(0, u), 1,
                      (short)*(int *)((char *)D_800966EC + ((r << 10) & 0x3C00)) >> 6, 0);
    }
    return 0;
}
