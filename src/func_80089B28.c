/* Set the display-state enable bit.
 * VRAM 0x80089B28 / file 0x7A328 / size 0x20.
 */
extern unsigned int D_8009D2C4;

void func_80089B28(void) {
    D_8009D2C4 |= 0x100;
}
