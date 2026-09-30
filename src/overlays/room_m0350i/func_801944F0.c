/* room_m0350i — func_801944F0, blob offset 0x5508, 0x164 bytes. Flags -O2 -G0 + MASPSX_NARROW_SHIFTED_REG_LOAD (K3b);
 * LINK_EXACT at the target VMA (lane ovl11 2026-09-28).
 * Two-pass drawer (func_800D004C + func_800D0728); sin-table base local tb; *(int *)p >> 21 narrowed by K3b. */

typedef struct { short vx, vy, vz, pad; } SVECTOR;
extern int D_800E27EC;
extern short D_800966EC[];
extern char D_8019A3C8[];
extern char D_8019A4DC[];
extern char D_8019A4E0[];
extern void func_800D004C();
extern void func_800D0728();

int func_801944F0(int a0, int *a1)
{
    SVECTOR v;
    short *p;
    int s;
    char *tb;

    if (a0 == 1) {
        if (D_800E27EC >= 8) {
            return 1;
        }
    } else if (a0 == 2) {
        tb = (char *)D_800966EC;
        p = (short *)(tb + (((D_800E27EC - 1) << 9) & 0x3E00));
        s = p[0];
        v.vx = 0;
        v.vy = 0;
        v.vz = D_800E27EC << 7;
        v.pad = 0;
        func_800D004C(*a1, 0x140, 0x140, 0x10, &v, s, s, D_8019A4DC, D_8019A3C8, *(int *)p >> 21, 1);
        s = (D_800E27EC << 9) + 0x800;
        func_800D0728(*a1, 0x100, 0x200, 0x10, &v, s, s, D_8019A3C8, D_8019A4E0,
                      *(int *)(tb + (((D_800E27EC - 1) << 10) & 0x3C00)) << 16 >> 21, 1);
    }
    return 0;
}
