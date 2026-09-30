/* ovl_03D2 (PE.IMG title/boot overlay, VRAM 0x8018EFF0)
 * func_80192C48 — blob offset 0x3C58, 0x54 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the overlay VMA (docs/evidence/ovl_03D2-func_80192C48/REPORT.md).
 * Twin of ovl_03C5 func_80122354. */

extern unsigned char D_800B0DBA;
extern void func_800870F0();
extern void func_8010C0D8();
extern void func_8007A2A4();
extern void func_80080DC4();
void func_80192C48(void)
{
    unsigned char *p = &D_800B0DBA;
    *p = *p - 1;
    func_800870F0(0);
    func_8010C0D8(0);
    func_8007A2A4();
    func_80080DC4(9, 0, 0);
}
