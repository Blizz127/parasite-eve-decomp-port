/* room_m0273i — func_8019353C, blob offset 0x4554, 0x1B4 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl11 2026-09-28).
 * Two-ring sprite drawer (func_800D004C x2, scale*3/2, alpha>>1); table word w loaded before the scale division, shifted after; pad[2] frame. */

typedef struct { short vx, vy, vz, pad; } SVECTOR;
extern int D_800E27EC;
extern short D_800942EC;
extern char D_8019AB70[];
extern short D_800966EC[];
extern void func_800D004C();

int func_8019353C(int a0, SVECTOR *a1)
{
    SVECTOR a;
    SVECTOR b;
    short t;
    short s;
    short r;
    short i;
    int w;
    int pad[2];

    if (a0 == 1) {
        if (D_800E27EC >= 8) {
            return 1;
        }
    } else if (a0 == 2) {
        t = D_800E27EC - 1;
        w = *(int *)((char *)D_800966EC + ((t << 9) & 0x3E00));
        s = ((*(short *)((char *)D_800966EC + ((t << 9) & 0x3E00)) * 2 + 0x1000) * a1->pad) / 4096;
        r = w >> 21;
        b.vx = 0x400;
        b.vz = t * 170;
        b.vy = 0;
        b.pad = -1;
        a = *a1;
        for (i = 0; i < 2; i++) {
            func_800D004C(&a, 0x80, 0x80, 0xC, &b, s, s, D_8019AB70, a1 + 1, r, 1);
            r >>= 1;
            s = s * 3 / 2;
            a.vy = D_800942EC;
        }
    }
    return 0;
}
