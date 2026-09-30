/* Phase 5AT: forty-third matching C leaf (mid-2E7D0 carve).
 * VRAM 0x8003FFBC / file 0x307BC / size 0x10.
 * Original: lui $v0,%hi(D_800A1704); lw $v0,%lo(D_800A1704)($v0); jr $ra; nop
 * 32-bit global getter (returns D_800A1704).
 * Same load-return schedule as the 16-bit twins (5AR/5AS): plain Phase 4J
 * -O1 matches (MIPS-I load-delay keeps lw out of the jr delay slot).
 * Setter twin func_8003FFAC joined this getter in Phase 5EE via the era path.
 */
extern int D_800A1704;

int func_8003FFBC(void) {
    return D_800A1704;
}
