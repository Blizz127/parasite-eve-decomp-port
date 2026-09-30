/*
 * func_800176B8 — field-VM handler: OR an operand word into the state flags.
 *
 * VRAM 0x800176B8 / file 0x7EB8 / size 0x28 (10 words). Field-VM dispatch
 * target 0x8026B8 in the low byte (D_800910A0 opcode table).
 *
 * Retail (asm/disc1/7C10.s):
 *   D_8009D2F0[0x26] |= **a0;   (0x98 state flags)
 *   return 1;
 *
 * Same absolute-base split as func_80017294:
 * `MASPSX_FORCE_ABSOLUTE_SYMBOLS=D_8009D2F0`. era -O2 -G8; LINK_EXACT.
 * ROM: asm/disc1/7C10.s @ file 0x7EB8, 10 words (0x28 bytes).
 */

extern unsigned int *D_8009D2F0;

int func_800176B8(unsigned int **a0) {
    D_8009D2F0[0x26] |= **a0;
    return 1;
}
