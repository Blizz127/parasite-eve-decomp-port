/* ovl_0700 (PE.IMG map-exit overlay, VRAM 0x8018EFF0)
 * func_801939B0 — blob offset 0x49C0, 0x100 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the overlay VMA (docs/evidence/ovl_0700-func_801939B0/REPORT.md).
 * Arm the D_8019C058 countdown (0x20) and compute per-step deltas D_801EA268[i] = ((coords<<16) - D_8019CAA8[i]) >> 5 for 10 entries. */

typedef struct { int x, y, z, w; } V4;
typedef struct { short vx, vy, vz, pad; } SVECTOR;
extern short D_8019C058;
extern V4 D_8019CAA8[];
extern V4 D_801EA268[];
extern unsigned char D_801D0260;
extern int func_8006EC6C();
extern void func_8018F55C();
void func_801939B0(short a0)
{
    unsigned int i;
    V4 v;
    SVECTOR s;
    D_8019C058 = 0x20;
    for (i = 0; i < 10; i++) {
        func_8018F55C(i << 8, a0 + 0x40, func_8006EC6C(&D_801D0260, 2), &v, &s);
        D_801EA268[i].x = ((v.x << 16) - D_8019CAA8[i].x) >> 5;
        D_801EA268[i].y = ((v.y << 16) - D_8019CAA8[i].y) >> 5;
        D_801EA268[i].z = ((v.z << 16) - D_8019CAA8[i].z) >> 5;
    }
}
