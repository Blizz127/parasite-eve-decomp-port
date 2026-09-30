/*
 * func_80086FF8 — store 0xF0 into D_800BCD80, then jal func_8008CBA8.
 * VRAM 0x80086FF8, file 0x777F8, 11 words / 0x2C. Twin of func_80087024
 * (0xF1). era -O2 -G0. 8 direct jal sites.
 */
extern int D_800BCD80;
extern void func_8008CBA8(void);

void func_80086FF8(void)
{
    D_800BCD80 = 0xF0;
    func_8008CBA8();
}
