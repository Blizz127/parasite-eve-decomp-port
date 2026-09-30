/* ovl_0457 (PE.IMG credits/XA overlay, VRAM 0x80120D00)
 * func_801255A4 — blob offset 0x48A4, 0x88 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the overlay VMA (docs/evidence/ovl_0457-func_801255A4/REPORT.md).
 * CD shutdown sequence: command 8, poll command 9, drain func_8007F72C/func_8007F778, func_800824C8(0), re-register func_8003E91C via func_80073D24. */

extern void func_80080D5C();
extern int func_80080DC4();
extern int func_8007F72C();
extern int func_8007F778();
extern void func_800824C8();
extern void func_80073D24();
extern void func_8003E91C();
int func_801255A4(void)
{
    func_80080D5C(8, 0, 0);
    while (func_80080DC4(9, 0, 0) == 0) {
    }
    do {
        while (func_8007F72C() != 1) {
        }
    } while (func_8007F778() != 0);
    func_800824C8(0);
    func_80073D24(func_8003E91C);
    return 0;
}
