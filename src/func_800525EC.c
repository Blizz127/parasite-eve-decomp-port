/* Phase 5FV: matching C leaf (mid-42D94 carve).
 * VRAM 0x800525EC / file 0x42DEC / size 0x48.
 *
 * Original: menu sound helper.  Loads D_800B0E08, and when it is non-zero calls
 * func_8006DF50(sound, 0x44C, 0x100, 0x80, 0x7F) — the 5th argument goes on the
 * stack, the 4th into $a3 in the jal delay slot.
 *
 * Codegen: retail materialises &D_800B0E08 exactly once into $a0 and reloads
 * through it (test read into $v0, then a second read into $a0 for the call).
 *   - `extern int D_800B0E08;`           folds both reads into one load
 *     (16 word mismatches),
 *   - `extern volatile int D_800B0E08;`  materialises the address twice
 *     (11 mismatches),
 *   - reading through a `volatile int *` derived once from the symbol gives the
 *     single materialisation plus two loads, and is word-exact at -O2 -G0.
 * Verified with tools/analysis/era_link_check.py (era cc1, -O2 -G0): LINK_EXACT,
 * 18/18 words, zero pad.
 */
extern volatile int D_800B0E08;

void func_8006DF50(int a0, int a1, int a2, int a3, int a4);

void func_800525EC(void) {
    volatile int *p = &D_800B0E08;
    if (*p != 0) {
        func_8006DF50(*p, 0x44C, 0x100, 0x80, 0x7F);
    }
}
