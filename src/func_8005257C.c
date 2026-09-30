/* Phase 5BT: sixty-ninth matching C leaf (mid-42D34 carve).
 * VRAM 0x8005257C / file 0x42D7C / size 0x18.
 * Original: lui/lw/nop/srl 1/jr/andi 1 on D_8009D1A0.
 * Fixed-shift bit test; plain -O1 matches (reloc resolved at link).
 */
extern unsigned int D_8009D1A0;

int func_8005257C(void) {
    return (D_8009D1A0 >> 1) & 1;
}
