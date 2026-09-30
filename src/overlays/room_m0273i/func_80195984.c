/* room_m0273i — func_80195984, blob offset 0x699C, 0x24C bytes. Flags -O2 -G0 + MASPSX_NARROW_SHIFTED_WORD_LOAD (K3);
 * LINK_EXACT at the target VMA (lane ovl13 2026-09-28).
 * Two-pass func_800CEE20 (k==4 li-after twice): short s, us local for the first call and inline func_80077AA4 for the second; first *(int)>>21 narrowed by K3. */

typedef struct { short x, y, z, w; } SV;
extern int D_800E27EC;
extern unsigned short D_800F336C;
extern short D_800F336A;
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern short D_800966EC[];
extern short D_8019ACC0[];
extern unsigned short func_80077AA4();
extern void func_800CEE20();

int func_80195984(int a0, SV *a1)
{
    SV c;
    int t;
    short s;
    int k;
    int u;
    unsigned short us;

    if (a0 == 1) {
        if (D_800E27EC >= 0x10) {
            return 1;
        }
        ((short *)a1)[3] += ((short *)a1)[4];
        ((short *)a1)[4] += 2;
    } else if (a0 == 2) {
        t = D_800E27EC - 1;
        k = D_800F336C;
        s = *(short *)((char *)D_800966EC + ((t << 8) & 0x3F00)) * 2 + 0x800;
        c = *a1;
        u = D_800E1204[k];
        if (k == 4 && D_800F3428 != 0) {
            u += 4;
        }
        us = func_80077AA4(0x10, u);
        func_800CEE20(&c, 0, s, s, D_800F336A * (t / 2) + 0x80, us, 1,
                      *(int *)((char *)D_800966EC + ((t << 8) & 0x3F00)) >> 21, 0);
        c.y -= ((short *)a1)[3];
        u = D_800E1204[D_800F336C];
        if (D_800F336C == 4 && D_800F3428 != 0) {
            u += 4;
        }
        func_800CEE20(&c, 0, 0x2000, 0x2000, D_800F336A * D_8019ACC0[t / 4], func_80077AA4(0x40, u), 1,
                      (short)*(int *)((char *)D_800966EC + ((t << 9) & 0x3E00)) >> 5, 0);
    }
    return 0;
}
