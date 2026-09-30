/* Phase 5DB: one-hundred-third matching C leaf.
 * VRAM 0x8008D820 / file 0x7E020 / size 0x24.
 * word memcpy countdown: word copy with a2>>=2 countdown; register asm for $v0.
 */
void func_8008D820(unsigned int *arg0, unsigned int *arg1, unsigned int arg2) {
    register unsigned int v0 asm("$2");

    arg2 >>= 2;
    do {
        v0 = *arg0;
        arg0++;
        arg2--;
        *arg1 = v0;
        arg1++;
    } while (arg2);
}
