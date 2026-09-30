/* ovl_0457 (PE.IMG credits/XA overlay, VRAM 0x80120D00)
 * func_801247F4 — blob offset 0x3AF4, 0xA8 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the overlay VMA (docs/evidence/ovl_0457-func_801247F4/REPORT.md).
 * Frame flip: DrawSync(0), VSync(2), SetDispMask(1), then draw/put the D_8009CDDC buffer's env pair (D_800BCE80 / D_800BCDC8 rows) and toggle D_8009CDDC. */

extern int D_8009CDDC;
extern unsigned char D_800BCE80[][20];
extern unsigned char D_800BCDC8[][92];
extern unsigned char *D_80172CB4;
extern void func_80074DC0();
extern void func_80073A44();
extern void func_80074A44();
extern void func_800755F0();
extern void func_800754E4();
void func_801247F4(void)
{
    func_80074DC0(0);
    func_80073A44(2);
    func_80074A44(1);
    func_800755F0(D_800BCE80[D_8009CDDC]);
    func_800754E4(D_80172CB4 + 0x3C, D_800BCDC8[D_8009CDDC]);
    D_8009CDDC ^= 1;
}
