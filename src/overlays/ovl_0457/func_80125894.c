/* ovl_0457 (PE.IMG credits/XA overlay, VRAM 0x80120D00)
 * func_80125894 — blob offset 0x4B94, 0x58 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the overlay VMA (docs/evidence/ovl_0457-func_80125894/REPORT.md).
 * Poll func_8007F418(0, 0) until 2, then issue CD command 0xD with the two-byte parameter {1, a0}. */

extern int func_8007F418();
extern int func_80080DC4();
int func_80125894(unsigned char a0)
{
    unsigned char result[8];
    unsigned char param[8];
    param[0] = 1;
    param[1] = a0;
    while (func_8007F418(0, 0) != 2) {
    }
    func_80080DC4(0xD, param, result);
    return 0;
}
