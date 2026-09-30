/* Phase 5GA: matching C leaf.
 * VRAM 0x8005E8C4 / file 0x4F0C4 / size 0x50 (20 words).
 *
 * Pushes an 8-byte packet onto the arena at D_8009D12C (gp+0x3BC): if the write
 * cursor is still below the arena end D_800A22B0, the cursor stores the two
 * words D_8009D124 (gp+0x3B4) and D_8009D128 (gp+0x3B8) and advances by 8;
 * otherwise it reports overflow through func_800527C0(2).
 *
 * Two spelling details are load-bearing:
 *  - both source words are read into locals *before* the cursor is advanced, so
 *    the two `lw`s are hoisted above the stores the way retail has them (loading
 *    them inline instead leaves the second load after the first store: 7 words);
 *  - D_800A22B0 is declared as a 3-word object, i.e. above the -G8 small-data
 *    threshold, so cc1 emits the absolute `lui`+`addiu` end address retail uses
 *    rather than a gp-relative reference.  Only the address is ever taken.
 */
extern int D_8009D12C;
extern int D_8009D124;
extern int D_8009D128;
extern int D_800A22B0[3];

void func_800527C0(int a0);

void func_8005E8C4(void) {
    int cursor = D_8009D12C;

    if ((unsigned int)cursor < (unsigned int)D_800A22B0) {
        int first = D_8009D124;
        int second = D_8009D128;

        D_8009D12C = cursor + 8;
        *(int *)(cursor + 0) = first;
        *(int *)(cursor + 4) = second;
    } else {
        func_800527C0(2);
    }
}
