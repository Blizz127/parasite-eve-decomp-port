/* ovl_0700 (PE.IMG map-exit overlay, VRAM 0x8018EFF0)
 * func_80191DE8 — blob offset 0x2DF8, 0x48 bytes. Era default profile era_o2_g0;
 * LINK_EXACT at the overlay VMA (docs/evidence/ovl_0700-func_80191DE8/REPORT.md).
 * Seeds the D_800B0DCE..D_800B0DD2 byte/short block; D_800B0DCF = a0 ? 0x40 : 0x10. */

extern unsigned char D_800B0DCE;
extern unsigned char D_800B0DCF;
extern short D_800B0DD0;
extern short D_800B0DD2;
void func_80191DE8(int a0)
{
    if (a0 == 0) {
        D_800B0DCE = 0x10;
        D_800B0DCF = 0x10;
    } else {
        D_800B0DCE = 0x10;
        D_800B0DCF = 0x40;
    }
    D_800B0DD0 = 0;
    D_800B0DD2 = 0xFFF;
}
