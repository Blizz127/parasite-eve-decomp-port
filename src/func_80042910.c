/* READY-FROM-READER dual clear (option a dual-type leaf).
 * VRAM 0x80042910 / file 0x33110 / size 0x18.
 * Era path: gcc-2.7.2-psx -O2 -G0 + maspsx 2.21 --dont-expand-li.
 *
 * D_800A1860: int from func_800428C4 (addiu -1 arithmetic).
 * D_800A1868: unsigned int opaque-word (Stage 0 post-5EH; write-only sw 0/1).
 * Declared here so this dual-store integrates cleanly; D_800A1868's other
 * writers remain asm until the opaque phase. Decl only, not a carve for 1868.
 *
 * ROM: sw $zero,D_800A1860; sw $zero,D_800A1868; jr; nop
 */
extern int D_800A1860;
extern unsigned int D_800A1868;

void func_80042910(void) {
    D_800A1860 = 0;
    D_800A1868 = 0;
}
