/* Phase 5BS: sixty-eighth matching C leaf (mid-4C4B0 carve).
 * VRAM 0x8006EBD4 / file 0x5F3D4 / size 0x10.
 * Original: lui $v0,%hi(D_800B0DBA); lb $v0,%lo(D_800B0DBA)($v0);
 *           jr $ra; nop
 */
extern signed char D_800B0DBA;

signed char func_8006EBD4(void) {
    return D_800B0DBA;
}
