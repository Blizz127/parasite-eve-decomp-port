/* Phase 5N: eleventh matching C leaf (mid-80098.s).
 * VRAM 0x8008FCB4 / file 0x804B4 / size 0x8.
 * Original: jr; sh $zero, 0x82($a0)
 *   → *(unsigned short *)(arg0 + 0x82) = 0
 * Scratch probe: GCC 14.2 Phase 4J flags exact 0x8-byte match.
 */
void func_8008FCB4(void *arg0) {
    *(unsigned short *)((unsigned char *)arg0 + 0x82) = 0;
}
