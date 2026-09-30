/* Phase 5CA: seventy-sixth matching C leaf.
 * VRAM 0x80064C20 / file 0x55420 / size 0x10.
 * Store -1 to *(a0+0x70) and *(a0+0x44); plain -O1 matches.
 */
void func_80064C20(unsigned char *arg0) {
    unsigned int v0 = 0xFFFFFFFFu;
    *(unsigned int *)(arg0 + 0x70) = v0;
    *(unsigned int *)(arg0 + 0x44) = v0;
}
