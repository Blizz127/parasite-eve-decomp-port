/* Phase 5AW: forty-sixth matching C leaf (mid-330D4 carve).
 * VRAM 0x80042B28 / file 0x33328 / size 0x10.
 * Original: lui $v0,%hi(D_800A1838); lw $v0,%lo(D_800A1838)($v0);
 *           jr $ra; nop
 * 32-bit global getter; plain Phase 4J -O1 emits the exact four words.
 */
extern int D_800A1838;

int func_80042B28(void) {
    return D_800A1838;
}
