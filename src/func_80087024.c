/*
 * func_80087024 — store 0xF1 into D_800BCD80, then jal func_8008CBA8.
 * VRAM 0x80087024, file 0x77824, 11 words / 0x2C. Twin of func_80086FF8
 * (0xF0). era -O2 -G0. 7 direct jal sites.
 */
extern int D_800BCD80;
extern void func_8008CBA8(void);

void func_80087024(void)
{
    D_800BCD80 = 0xF1;
    func_8008CBA8();
}
