/* VRAM 0x80085174 / file 0x75974 / size 0x34.
 * Wait while D_8009D24C is 1 after observing it equal 1. */
extern volatile unsigned int D_8009D24C;
void func_80085174(void) {
    if (D_8009D24C == 1) {
        unsigned int one = 1;
        while (D_8009D24C == one) {
        }
    }
}
