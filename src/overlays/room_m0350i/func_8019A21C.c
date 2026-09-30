/* room_m0350i — func_8019A21C, blob offset 0xB234, 0xFC bytes. Flags -O2 -G0 + MASPSX_NARROW_SHIFTED_REG_LOAD (K3b);
 * LINK_EXACT at the target VMA (lane ovl9 2026-09-28).
 * Two-state tick (wait / draw). Levers: K3b (lh 2 + sra 5 for the word >> 21); byte index t = (k - 1) << 8 as a named local before b = D_800966EC, masked with 0x3F00 at the use — puts the la between the sll and the andi as retail (3 -> 0). */

typedef struct { short x, y, z, pad; } SV;
extern int D_800E27EC;
extern int D_800966EC[];
extern char D_8019A3C8[];
extern char D_8019A628[];
extern void func_800D004C();

int func_8019A21C(int a0, void *a1)
{
    SV s;
    short *p;
    int w;
    int *b;
    int k, t;

    if (a0 == 1) {
        if (D_800E27EC >= 0x10) {
            return 1;
        }
    } else {
        if (a0 != 2) {
            return 0;
        }
        t = (D_800E27EC - 1) << 8;
        b = D_800966EC;
        p = (short *)((char *)b + (t & 0x3F00));
        w = p[0];
        s.x = 0x400;
        s.y = 0;
        s.pad = 1;
        s.z = (D_800E27EC << 12) / 20;
        func_800D004C(a1, 0x180, 0x180, 10, &s, w, w, D_8019A3C8, D_8019A628, *(int *)p >> 21, 1);
    }
    return 0;
}
