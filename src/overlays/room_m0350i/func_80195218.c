/* room_m0350i — func_80195218, blob offset 0x6230, 0x160 bytes. Flags -O2 -G0 + MASPSX_NARROW_SHIFTED_WORD_LOAD (K3);
 * LINK_EXACT at the target VMA (lane ovl13 2026-09-28).
 * k==4 sprite (func_800CEE20) with t>=0 guard; *(int *)p >> 21 narrowed by K3. */

extern int D_800E27EC;
extern unsigned short D_800F336C;
extern short D_800F336A;
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern short D_800966EC[];
extern short D_800966EE[];
extern char D_8019A3D0[];
extern unsigned short func_80077AA4();
extern void func_800CEE20();

int func_80195218(int a0, short *a1)
{
    short v[4];
    int t;
    int s;
    int k;
    int u;

    if (a0 == 1) {
        if (D_800E27EC >= 0x10) {
            return 1;
        }
    } else if (a0 == 2) {
        t = D_800E27EC - 2;
        if (t >= 0) {
            v[0] = 0x400;
            v[1] = a1[3];
            v[2] = 0x400;
            v[3] = 1;
            k = D_800F336C;
            u = D_800E1204[k];
            s = *(short *)((char *)D_800966EE + ((t << 8) & 0x3F00));
            if (k == 4 && D_800F3428 != 0) {
                u += 4;
            }
            func_800CEE20(a1, v, s, s * 2, D_800F336A * a1[4] + 0x40, func_80077AA4(0, u), 1,
                          *(int *)((char *)D_800966EC + ((t << 8) & 0x3F00)) >> 21, D_8019A3D0);
        }
    }
    return 0;
}
