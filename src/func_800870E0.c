/* Phase 5BD: fifty-third matching C leaf (mid-72ABC carve).
 * VRAM 0x800870E0 / file 0x778E0 / size 0x10.
 * Original: lui $v0,%hi(D_8009D24C); lw $v0,%lo(D_8009D24C)($v0);
 *           jr $ra; nop
 * 32-bit global getter; plain Phase 4J -O1 emits the exact four words.
 */
extern int D_8009D24C;

int func_800870E0(void) {
    return D_8009D24C;
}
