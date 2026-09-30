/* Phase 5K: eighth matching C leaf (mid-2A0C prefix).
 * VRAM 0x8008F694 / file 0x7FE94 / size 0x14.
 * Original: lw; nop; addiu +2; jr; sw (delay slot)
 *   → *(unsigned int *)arg0 += 2
 * Scratch probe: GCC 14.2 Phase 4J flags exact 0x14-byte match.
 */
void func_8008F694(void *arg0) {
    unsigned int v0;

    v0 = *(unsigned int *)arg0;
    v0 = v0 + 2u;
    *(unsigned int *)arg0 = v0;
}
