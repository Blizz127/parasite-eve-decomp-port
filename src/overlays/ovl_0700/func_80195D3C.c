/* ovl_0700 (PE.IMG map-exit overlay, VRAM 0x8018EFF0)
 * func_80195D3C — blob offset 0x6D4C, 0x110 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the overlay VMA (docs/evidence/ovl_0700-func_80195D3C/REPORT.md).
 * While the D_8019C040 counter is >= 2: step it, advance D_8019C041, and sample two path points (func_8018F55C) into D_8019C810 / D_8019C330. */

typedef struct { int x, y, z, w; } V4;
typedef struct { short vx, vy, vz, pad; } SVECTOR;
typedef struct { int x, y, z; } V3;
extern unsigned char D_8019C040;
extern unsigned char D_8019C041;
extern unsigned char D_8019C042;
extern V3 D_8019C810;
extern V3 D_8019C330;
extern unsigned char D_801D0260;
extern int func_8006EC6C();
extern int func_8018F55C();
void func_80195D3C(void)
{
    V4 v;
    SVECTOR s;
    if (D_8019C040 >= 2) {
        D_8019C040--;
        D_8019C041++;
        func_8018F55C(D_8019C041 << 8, D_8019C042 + 1, func_8006EC6C(&D_801D0260, 2), &v, &s);
        D_8019C810.x = v.x;
        D_8019C810.y = v.y;
        D_8019C810.z = v.z;
        func_8018F55C(D_8019C041 << 8, D_8019C042, func_8006EC6C(&D_801D0260, 2), &v, &s);
        D_8019C330.x = v.x;
        D_8019C330.y = v.y;
        D_8019C330.z = v.z;
    }
}
