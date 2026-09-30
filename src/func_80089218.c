/* func_80089218 — VRAM 0x80089218, size 0x38, file 0x79A18-0x79A50.
 *
 * Walks 0x18 records starting at a0+0xF0 with stride 0x11C; whenever the
 * record's first word equals a1 it is rewritten with 0x18.
 *
 * era_o2_g0. Written as a do/while over the offset form `a0 + 0xF0` so the
 * `i = 0` and `li a2,0x18` initialisations precede the pointer bump, matching
 * retail 0x00001821 / 0x24060018 / 0x248400F0.
 */
void func_80089218(unsigned char *a0, unsigned int a1) {
    unsigned int i;

    i = 0;
    do {
        if (a1 == *(unsigned int *)(a0 + 0xF0))
            *(unsigned int *)(a0 + 0xF0) = 0x18;
        a0 += 0x11C;
        i++;
    } while (i < 0x18);
}
