/*
 * func_8006D24C — VRAM 0x8006D24C, size 0x6C, file 0x5DA4C-0x5DAB8.
 *
 * Clears six signed-byte state globals to -1, masks 0xF0 out of the
 * D_800B0CD8 flag word, then calls func_80086FF8. Returns nothing.
 *
 * era -O2 -G0. Store order is retail's: DB5, DB4, DB7, DB6, DB3, DB2.
 */
extern signed char D_800B0DB2, D_800B0DB3, D_800B0DB4, D_800B0DB5, D_800B0DB6, D_800B0DB7;
extern unsigned int D_800B0CD8;

void func_80086FF8(void);

void func_8006D24C(void) {
    D_800B0DB5 = -1;
    D_800B0DB4 = -1;
    D_800B0DB7 = -1;
    D_800B0DB6 = -1;
    D_800B0DB3 = -1;
    D_800B0DB2 = -1;
    D_800B0CD8 &= ~0xF0;
    func_80086FF8();
}
