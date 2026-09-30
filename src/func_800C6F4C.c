/* Phase 5GB: matching C leaf.
 * VRAM 0x800C6F4C / file 0xB774C / size 0x54 (21 words).
 *
 * The mirror of func_800C6EF8: copies the word array from the fixed buffer
 * D_800E2370 back *into* the record at `record + *(u16 *)(record + 8)`, again
 * re-reading the `*(u16 *)(record + 0xA)` count each iteration, with the same
 * in-body `i++` placement that fixes the body ordering.
 */
extern int D_800E2370[];

void func_800C6F4C(unsigned char *record) {
    int i = 0;
    int *src = D_800E2370;
    int *dst = (int *)(record + *(unsigned short *)(record + 8));

    while (i < *(unsigned short *)(record + 0xA)) {
        i++;
        *dst++ = *src++;
    }
}
