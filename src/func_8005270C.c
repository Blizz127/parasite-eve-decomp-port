/* Phase 5FY: matching C leaf (42E34 cluster, the 0x58 member).
 * VRAM 0x8005270C / file 0x42F0C / size 0x58 (22 words).
 *
 * Plays sound 0x450 through func_8006DF50 when the sound package pointer at
 * D_800B0E08 is loaded, and publishes the call's result in D_8009D01C
 * (gp+0x2AC — the field-message fade target the port tracks as GM_D_8009D01C);
 * with no package it publishes 0.  func_80052764 stops that target.
 *
 * Two codegen devices are load-bearing here:
 *
 * 1. `volatile int *p` keeps both reads of the package pointer (the test read and
 *    the call argument); a plain global is commoned into one load.
 * 2. `D_800B0E08` is declared as a 3-word object rather than an int.  Only
 *    element 0 is ever touched, so the emitted loads are unchanged, but the
 *    declaration puts it above the -G8 small-data threshold.  That matters:
 *    under -G8 a small declaration makes cc1 address it gp-relative, and the
 *    "force absolute" knob only rewrites that into a *fused* `lui`/`lw`, whereas
 *    retail computes the address once into `$a0` with `lui`+`addiu` and reloads
 *    through it.  A non-small declaration makes cc1 emit exactly that.
 *
 * Net effect at -O2 -G8: LINK_EXACT, no maspsx knob required.
 */
extern int D_800B0E08[3];
extern int D_8009D01C;

int func_8006DF50(int a0, int a1, int a2, int a3, int a4);

int func_8005270C(void) {
    volatile int *p = D_800B0E08;
    int result;

    if (*p) {
        result = func_8006DF50(*p, 0x450, 0x100, 0x80, 0x7F);
    } else {
        result = 0;
    }
    D_8009D01C = result;
    return result;
}
