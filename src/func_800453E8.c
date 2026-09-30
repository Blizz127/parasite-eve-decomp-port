/*
 * func_800453E8 — window callback (retail 0x800453E8).
 *
 * VRAM 0x800453E8 / file 0x35BE8 / size 0x44 (17 words), head of a mid-35698
 * carve. func_800452C0 precedes it; func_8004542C follows at 0x35C2C.
 *
 * Sets up a window with func_8005E8A4(0x3C, 0x12), forwards the gp-relative
 * word at D_8009CFA0 + 0x0C (0x23C($gp)) plus 0x4D to func_8005EB64, tears
 * down with func_8005E8A4(-0x38, -0xE) and returns func_80056FB8().
 *
 * Build: era -O2 -G8. The gp-relative `lw $a0, 0x23C($gp)` only appears when
 * the containing data symbol is declared as a scalar: `extern int
 * D_8009CFA0;` emits `.extern D_8009CFA0, 4`, so maspsx resolves the +0x0C
 * field as a small-data load. Declaring the array (`extern int D_8009CFA0[]`)
 * or a sized array makes cc1 emit lui/`lw %lo` absolute instead.
 */
extern int D_8009CFA0;
extern int func_8005E8A4(int, int);
extern int func_8005EB64(int);
extern int func_80056FB8(void);

int func_800453E8(void) {
    func_8005E8A4(0x3C, 0x12);
    func_8005EB64(*(int *)((char *)&D_8009CFA0 + 12) + 0x4D);
    func_8005E8A4(-0x38, -0xE);
    return func_80056FB8();
}
