/* ovl_03D2 (PE.IMG title/boot overlay, VRAM 0x8018EFF0)
 * func_80192C9C — blob offset 0x3CAC, 0x4C bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the overlay VMA (docs/evidence/ovl_03D2-func_80192C9C/REPORT.md).
 * Twin of ovl_03C5 func_801223A8 with target D_801D0DC0. */

extern signed char D_800B0DBB;
extern unsigned char D_801D0DC0;
void func_80192C9C(signed char a0)
{
    if (a0) {
        if (D_800B0DBB == 0) {
            goto set;
        }
    } else if (D_800B0DBB != 0) {
set:
        D_801D0DC0 = 1;
    }
}
