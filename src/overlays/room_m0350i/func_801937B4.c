/* room_m0350i — func_801937B4, blob offset 0x47CC, 0x130 bytes. Flags -O2 -G0 + MASPSX_NARROW_SHIFTED_WORD_LOAD (K3);
 * LINK_EXACT at the target VMA (lane ovl9 2026-09-28).
 * Two-state sprite tick (k == 4 family). Levers: K3 MASPSX_NARROW_SHIFTED_WORD_LOAD (lw SYM($b)+sra 21 -> lh SYM+2 + sra 5; 10 -> 5); pins four asm($4) set before the table read (retail li $a0,4 in the load delay), u asm($5) (table value straight into the call arg), o = m << 1 asm($3) (5 -> 0); separate us for the func_80077AA4 result; (unsigned short) k in case 1. */

extern int D_800E27EC;
extern unsigned short D_800F336C;
extern short D_800F336A;
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern int D_800966EC[];
extern char D_8019A464[];
extern unsigned short func_80077AA4();
extern void func_800CEE20();

int func_801937B4(int a0, short *a1)
{
    register int u asm("$5");
    int t;
    int m;
    register int o asm("$3");
    unsigned short us;
    int k;
    register int four asm("$4");

    if (a0 == 1) {
        if (D_800E27EC >= 0x10) {
            return 1;
        }
        k = (unsigned short)a1[3];
        a1[3] = k + 1;
        a1[1] -= k;
    } else if (a0 == 2) {
        four = 4;
        m = D_800F336C;
        o = m << 1;
        u = *(unsigned short *)((char *)D_800E1204 + o);
        if (m == four && D_800F3428 != 0) {
            u += 6;
        } else {
            u += 2;
        }
        us = func_80077AA4(0, u);
        t = D_800E27EC - 1;
        func_800CEE20(a1, 0, 0x2000, 0x2000, D_800F336A * ((t >> 1) & 7), us, 1,
                      *(int *)((char *)D_800966EC + ((t << 8) & 0x3F00)) >> 21, D_8019A464);
    }
    return 0;
}
