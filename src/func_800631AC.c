/* Phase 5BZ: seventy-fifth matching C leaf.
 * VRAM 0x800631AC / file 0x539AC / size 0x14.
 * Null-checked store 1 to *(a0+0x48); plain -O1 matches.
 */
void func_800631AC(unsigned char *arg0) {
    if (arg0 != 0) {
        *(unsigned int *)(arg0 + 0x48) = 1;
    }
}
