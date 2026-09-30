/* room_m0350i — func_80195B64, blob offset 0x6B7C, 0x17C bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl13 2026-09-28).
 * k==4 sprite over a rotated 3-vector (w>>16 cos, (short)w>>5); first try. */

extern int D_800E27EC;
extern unsigned short D_800F336C;
extern short D_800F336A;
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern short D_800966EC[];
extern char *D_800F32D0;
extern char D_8019A56C[];
extern unsigned short func_80077AA4();
extern void func_800CEE20();

int func_80195B64(int a0, short *a1)
{
    short v[4];
    int i;
    int w;
    int c;
    int s;
    int *b;
    int u;

    if (a0 == 1) {
        if (D_800E27EC >= 8) {
            return 1;
        }
    } else if (a0 == 2) {
        w = *(int *)((char *)D_800966EC + ((D_800E27EC << 9) & 0x3E00));
        c = w >> 16;
        s = (short)w >> 5;
        b = (int *)(*(char **)(*(char **)(D_800F32D0 + 8) + 0x238) + 0x434);
        for (i = 0; i < 3; i++) {
            v[i] = a1[i] * c / 4096 + b[i];
        }
        u = D_800E1204[D_800F336C];
        if (D_800F336C == 4 && D_800F3428 != 0) {
            u += 4;
        }
        func_800CEE20(v, 0, a1[3], a1[3], D_800F336A + 0xD8, func_80077AA4(0x20, u), 1, s, D_8019A56C);
    }
    return 0;
}
