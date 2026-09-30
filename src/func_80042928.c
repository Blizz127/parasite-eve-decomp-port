/*
 * decomp-continue-4 — func_80042928, file 0x33128, size 0x3C.
 * func_8004298C(D_800A1860 - 1, 1) then clears D_800A1860 / D_800A1868.
 * Matching authority: scripts/build_us.sh EXACT SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 */
extern int func_8004298C(int, int);
extern int D_800A1860;
extern int D_800A1868;
void func_80042928(void) {
    func_8004298C(D_800A1860 - 1, 1);
    D_800A1860 = 0;
    D_800A1868 = 0;
}
