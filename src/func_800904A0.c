/* Phase 5O: twelfth matching C leaf (mid-804BC.s).
 * VRAM 0x800904A0 / file 0x80CA0 / size 0xC.
 * Original: addiu $v0,$zero,1; jr; sh $v0,0x84($a0)
 *   → *(unsigned short *)(arg0 + 0x84) = 1
 * Scratch probe: GCC 14.2 Phase 4J flags exact 0xC-byte match.
 */
void func_800904A0(void *arg0) {
    *(unsigned short *)((unsigned char *)arg0 + 0x84) = 1;
}
