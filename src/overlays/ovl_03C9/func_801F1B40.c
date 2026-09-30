/* ovl_03C9 (PE.IMG handler-module overlay, VRAM 0x801ED7F8)
 * func_801F1B40 — blob offset 0x4348, 0x10 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the overlay VMA (docs/evidence/ovl_03C9-func_801F1B40/REPORT.md).
 * Returns &D_801F1F28 (the handler module's shared state block); six byte-identical accessors in a row. */

extern unsigned char D_801F1F28;
void *func_801F1B40(void)
{
    return &D_801F1F28;
}
