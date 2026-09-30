/* ovl_0700 (PE.IMG map-exit overlay, VRAM 0x8018EFF0)
 * func_8019BF8C — blob offset 0xCF9C, 0x38 bytes. Era default profile era_o2_g0;
 * LINK_EXACT at the overlay VMA (docs/evidence/ovl_0700-func_8019BF8C/REPORT.md).
 * Seeds *a0 from the EXE timer word D_800B0E4C; +90000 unless a0 is the overlay's own D_8019C1F8. */

extern int D_8019C1F8;
extern int D_800B0E4C;
void func_8019BF8C(int *a0)
{
    if (a0 == &D_8019C1F8) {
        *a0 = D_800B0E4C;
    } else {
        *a0 = D_800B0E4C + 90000;
    }
}
