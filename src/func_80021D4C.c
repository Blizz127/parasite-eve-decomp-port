/* VRAM 0x80021D4C / file 0x1254C / size 0x94.
 * Command-cancellation walk over D_800BE834[index]: while the 8-bit index
 * (D_8009D1D4) is below the 8-bit command count (D_8009CE3C), sign-extend the
 * 16-bit table word at stride 8, and for action-3 in [0,0x180) call
 * func_80053D2C(action-3).  Then clear both bytes and drop bit 8 of
 * D_8009D1A0.
 *
 * Retail keeps the count in a reloaded local: the entry load feeds the test
 * directly (the count is re-read after every call), and the loop is the
 * explicit goto/test form so cc1 emits retail's j-to-bottom-test rotation with
 * the count load outside the test block.
 *
 * era -O2 -G8 + MASPSX_THREE_WORD_SYMBOL_STORE=1 (indexed lui/addu/lhu-%lo);
 * MASPSX_FORCE_ABSOLUTE_SYMBOLS=D_8009D1A0 keeps that word absolute while the
 * two bytes stay gp-relative. */
extern unsigned char D_8009CE3C;
extern unsigned char D_8009D1D4;
extern unsigned int D_8009D1A0;
extern unsigned char D_800BE834[];

extern int func_80053D2C(int);

void func_80021D4C(void)
{
    unsigned int index = D_8009D1D4;
    unsigned char count = D_8009CE3C;
    unsigned short action;

    goto test;
body:
    action = *(unsigned short *)(D_800BE834 + (index & 0xFF) * 8);
    if ((unsigned int)(action - 3) < 0x180U) {
        func_80053D2C((short)action - 3);
    }
    index += 1;
    count = D_8009CE3C;
test:
    if ((index & 0xFF) < count) {
        goto body;
    }
    D_8009CE3C = 0;
    D_8009D1D4 = 0;
    D_8009D1A0 &= ~0x100;
}
