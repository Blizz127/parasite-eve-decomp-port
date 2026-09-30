/* Phase 5BY: seventy-fourth matching C leaf.
 * VRAM 0x80063198 / file 0x53998 / size 0x14.
 * Null-checked clear of *(a0+0x48); plain -O1 matches.
 */
void func_80063198(unsigned char *arg0) {
    if (arg0 != 0) {
        *(unsigned int *)(arg0 + 0x48) = 0;
    }
}
