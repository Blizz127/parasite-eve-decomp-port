/* vendor-derived: khasinski/parasite-eve-decomp@3067919e7b82f0334b873117eef07e7157f4ed69 src/main/render/Render_GteScreenOffset.c — attribution khasinski (Chris Hasinski) */
/* inline GTE/asm: this function uses the vendor's inline PSY-Q GTE (COP2) asm macros — count separately from plain C. */
/* Imported under the owner's explicit authorization (2026-09-28, via Devbox Coordinator, vendor lane).
 * Upstream tree kept verbatim in vendor/khasinski-parasite-eve-decomp/ (see its PROVENANCE.md).
 * This file is that vendor TU run through the era cpp with the vendor CPP flags
 * (-undef -P, -I include, -I tools/m2c), `Render_ResetGteScreenOffset` renamed to `func_800661CC`, and vendor symbol
 * names mapped (post-cpp, token-wise) to this repo's address names from the vendor sym tables. */
/* Single-function extract: every other function definition of the vendor TU is reduced to a
 * prototype so this file compiles only `func_800661CC` (the TU spans several of this repo's yaml functions). */
extern unsigned short D_800BCF94;
extern unsigned short D_800BCF96;
int func_800661A4(void);
int func_800661CC(void) {
    register int x asm("v1") = 0xA0;
    register int y asm("a0") = 0x70;
    register int sx asm("t4");
    register int sy asm("t5");
    asm volatile("" : "=r"(x), "=r"(y) : "0"(x), "1"(y));
    sx = x << 16;
    sy = y << 16;
    asm volatile("ctc2 %0,$24" : : "r"(sx));
    asm volatile("ctc2 %0,$25" : : "r"(sy));
    return 0;
}
