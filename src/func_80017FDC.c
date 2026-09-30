/* READY-FROM-READER state setter: D_8009D28C = 5, return 1.
 * VRAM 0x80017FDC / file 0x87DC / size 0x14.
 * Era path: gcc-2.7.2-psx -O2 -G0 + maspsx 2.21 --dont-expand-li.
 *
 * D_8009D28C type = int (state value). Provenance: func_80019154 word-copies
 * via lw→sw (no narrowing); A404 equality-tests via bne vs 1 (no slt/sltu).
 * Distinct state codes 0/3/4/5/6/8 — NOT opaque-word (A182x were bare flags).
 * Signedness undetermined → int (sign-neutral).
 *
 * ROM: addiu $v0,5; lui $at,%hi; sw $v0,%lo; jr; addiu $v0,1 (delay slot)
 */
extern int D_8009D28C;

int func_80017FDC(void) {
    D_8009D28C = 5;
    return 1;
}
