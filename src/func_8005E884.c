/* Phase 5BR: sixty-seventh matching C leaf (mid-4C4B0 carve).
 * VRAM 0x8005E884 / file 0x4F084 / size 0x10.
 * Original: lui $v0,%hi(D_800B0DB1); lb $v0,%lo(D_800B0DB1)($v0);
 *           jr $ra; nop
 */
extern signed char D_800B0DB1;

signed char func_8005E884(void) {
    return D_800B0DB1;
}
