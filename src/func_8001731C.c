/*
 * func_8001731C — field-VM handler: conditional cursor repoint (argument-only
 * variant).
 *
 * VRAM 0x8001731C / file 0x7B1C / size 0x40 (16 words). Field-VM dispatch
 * target 0x80231C in the low byte (D_800910A0 opcode table).
 *
 * Retail (asm/disc1/7B1C.s):
 *   if (**a0 == 0) {
 *       v = *a0[1];
 *       cursor = D_8009D2F0[0x27] + v * 2;   (0x9C state field, gp-relative)
 *   }
 *   return 1;
 *
 * Same absolute-base + gp-relative-cursor split as func_80017294:
 * `MASPSX_FORCE_ABSOLUTE_SYMBOLS=D_8009D2F0`. era -O2 -G8; LINK_EXACT.
 * ROM: asm/disc1/7B1C.s @ file 0x7B1C, 16 words (0x40 bytes).
 */

extern unsigned int *D_8009D2F0;
extern int D_8009CE00;

int func_8001731C(unsigned int **a0) {
    if (**a0 == 0) {
        unsigned int v = *a0[1];

        D_8009CE00 = D_8009D2F0[0x27] + v * 2;
    }
    return 1;
}
