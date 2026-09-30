/*
 * func_80017294 — field-VM handler: repoint the VM cursor to a per-entry table.
 *
 * VRAM 0x80017294 / file 0x7A94 / size 0x28 (10 words). Field-VM dispatch
 * target 0x802294 in the low byte (D_800910A0 opcode table).
 *
 * Retail (asm/disc1/7640.s):
 *   v = **a0;
 *   cursor = D_8009D2F0[0x27] + v * 2;   (0x9C state field)
 *   return 1;
 *
 * The cursor is gp-relative D_8009CE00 (`sw $3,0x90($gp)`); the state object
 * D_8009D2F0 is loaded absolutely. Under `-G8` cc1 gp-relocates both, so this
 * leaf needs `MASPSX_FORCE_ABSOLUTE_SYMBOLS=D_8009D2F0` (extern stripped) to
 * keep the base absolute while the store stays gp-relative.
 * era -O2 -G8; LINK_EXACT.
 * ROM: asm/disc1/7640.s @ file 0x7A94, 10 words (0x28 bytes).
 */

extern unsigned int *D_8009D2F0;
extern int D_8009CE00;

int func_80017294(unsigned int **a0) {
    unsigned int v = **a0;

    D_8009CE00 = D_8009D2F0[0x27] + v * 2;
    return 1;
}
