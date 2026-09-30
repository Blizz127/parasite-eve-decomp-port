/* Phase 5GA: matching C leaf.
 * VRAM 0x8005E914 / file 0x4F114 / size 0x54 (21 words).
 *
 * The pop counterpart of func_8005E8C4: while the arena write cursor
 * D_8009D12C (gp+0x3BC) is above the arena start D_800A2270, it steps back eight
 * bytes and publishes the two words it just passed into D_8009D124 (gp+0x3B4)
 * and D_8009D128 (gp+0x3B8); otherwise it reports underflow through
 * func_800527C0(3).
 *
 * As with the push, both words are read into locals before the cursor moves (the
 * two `lw`s stay above the stores), and D_800A2270 is declared as a 3-word object
 * so its address is emitted absolutely rather than gp-relative.  Note the
 * comparison is the mirror of the push: `start < cursor`.
 */
extern int D_8009D12C;
extern int D_8009D124;
extern int D_8009D128;
extern int D_800A2270[3];

void func_800527C0(int a0);

void func_8005E914(void) {
    int cursor = D_8009D12C;

    if ((unsigned int)D_800A2270 < (unsigned int)cursor) {
        int first = *(int *)(cursor - 8);
        int second = *(int *)(cursor - 4);

        D_8009D12C = cursor - 8;
        D_8009D124 = first;
        D_8009D128 = second;
    } else {
        func_800527C0(3);
    }
}
