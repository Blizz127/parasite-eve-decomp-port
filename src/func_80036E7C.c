/* func_80036E7C — VRAM 0x80036E7C, size 0x100 (64 words), file 0x2767C.
 * Straight-line; no calls.  Packs three "(a0[1] % 60)" fields into a1[0]:
 *   bits 0..19   = (x%60) * 216000 & 0xFFFFF
 *   bits 20..25  = (x%60) * 3600 & 0x3F
 *   bits 28..31  = (x%60) * 15
 * The unsigned `%` is what makes cc1 emit retail's `multu 0x88888889` /
 * `mfhi` / `srl 5` and keep the division constant live across all three
 * fields; a signed operand adds the sign-correction sequence (12 extra words).
 * Each statement is a read-modify-write of a1[0], and cc1 keeps the running
 * value in a register (retail loads a1[0] once). */
void func_80036E7C(unsigned int *a0, unsigned int *a1)
{
    a1[0] = (a1[0] & 0xFFF00000u) | ((a0[1] % 60u) * 216000u & 0xFFFFFu);
    a1[0] = (a1[0] & 0xFC0FFFFFu) | (((a0[1] % 60u) * 3600u & 0x3Fu) << 20);
    a1[0] = (a1[0] & 0x03FFFFFFu) | ((a0[1] % 60u) * 15u << 28);
}
