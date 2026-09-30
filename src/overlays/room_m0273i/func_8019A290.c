/* room_m0273i — func_8019A290, blob offset 0xB2A8, 0x108 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl7 2026-09-27).
 * Effect-sprite spawn via func_800D004C; w>>21 word read narrowed by maspsx K3b (MASPSX_NARROW_SHIFTED_REG_LOAD). */

typedef struct { short x, y, z; } SV3;
extern int D_800E27EC;
extern unsigned short D_800942EC;
extern int D_800966EC[];
extern char D_8019ACCC[];
extern char D_8019AB70[];
extern char D_8019ACC8[];
extern void func_800D004C();

int func_8019A290(int a0, short **a1)
{
    SV3 s;
    int i;
    short *p;
    short w;

    if (a0 == 1) {
        if (D_800E27EC >= 0x10) {
            return 1;
        }
    } else {
        if (a0 != 2) {
            return 0;
        }
        i = D_800E27EC - 1;
        p = (short *)&D_800966EC[(i << 6) & 0xFC0];
        w = p[0] * 2 + 0x800;
        s.x = (*a1)[0];
        s.y = D_800942EC;
        s.z = (*a1)[2];
        func_800D004C(&s, 0xC0, 0xC0, 0xA, D_8019ACCC + (i & 1) * 8, w, w,
                      D_8019AB70, D_8019ACC8, *(int *)p >> 21, 1);
    }
    return 0;
}
