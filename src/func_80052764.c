/* Phase 5FW: matching C leaf (42E34 cluster, part 5).
 * VRAM 0x80052764 / file 0x42F64 / size 0x2C.
 *
 * Companion of func_8005270C: if a fade target was published in D_8009D01C,
 * stop it (func_800866A4(target, 0)) and clear the target.  D_8009D01C is in the
 * small-data window, so both accesses are gp-relative (0x2AC($gp)) — -G8 profile.
 */
extern int D_8009D01C;

int func_800866A4(int a0, int a1);

void func_80052764(void) {
    int target = D_8009D01C;

    if (target != 0) {
        func_800866A4(target, 0);
        D_8009D01C = 0;
    }
}
