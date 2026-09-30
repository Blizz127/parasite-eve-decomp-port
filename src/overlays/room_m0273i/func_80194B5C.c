/* room_m0273i — func_80194B5C, blob offset 0x5B74, 0x1F8 bytes. Flags -O2 -G0 + MASPSX_NARROW_SHIFTED_REG_LOAD (K3b);
 * LINK_EXACT at the target VMA (lane ovl11 2026-09-28).
 * Two-pass sprite drawer; ix = &D_8019AE98 pointer local, b declared before a, *(int *)p >> 21 narrowed by K3b. */

typedef struct { short vx, vy, vz, pad; } SVECTOR;
extern int D_800E27EC;
extern short D_800942EC;
extern short D_8019AE98;
extern char D_8019AB70[];
extern int D_8019ABFC[];
extern short D_800966EC[];
extern void func_800D004C();

int func_80194B5C(int a0, SVECTOR *a1)
{
    SVECTOR b;
    SVECTOR a;
    short t;
    short s;
    short *p;
    short *ix;

    if (a0 == 1) {
        if (D_800E27EC >= 0x10) {
            return 1;
        }
    } else if (a0 == 2) {
        t = D_800E27EC - 1;
        p = (short *)((char *)D_800966EC + ((t << 8) & 0x3F00));
        s = p[0] * 10240 / 4096 + 0x800;
        a = *a1;
        ix = &D_8019AE98;
        b.vx = 0;
        b.vy = 0;
        b.vz = t * 204;
        b.pad = 0;
        func_800D004C(&a, 0xA0, 0xA0, 0xA, &b, s, s, D_8019AB70, &D_8019ABFC[*ix], *(int *)p >> 21, 1);
        b.vx = 0x400;
        b.pad = 1;
        a.vy = D_800942EC;
        func_800D004C(&a, 0x100, 0x100, 0xA, &b, s >> 1, s >> 1, &D_8019ABFC[*ix], D_8019AB70, *(int *)p >> 21, 1);
    }
    return 0;
}
