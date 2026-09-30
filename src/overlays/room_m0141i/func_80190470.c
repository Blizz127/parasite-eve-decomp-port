/* room_m0141i — func_80190470, blob offset 0x1488, 0xDC bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl5 2026-09-27).
 * Trail-history shift (5x V4 + 5 shorts) then velocity>>8 integrate; non-struct S() short accesses and statement order are load-bearing (19->0). */

typedef struct { short x, y, z, pad; } V4;
extern short D_800942EC;
#define S(o) (*(short *)((char *)v + (o)))

void func_80190470(int a0, unsigned char *a1, V4 *v)
{
    int i;

    for (i = 5; i != 0; i--) {
        v[i] = v[i - 1];
        ((short *)((char *)v + 0x78))[i] = ((short *)((char *)v + 0x78))[i - 1];
    }
    S(0) += S(0x30) >> 8;
    S(2) += S(0x32) >> 8;
    S(4) += S(0x34) >> 8;
    S(0x32) += *(int *)((char *)v + 0x60);
    if (S(2) > D_800942EC) {
        S(0x78) = 0;
    }
    if (*(short *)(a1 + 2) == 0x3C) {
        a1[1] = 2;
    }
}
