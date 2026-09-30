/* vendor-derived: khasinski/parasite-eve-decomp@3067919e7b82f0334b873117eef07e7157f4ed69 src/main/math/math_fixed.c — attribution khasinski (Chris Hasinski) */
/* inline GTE/asm: this function uses vendor inline asm (hand-written instructions) — count separately from plain C. */
/* Imported under the owner's explicit authorization (2026-09-28, via Devbox Coordinator, vendor lane).
 * Upstream tree kept verbatim in vendor/khasinski-parasite-eve-decomp/ (see its PROVENANCE.md).
 * This file is that vendor TU run through the era cpp with the vendor CPP flags
 * (-undef -P, -I include, -I tools/m2c), `Math_FixedRoundToByte` renamed to `func_800370CC`, and vendor symbol
 * names mapped (post-cpp, token-wise) to this repo's address names from the vendor sym tables. */
/* Single-function extract: every other function definition of the vendor TU is reduced to a
 * prototype so this file compiles only `func_800370CC` (the TU spans several of this repo's yaml functions). */
void func_80077C84(void *p, int dfe, int dtd, int tpage);
void func_80077C04(unsigned char *arg0);
int func_80077CB4(void *arg0, void *arg1);
void func_800719E4(int code);
int func_800370A8(int a, int b);
int Math_FixedRoundToInt(int arg0);
int func_800370CC(int arg0) {
    register int bias asm("$1");
    int value;
    bias = 0x8000;
    asm volatile("add\t%0,%1,%2" : "=r"(value) : "r"(arg0), "r"(bias));
    return value >> 8;
}
void func_800370DC(void *packet, int tpage);
