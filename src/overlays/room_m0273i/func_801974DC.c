/* room_m0273i — func_801974DC, blob offset 0x84F4, 0x16C bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl13 2026-09-28).
 * k==4 sprite (li-after, no pins); pad[2] frame slack; (short) word >> 5. */

extern int D_800E27EC;
extern unsigned short D_800F336C;
extern short D_800F336A;
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern short D_800966EC[];
extern char D_8019AD70[];
extern unsigned short func_80077AA4();
extern void func_800CEE20();

int func_801974DC(int a0, short *a1)
{
    int t;
    int s;
    int w;
    int k;
    int u;
    int pad[2];

    if (a0 == 1) {
        if (D_800E27EC >= 0x20) {
            return 1;
        }
        a1[1] -= a1[3];
        a1[3]++;
    } else if (a0 == 2) {
        t = D_800E27EC - 1;
        k = D_800F336C;
        w = (short)*(int *)((char *)D_800966EC + ((t << 8) & 0x3F00)) >> 5;
        u = D_800E1204[k];
        s = *(short *)((char *)D_800966EC + ((t << 7) & 0x3F80)) + 0x2000;
        if (k == 4 && D_800F3428 != 0) {
            u += 4;
        }
        func_800CEE20(a1, 0, s, s, D_800F336A * (t / 4) + 0xC8, func_80077AA4(0x10, u), 1, w, D_8019AD70);
    }
    return 0;
}
