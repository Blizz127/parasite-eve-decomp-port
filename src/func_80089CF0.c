/* Set the display-state enable bit.
 * VRAM 0x80089CF0 / file 0x7A4F0 / size 0x20.
 */
extern unsigned int D_8009D2C4;

void func_80089CF0(void) {
    D_8009D2C4 |= 0x100;
}
