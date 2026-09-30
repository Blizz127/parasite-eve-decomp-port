/* ovl_0700 (PE.IMG map-exit overlay, VRAM 0x8018EFF0)
 * func_80190D08 — blob offset 0x1D18, 0x34 bytes. Era default profile era_o2_g0;
 * LINK_EXACT at the overlay VMA (docs/evidence/ovl_0700-func_80190D08/REPORT.md).
 * Unlink the record (func_80191834), then func_80191678(its leading short). */

extern void func_80191834();
extern void func_80191678();
void func_80190D08(short *a0)
{
    func_80191834(a0);
    func_80191678(*a0);
}
