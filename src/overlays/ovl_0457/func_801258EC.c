/* ovl_0457 (PE.IMG credits/XA overlay, VRAM 0x80120D00)
 * func_801258EC — blob offset 0x4BEC, 0x94 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the overlay VMA (docs/evidence/ovl_0457-func_801258EC/REPORT.md).
 * Issue a CD read at position D_80172CE8 (func_80080B44 to BCD, func_8007F0C8), mirror it into D_80172CF0 and count in D_80172CF4. */

extern int D_80172CE8;
extern volatile int D_80172CF0;
extern volatile int D_80172CF4;
extern void func_80080B44();
extern unsigned char func_8007F0C8();
unsigned char func_801258EC(unsigned char a0, unsigned char a1)
{
    unsigned char loc[8];
    int *p = &D_80172CE8;
    unsigned char r;
    func_80080B44(*p, loc);
    r = func_8007F0C8(a0, loc, a1, 0, -1);
    D_80172CF0 = *p;
    D_80172CF4++;
    return r;
}
