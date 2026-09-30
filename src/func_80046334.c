/*
 * func_80046334 — deferred teardown poll (retail 0x80046334).
 *
 * VRAM 0x80046334 / file 0x36B34 / size 0x44 (17 words). Tail of the 35698.s
 * run; func_80046ABC follows at 0x372BC.
 *
 * Loads the gp-relative word at D_8009CFA0 + 0x10 (0x240($gp)); when set,
 * reads the absolute byte D_800B0CE6 and, if its low two bits are clear,
 * clears the pending flag via func_8005E114(0) and stores zero back.
 *
 * Build: era -O2 -G8. D_8009CFA0 is declared as a scalar so the +0x10 access
 * stays gp-relative. D_800B0CE6 is declared as an incomplete array: at
 * 0x800B0CE6 it lies outside the $gp window, so it must stay absolute — the
 * incomplete element type emits no `.extern D_800B0CE6, 1`, which otherwise
 * makes maspsx turn the byte load into a (unlinkable) gp-relative one.
 */
extern int D_8009CFA0;
extern unsigned char D_800B0CE6[];
extern int func_8005E114(int);

int func_80046334(void) {
    int var_v0;

    var_v0 = *(int *)((char *)&D_8009CFA0 + 16);
    if (var_v0 != 0) {
        var_v0 = D_800B0CE6[0] & 3;
        if (var_v0 == 0) {
            var_v0 = func_8005E114(0);
            *(int *)((char *)&D_8009CFA0 + 16) = 0;
        }
    }
    return var_v0;
}
