/* ovl_0700 (PE.IMG map-exit overlay, VRAM 0x8018EFF0)
 * func_80195E4C — blob offset 0x6E5C, 0x120 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the overlay VMA (docs/evidence/ovl_0700-func_80195E4C/REPORT.md).
 * Seed the path walker: sample points a0+1 and a0 into D_8019C810 / D_8019C330, D_8019C040 = count-2, clear the step state. */

typedef struct { int x, y, z, w; } V4;
typedef struct { short vx, vy, vz, pad; } SVECTOR;
typedef struct { int x, y, z; } V3;
extern unsigned char D_8019C040;
extern unsigned char D_8019C041;
extern unsigned char D_8019C042;
extern short D_8019C02C;
extern short D_8019C050;
extern short D_8019C054;
extern V3 D_8019C810;
extern V3 D_8019C330;
extern unsigned char D_801D0260;
extern int func_8006EC6C();
extern int func_8018F55C();
void func_80195E4C(short a0, int a1, int a2, int a3)
{
    V4 v;
    SVECTOR s;
    func_8018F55C(a3, a0 + 1, func_8006EC6C(&D_801D0260, 2), &v, &s);
    D_8019C054 = 0;
    D_8019C810.x = v.x;
    D_8019C810.y = v.y;
    D_8019C810.z = v.z;
    D_8019C040 = func_8018F55C(a3, a0, func_8006EC6C(&D_801D0260, 2), &v, &s) - 2;
    D_8019C050 = 0;
    D_8019C041 = 0;
    D_8019C042 = a0;
    D_8019C02C = 0;
    D_8019C330.x = v.x;
    D_8019C330.y = v.y;
    D_8019C330.z = v.z;
}
