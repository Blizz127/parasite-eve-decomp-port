/* Phase 5FT: dynamic bit setter (era -O2 -G0).
 * The $v0/$v1 pins reproduce the retail address/mask allocation.
 */
extern unsigned int D_800C0E24;

void func_8005184C(unsigned int bit) {
    register unsigned int *ptr asm("$2") = &D_800C0E24;
    register unsigned int mask asm("$3") = 1;

    *ptr |= mask << bit;
}
