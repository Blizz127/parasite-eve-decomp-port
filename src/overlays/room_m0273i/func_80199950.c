/* room_m0273i — func_80199950, blob offset 0xA968, 0x140 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl13 2026-09-28).
 * k==4 sprite (li-before): s0/s1 as SHORT locals (retail's sext-at-use), asm volatile("") then o = k << 1 then pinned four $3 (park 4w -> 0). */

extern int D_800E27EC;
extern unsigned short D_800F336C;
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern int D_800966EC[];
extern unsigned short func_80077AA4();
extern void func_800CEE20();

int func_80199950(int a0, int *a1)
{
    short t;
    short s1;
    short s0;
    int k;
    int u;
    int o;
    register int four asm("$3");
    int pad[2];

    if (a0 == 1) {
        if (D_800E27EC >= 0x10) {
            return 1;
        }
    } else if (a0 == 2) {
        t = D_800E27EC - 1;
        k = D_800F336C;
        s1 = *(int *)((char *)D_800966EC + ((t << 8) & 0x3F00)) + 0x800;
        s0 = (short)*(int *)((char *)D_800966EC + ((t << 9) & 0x3E00)) >> 6;
        asm volatile("");
        o = k << 1;
        four = 4;
        u = *(unsigned short *)((char *)D_800E1204 + o);
        if (k == four && D_800F3428 != 0) {
            u += 7;
        } else {
            u += 3;
        }
        func_800CEE20(*a1, 0, s1, s1, 0x40, func_80077AA4(0, u), 1, s0, 0);
    }
    return 0;
}
