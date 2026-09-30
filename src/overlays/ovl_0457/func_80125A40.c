/* ovl_0457 (PE.IMG credits/XA overlay, VRAM 0x80120D00)
 * func_80125A40 — blob offset 0x4D40, 0xB0 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the overlay VMA (docs/evidence/ovl_0457-func_80125A40/REPORT.md).
 * Every 32nd VSync: stop (func_800870F0) when the D_80172CF0 counter passes D_80172CEC, fade out (func_801258EC) when it trails D_80172CE8-300, else schedule func_80125AF0. Pointer-to-volatile keeps the counter base in $a0 with two loads; '*p > CEC' operand order is load-bearing. */

extern int D_80172CF0;
extern int D_80172CEC;
extern int D_80172CE8;
extern int D_80172CE4;
extern int func_80073A44();
extern void func_800870F0();
extern void func_801258EC();
extern void func_8007EE84();
extern void func_80125AF0();
void func_80125A40(void)
{
    volatile int *p;
    if ((func_80073A44(-1) & 0x1F) == 0) {
        p = &D_80172CF0;
        if (*p > D_80172CEC) {
            func_800870F0(0);
            D_80172CE4 = 1;
        } else if (*p < D_80172CE8 - 300) {
            func_801258EC(200, 0x1B);
        } else {
            func_8007EE84(0x11, 0, func_80125AF0, 0);
        }
    }
}
