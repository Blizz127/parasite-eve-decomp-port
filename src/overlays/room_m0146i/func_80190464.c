/* room_m0146i (PE.IMG room m0146i chunk 2, VRAM 0x8018EFE8)
 * func_80190464 — blob offset 0x147c, 0xdc bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0141i func_80190470; C re-targeted by symbol address
 * (docs/evidence/room_m0146i-ports-2026-09-23/REPORT.md). */

typedef struct { short x, y, z, pad; } V4;
extern short D_800942EC;
#define S(o) (*(short *)((char *)v + (o)))

void func_80190464(int a0, unsigned char *a1, V4 *v)
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
