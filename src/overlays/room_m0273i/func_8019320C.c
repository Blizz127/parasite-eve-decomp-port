/* room_m0273i — func_8019320C, blob offset 0x4224, 0x194 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl13 2026-09-28).
 * Two-pass func_800D004C drawer: short x/y loop scalars, (unsigned)b >> 5 for retail's srl, SV copy declared before v[4], pad[2]. */

typedef struct { short x, y, z, w; } SV;
extern int D_800E27EC;
extern short D_800966EC[];
extern short D_800966EE[];
extern unsigned short D_800942EC;
extern char D_8019AB70[];
extern void func_800D004C();

int func_8019320C(int a0, SV *a1)
{
    SV c;
    short v[4];
    short t;
    int a;
    int b;
    short x;
    short y;
    short i;
    int pad[2];

    if (a0 == 1) {
        if (D_800E27EC >= 8) {
            return 1;
        }
    } else if (a0 == 2) {
        t = D_800E27EC - 1;
        a = *(short *)((char *)D_800966EC + ((t << 9) & 0x3E00));
        b = *(short *)((char *)D_800966EE + ((t << 9) & 0x3E00));
        v[0] = 0x400;
        v[2] = t * 170;
        v[1] = 0;
        v[3] = -1;
        c = *a1;
        x = a * 2 + 0x1000;
        y = (unsigned int)b >> 5;
        for (i = 0; i < 2; i++) {
            func_800D004C(&c, 0x80, 0x80, 0xC, v, x, x, D_8019AB70, &a1[1], y, 1);
            y = y >> 1;
            x = x * 3 / 2;
            c.y = D_800942EC;
        }
    }
    return 0;
}
