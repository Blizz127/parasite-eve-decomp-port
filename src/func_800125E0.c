/* Spawn each two-byte descriptor referenced by the relocated boot list.
 * VRAM 0x800125E0 / file 0x2DE0 / size 0x8C (35 words). era -O2 -G8.
 *
 * Natural if+do/while is size-correct but vars=0 (frame 0x20) and swaps
 * $s0/$s1. Pins keep retail $s0=count / $s1=offset. The unused int with
 * an empty m-constraint is the 8-byte vars home (frame 0x28); it emits
 * no instructions.
 */
extern unsigned char **D_8009CE04;

extern void func_80074DC0(int mode);
extern void func_80035038(unsigned char *descriptor, int parent, int active);

void func_800125E0(void) {
    unsigned char **p;
    register unsigned int i asm("$16");      /* retail $s0 count */
    register unsigned int offset asm("$17"); /* retail $s1 byte offset */
    int unused;

    func_80074DC0(0);
    p = D_8009CE04;
    i = 0;
    if (**p != 0) {
        offset = 1;
        do {
            func_80035038(*p + offset, 0, 1);
            p = D_8009CE04;
            i++;
            offset += 2;
        } while (i < **p);
    }
    asm volatile("" : : "m"(unused));
}
