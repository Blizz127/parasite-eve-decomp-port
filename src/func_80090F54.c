/* Phase 5D: third matching C leaf.
 * VRAM 0x80090F54 / file 0x81754 / size 0x14.
 * Original: lw; lui $v1,0x10; or; jr; sw
 *   → set bit 0x100000 at *(arg0+0x38)
 * Same GCC 14.2 flags as Phase 4J/5B/5C. No semantic struct/field names yet.
 */
void func_80090F54(void *arg0) {
    *(unsigned int *)((unsigned char *)arg0 + 0x38) |= 0x100000u;
}
