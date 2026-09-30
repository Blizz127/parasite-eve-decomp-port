/* room_m0350i — func_80193A80, blob offset 0x4A98, 0x14C bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl13 2026-09-28).
 * k==4 sprite: o = k << 1 then asm volatile("") then pinned four $3 (ovl7 park 27w -> 0). */

extern int D_800E27EC;
extern unsigned short D_800F336C;
extern short D_800F336A;
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern int D_800966EC[];
extern unsigned short func_80077AA4();
extern void func_800CEE20();

int func_80193A80(int a0, int *a1)
{
    int c;
    int k;
    int o;
    int v;
    register int four asm("$3");

    if (a0 == 1) {
        if (D_800E27EC >= 0x10) {
            return 1;
        }
    } else if (a0 == 2) {
        c = *(short *)&D_800966EC[((D_800E27EC - 1) << 6) & 0xFC0] * 3;
        k = D_800F336C;
        o = k << 1;
        asm volatile("");
        four = 4;
        v = *(unsigned short *)((char *)D_800E1204 + o);
        if (k == four && D_800F3428 != 0) {
            v += 8;
        } else {
            v += 4;
        }
        func_800CEE20(*a1, 0, c, c, D_800F336A * 6 + 0x80, func_80077AA4(0, v), 1,
                      (short)D_800966EC[((D_800E27EC - 1) << 7) & 0xF80] >> 5, 0);
    }
    return 0;
}
