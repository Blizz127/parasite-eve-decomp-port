/* READY-FROM-BITWISE flags setter: D_8009D270 = 1, return 0.
 * VRAM 0x80087198 / file 0x77998 / size 0x14.
 * Era path: gcc-2.7.2-psx -O2 -G0 + maspsx 2.21 --dont-expand-li.
 *
 * D_8009D270 = unsigned int flags. Provenance: func_800871AC andi 0x1 /
 * and -0x2 (test/clear bit 0); func_80087428 andi 0x2 / and -0x3 (bit 1).
 * READY-FROM-BITWISE — not opaque-word, not int-state.
 *
 * ROM: addiu $v0,1; lui $at,%hi; sw $v0,%lo; jr; addu $v0,$zero,$zero
 * (sb+ret0 fingerprint; whole-word store of flag bit 0.)
 */
extern unsigned int D_8009D270;

int func_80087198(void) {
    D_8009D270 = 1;
    return 0;
}
