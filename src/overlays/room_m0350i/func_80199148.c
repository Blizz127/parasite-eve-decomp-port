/* room_m0350i — func_80199148, blob offset 0xA160, 0x158 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl9 2026-09-28).
 * k == 4 sprite family (animated variant). Levers: short t for the a1[9] >> 1 phase (60 -> 59 region), a1[1] -= a1[4] >> 1 before a1[4] += a1[5] (retail reloads a1[4]; 59 -> 0 w/ the rest), four/u/o pins + us local as func_801937B4, int z[2] zeroed at entry. */

typedef struct { short x, y, z, pad; } SV;
extern unsigned short D_800F336C;
extern short D_800F336A;
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern unsigned short func_80077AA4();
extern void func_800CEE20();

int func_80199148(int a0, short *a1)
{
    int z[2];
    register int u asm("$5");
    int m;
    register int o asm("$3");
    unsigned short us;
    short t;
    register int four asm("$4");

    z[0] = 0;
    z[1] = 0;
    if (a0 == 1) {
        t = a1[9] >> 1;
        a1[8] = t;
        if (t >= 8) {
            return 1;
        }
        if (t >= 4) {
            a1[6] -= 8;
        }
        a1[9] += 1;
        a1[1] -= a1[4] >> 1;
        a1[4] += a1[5];
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
        func_800CEE20(a1, 0, a1[7], a1[7], D_800F336A * a1[8], us, 1, a1[6], 0);
    }
    return 0;
}
