/* Relocate a loaded boot table in place: word 0 and the a0[1] entries that
 * follow the count are self-relative offsets; skip if already relocated.
 * VRAM 0x80012574 / file 0x2D74 / size 0x6C (27 words). era -O2 -G8
 * (D_8009CE04 is 0x94($gp)).
 */
extern unsigned int *D_8009CE04;

unsigned int *func_80012574(unsigned int *a0) {
    unsigned int i;
    unsigned int v;

    v = a0[0];
    D_8009CE04 = a0;
    if (v > 0x80000000) {
        return a0;
    }
    a0[0] = (unsigned int)a0 + v;
    for (i = 0; i < a0[1]; i++) {
        a0[i + 2] += (unsigned int)a0;
    }
    return D_8009CE04;
}
