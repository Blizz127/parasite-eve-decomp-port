/* Phase 5C: second matching C leaf (sibling of func_80090C38).
 * VRAM 0x80090C4C / file 0x8144C / size 0x14.
 * Original: lw; addiu $v1,$zero,-0x11; and; jr; sw
 *   → clear bit 0x10 at *(arg0+0x38)
 * Same GCC 14.2 flags as Phase 4J/5B. No semantic struct/field names yet.
 */
void func_80090C4C(void *arg0) {
    *(unsigned int *)((unsigned char *)arg0 + 0x38) &= ~0x10u;
}
