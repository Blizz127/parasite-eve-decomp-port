/* room_m0273i — func_8019A4CC, blob offset 0xB4E4, 0x108 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl7 2026-09-27).
 * Effect-sprite spawn via func_800D0728 (8019A290 family); table word arg w computed before the SV stores. */

typedef struct { short x, y, z, pad; } SV;
extern int D_800E27EC;
extern int D_800966EC[];
extern char *D_800F32D0;
extern char D_8019ACDC[];
extern char D_8019ACE0[];
extern void func_800D0728();

int func_8019A4CC(int a0, int *a1)
{
    SV s;
    int i;
    int h;
    int w;

    if (a0 == 1) {
        if (D_800E27EC >= 0x10) {
            return 1;
        }
    } else {
        if (a0 != 2) {
            return 0;
        }
        i = D_800E27EC - 1;
        h = *(short *)&D_800966EC[(i << 6) & 0xFC0] * 2 + 0x1000;
        w = (short)D_800966EC[(i << 7) & 0xF80] >> 5;
        s.x = 0;
        s.y = *(unsigned short *)(*(char **)(D_800F32D0 + 8) + 0x3A);
        s.z = i << 8;
        s.pad = 1;
        func_800D0728(*a1, 0x20, 0x60, 8, &s, h, h, D_8019ACDC, D_8019ACE0,
                      w, 1);
    }
    return 0;
}
