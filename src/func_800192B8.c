/* READY-FROM-READER state setter: D_8009D28C = 0, return 1.
 * VRAM 0x800192B8 / file 0x9AB8 / size 0x10.
 * Era path: gcc-2.7.2-psx -O2 -G0 + maspsx 2.21 --dont-expand-li.
 *
 * D_8009D28C = int state value (see func_80017FDC).
 * ROM: lui $at,%hi; sw $zero,%lo; jr; addiu $v0,1 (delay slot) — no addiu for 0.
 */
extern int D_8009D28C;

int func_800192B8(void) {
    D_8009D28C = 0;
    return 1;
}
