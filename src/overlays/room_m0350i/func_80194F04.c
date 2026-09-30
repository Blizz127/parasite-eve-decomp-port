/* room_m0350i — func_80194F04, blob offset 0x5F1C, 0x160 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl13 2026-09-28).
 * k==4 sprite (li-before): two asm volatile("") fences (after the m load, after four = 4) order the D_800E27EC read and li 4 (park 8w -> 0). */

typedef struct { short x, y, w, h; } RECT;
extern int D_800E27EC;
extern unsigned short D_800F336C;
extern short D_800F336A;
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern int D_800966EC[];
extern char D_8019A4E8[];
extern unsigned short func_80077AA4();
extern void func_800CEE20();

int func_80194F04(int a0, short *a1)
{
    RECT r;
    int u;
    int m;
    int o;
    register int four asm("$3");
    int t;
    unsigned short us;

    if (a0 == 1) {
        if (D_800E27EC >= 0x10) {
            return 1;
        }
        a1[2] -= 0x10;
    } else if (a0 == 2) {
        r = **(RECT **)a1;
        r.y += a1[2];
        m = D_800F336C;
        asm volatile("");
        t = D_800E27EC - 1;
        four = 4;
        asm volatile("");
        o = m << 1;
        u = *(unsigned short *)((char *)D_800E1204 + o);
        if (m == four && D_800F3428 != 0) {
            u += 6;
        } else {
            u += 2;
        }
        us = func_80077AA4(0, u);
        func_800CEE20(&r, 0, 0x2000, 0x2000, D_800F336A * (t >> 1), us, 1,
                      (short)*(int *)((char *)D_800966EC + ((t << 9) & 0x3E00)) >> 5, D_8019A4E8);
    }
    return 0;
}
