/* Phase 5AR: forty-first matching C leaf (mid-42658 carve).
 * VRAM 0x80052514 / file 0x42D14 / size 0x10.
 * Original: lui $v0,%hi(D_800C0E28); lhu $v0,%lo(D_800C0E28)($v0); jr $ra; nop
 * 16-bit unsigned global getter (returns D_800C0E28).
 *
 * The jr delay slot is nop (lhu before jr). On MIPS-I a load has a delay
 * slot, so GCC 14.2 -O1 never schedules the lhu into the return's delay
 * slot — plain Phase 4J flags already emit the exact 4 words (no
 * -fno-delayed-branch needed). Scratch probe confirmed.
 * Twin: func_80052524 (D_800C0E32) — Phase 5AS.
 */
extern unsigned short D_800C0E28;

unsigned short func_80052514(void) {
    return D_800C0E28;
}
