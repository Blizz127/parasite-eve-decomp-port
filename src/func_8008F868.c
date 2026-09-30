/* Phase 5L: ninth matching C leaf (mid-7FEA8.s).
 * VRAM 0x8008F868 / file 0x80068 / size 0x18.
 * Original: lhu 0x7C(a0); nop; addiu +1; andi 0xF; jr; sh 0x7C(a0)
 *   → *(unsigned short *)(arg0 + 0x7C) = (*field + 1) & 0xF
 * Scratch probe: GCC 14.2 Phase 4J flags exact 0x18-byte match.
 */
void func_8008F868(void *arg0) {
    unsigned short *ptr;
    unsigned int v0;

    ptr = (unsigned short *)((unsigned char *)arg0 + 0x7C);
    v0 = *ptr;
    v0 = (v0 + 1u) & 0xFu;
    *ptr = (unsigned short)v0;
}
