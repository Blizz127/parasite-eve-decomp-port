/* ovl_03C9 (PE.IMG handler-module overlay, VRAM 0x801ED7F8)
 * func_801F1B60 — blob offset 0x4368, 0x38 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the overlay VMA (docs/evidence/ovl_03C9-func_801F1B60/REPORT.md).
 * Mode-1 probe: returns 1 while the D_801F1F3A flag is clear, else clears it and returns 0. */

extern short D_801F1F3A;
int func_801F1B60(int a0)
{
    if (a0 == 1) {
        if (D_801F1F3A == 0) {
            return 1;
        }
        D_801F1F3A = 0;
    }
    return 0;
}
