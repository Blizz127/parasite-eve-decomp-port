/* ovl_0457 (PE.IMG credits/XA overlay, VRAM 0x80120D00)
 * func_80125980 — blob offset 0x4C80, 0xC0 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the overlay VMA (docs/evidence/ovl_0457-func_80125980/REPORT.md).
 * XA stream callback: exit (func_800719E4, declared noreturn) on a CD position mismatch; mode 1 feeds the next 2 KB sector slot (D_80172CF8 ring of 8), else bumps D_80172CFA. Volatile slot pointer keeps the store out of the jump delay slot. */

extern unsigned short D_800B0DD4;
extern unsigned short D_80172CF8;
extern volatile unsigned short D_80172CFA;
extern unsigned char *D_80172D00;
extern int func_8007F72C();
extern int func_8007F7A8();
extern void func_800719E4() __attribute__((noreturn));
extern void func_80080AE4();
void func_80125980(unsigned char mode)
{
    volatile unsigned short *p;
    if (func_8007F72C() == 1) {
        if (func_8007F7A8() != D_800B0DD4) {
            func_800719E4(1);
        }
    }
    if (mode == 1) {
        p = &D_80172CF8;
        func_80080AE4(D_80172D00 + (*p << 11), 0x200);
        *p = (*p + 1) & 7;
    } else {
        D_80172CFA++;
    }
}
