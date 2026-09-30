/*
 * func_8006DCE4 — forwarding shim: func_8006DED4(D_800B0E64, a0, a1, a2, a3, a4)
 * with the three trailing parameters declared `short` (retail sign-extends
 * each with sll/sra, the fifth from a full-word stack load).
 *
 * ROM: era gcc-2.7.2-psx -O2 -G0 -fno-expensive-optimizations (load-bearing:
 * without it cc1 copies both a0 and a1 through temporaries, 20 words differ).
 */
extern int D_800B0E64;
extern int func_8006DED4();

int func_8006DCE4(int a0, int a1, short a2, short a3, short a4) {
    return func_8006DED4(D_800B0E64, a0, a1, a2, a3, a4);
}
