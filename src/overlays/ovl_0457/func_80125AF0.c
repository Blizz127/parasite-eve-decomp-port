/* ovl_0457 (PE.IMG credits/XA overlay, VRAM 0x80120D00)
 * func_80125AF0 — blob offset 0x4DF0, 0x58 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the overlay VMA (docs/evidence/ovl_0457-func_80125AF0/REPORT.md).
 * On event 2, D_80172CFC = func_80080C48(a1 + 5) and mirror it into D_80172CF0 when nonzero. volatile on D_80172CFC is load-bearing (retail reloads it twice). */

extern volatile int D_80172CFC;
extern int D_80172CF0;
extern int func_80080C48();
void func_80125AF0(unsigned char a0, int a1)
{
    if (a0 == 2) {
        D_80172CFC = func_80080C48(a1 + 5);
        if (D_80172CFC != 0) {
            D_80172CF0 = D_80172CFC;
        }
    }
}
