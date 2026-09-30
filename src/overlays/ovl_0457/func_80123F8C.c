/* ovl_0457 (PE.IMG credits/XA overlay, VRAM 0x80120D00)
 * func_80123F8C — blob offset 0x328C, 0xA8 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the overlay VMA (docs/evidence/ovl_0457-func_80123F8C/REPORT.md).
 * Point the two stream windows D_80172CB4/B8 into D_80011610 by buffer parity D_8009CDDC and the PE.IMG range width (D_80093168 - D_80093166) sectors, then ClearOTag-style func_800752AC(D_80172CB4, 0x10). Both globals volatile (retail reloads CB4 after both stores); $2/$3/$4/$5 pins reproduce retail's in-place adds. */

extern int D_8009CDDC;
extern unsigned short D_80093168;
extern unsigned short D_80093166;
extern unsigned char *D_80011610;
extern unsigned char *volatile D_80172CB4;
extern unsigned char *volatile D_80172CB8;
extern void func_800752AC();
void func_80123F8C(void)
{
    register int a asm("$4");
    register int b asm("$5");
    register int off asm("$2");
    register unsigned char *base asm("$3");
    if (D_8009CDDC != 0) {
        a = 0x581E0;
        b = 0x560D0;
        off = (D_80093168 - D_80093166) << 11;
    } else {
        a = 0x581A0;
        b = 0x54000;
        off = (D_80093168 - D_80093166) << 11;
    }
    a = off + a;
    base = D_80011610;
    off = off + b;
    D_80172CB4 = base + a;
    base = base + off;
    D_80172CB8 = base;
    func_800752AC(D_80172CB4, 0x10);
}
