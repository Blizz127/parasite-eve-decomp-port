/* Phase 5E: fourth matching C leaf.
 * VRAM 0x80090C60 / file 0x81460 / size 0x14.
 * Original: lw; nop; ori $v0,$v0,0x20; jr; sw
 *   → set bit 0x20 at *(arg0+0x38)
 * Same GCC 14.2 flags as Phase 4J/5B–5D. No semantic struct/field names yet.
 */
void func_80090C60(void *arg0) {
    *(unsigned int *)((unsigned char *)arg0 + 0x38) |= 0x20u;
}
