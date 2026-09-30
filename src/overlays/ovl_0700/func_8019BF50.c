/* ovl_0700 (PE.IMG map-exit overlay, VRAM 0x8018EFF0)
 * func_8019BF50 — blob offset 0xCF60, 0x3C bytes. Era default profile era_o2_g0;
 * LINK_EXACT at the overlay VMA (docs/evidence/ovl_0700-func_8019BF50/REPORT.md).
 * func_800752AC(D_8019C9C0[1], 0x1000), then func_8019BF8C(D_8019C9C0). */

extern int *D_8019C9C0;
extern void func_800752AC();
extern void func_8019BF8C();
void func_8019BF50(void)
{
    func_800752AC(D_8019C9C0[1], 0x1000);
    func_8019BF8C(D_8019C9C0);
}
