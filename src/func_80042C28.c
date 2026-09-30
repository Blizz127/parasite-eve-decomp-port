/* Opaque-word pilot (Stage 2): set D_800A182C = 1, return 0.
 * VRAM 0x80042C28 / file 0x33428 / size 0x14.
 * Era path: gcc-2.7.2-psx -O2 -G0 + maspsx 2.21 --dont-expand-li.
 *
 * Type provenance (lead opaque-word ruling): every writer/reader of
 * D_800A182C is bare 32-bit sw/lw (test-and-clear flags in func_800405A4).
 * No arith/pointer/bitwise use anywhere in live asm or src. Typed unsigned
 * int (u32); refine if a narrowing reader is later found.
 *
 * ROM shape: addiu $v0,1; lui $at,%hi; sw $v0,%lo; jr; addu $v0,$zero,$zero
 */
extern unsigned int D_800A182C;

int func_80042C28(void) {
    D_800A182C = 1;
    return 0;
}
