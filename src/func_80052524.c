/* Phase 5AS: forty-second matching C leaf (zero-prefix carve of 42D24.s).
 * VRAM 0x80052524 / file 0x42D24 / size 0x10.
 * Original: lui $v0,%hi(D_800C0E32); lhu $v0,%lo(D_800C0E32)($v0); jr $ra; nop
 * 16-bit unsigned global getter (returns D_800C0E32).
 * Twin of func_80052514 (D_800C0E28) — identical shape; plain Phase 4J -O1
 * matches (MIPS-I load-delay keeps lhu out of the jr delay slot).
 */
extern unsigned short D_800C0E32;

unsigned short func_80052524(void) {
    return D_800C0E32;
}
