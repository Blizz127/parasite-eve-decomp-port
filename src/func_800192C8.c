/* READY-FROM-READER state setter: D_8009D28C = 8, return 1.
 * VRAM 0x800192C8 / file 0x9AC8 / size 0x14.
 * Era path: gcc-2.7.2-psx -O2 -G0 + maspsx 2.21 --dont-expand-li.
 *
 * D_8009D28C = int state value (see func_80017FDC).
 * ROM: addiu $v0,8; lui $at,%hi; sw $v0,%lo; jr; addiu $v0,1 (delay slot)
 */
extern int D_8009D28C;

int func_800192C8(void) {
    D_8009D28C = 8;
    return 1;
}
