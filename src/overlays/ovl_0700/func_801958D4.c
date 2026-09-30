/* ovl_0700 (PE.IMG map-exit overlay, VRAM 0x8018EFF0)
 * func_801958D4 — blob offset 0x68E4, 0xC0 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the overlay VMA (docs/evidence/ovl_0700-func_801958D4/REPORT.md).
 * Mode 0: switch the active id (release D_8019C144, set colour 0x80/0x80/0x80, attach) unless already D_8019C140; mode 1: release D_8019C144. */

extern int D_8019C140;
extern int D_8019C144;
extern void func_8003746C(short);
extern void func_80038940();
extern void func_800375E0();
short func_801958D4(short id, unsigned char mode)
{
    short tmp = 0;
    if (mode == 0) {
        if (id != D_8019C140) {
            func_8003746C(D_8019C144);
            func_80038940(id, 0x80, 0x80, 0x80);
            func_800375E0(id, 3, &tmp);
            D_8019C144 = id;
        }
    }
    if (mode == 1) {
        func_8003746C(D_8019C144);
    }
    return id;
}
