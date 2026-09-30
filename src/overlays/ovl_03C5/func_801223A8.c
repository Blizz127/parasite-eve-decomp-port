/* ovl_03C5 (PE.IMG movie-controller overlay, VRAM 0x80120D00)
 * func_801223A8 — blob offset 0x16A8, 0x4C bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the overlay VMA (docs/evidence/ovl_03C5-func_801223A8/REPORT.md).
 * Set D_801223F8 = 1 when the D_800B0DBB byte disagrees with the signed-char argument's truth; the shared-store goto layout is load-bearing (a ternary or early-return form misses by 2). */

extern signed char D_800B0DBB;
extern unsigned char D_801223F8;
void func_801223A8(signed char a0)
{
    if (a0) {
        if (D_800B0DBB == 0) {
            goto set;
        }
    } else if (D_800B0DBB != 0) {
set:
        D_801223F8 = 1;
    }
}
