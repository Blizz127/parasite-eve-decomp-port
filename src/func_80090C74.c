/* Phase 5F: fifth matching C leaf.
 * VRAM 0x80090C74 / file 0x81474 / size 0x14.
 * Original: lw; addiu $v1,$zero,-0x21; and; jr; sw
 *   → clear bit 0x20 at *(arg0+0x38)
 * Same GCC 14.2 flags as Phase 4J/5B–5E. No semantic struct/field names yet.
 */
void func_80090C74(void *arg0) {
    *(unsigned int *)((unsigned char *)arg0 + 0x38) &= ~0x20u;
}
