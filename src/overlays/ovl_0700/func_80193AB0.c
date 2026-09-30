/* ovl_0700 (PE.IMG map-exit overlay, VRAM 0x8018EFF0)
 * func_80193AB0 — blob offset 0x4AC0, 0xAC bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the overlay VMA (docs/evidence/ovl_0700-func_80193AB0/REPORT.md).
 * While the D_8019C058 countdown is positive, add D_801EA268[i] xyz to D_8019CAA8[i] (10 entries). Unsigned counter gives retail's sltu pointer bound. */

typedef struct { int x, y, z, w; } V4;
extern short D_8019C058;
extern V4 D_8019CAA8[];
extern V4 D_801EA268[];
void func_80193AB0(void)
{
    unsigned int i;
    if (D_8019C058 > 0) {
        D_8019C058--;
        for (i = 0; i < 10; i++) {
            D_8019CAA8[i].x += D_801EA268[i].x;
            D_8019CAA8[i].y += D_801EA268[i].y;
            D_8019CAA8[i].z += D_801EA268[i].z;
        }
    }
}
