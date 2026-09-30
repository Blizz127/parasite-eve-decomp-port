/* Phase 5J: seventh matching C leaf (90Cxx family, early sibling).
 * VRAM 0x80090A0C / file 0x8120C / size 0x14.
 * Original: lw; addiu $v1,$zero,-0x9; and; jr; sw
 *   → clear bit 0x8 at *(arg0+0x38)
 * Scratch probe: GCC 14.2 Phase 4J flags exact 0x14-byte match.
 * Same pattern as func_80090C4C / func_80090C74. No semantic names yet.
 */
void func_80090A0C(void *arg0) {
    *(unsigned int *)((unsigned char *)arg0 + 0x38) &= ~0x8u;
}
