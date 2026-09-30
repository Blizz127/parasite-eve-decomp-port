/* Phase 5AP: thirty-ninth matching C leaf (mid-2E7D0 carve).
 * VRAM 0x800428C4 / file 0x330C4 / size 0x10.
 * Original: lui v0,%hi(D_800A1860); lw v0,%lo(D_800A1860)(v0); jr ra; addiu v0,v0,-1
 * Simple global decrement getter.
 * Scratch probe GCC 14.2 Phase 4J flags emits exact words.
 */
extern int D_800A1860;

int func_800428C4(void) {
    return D_800A1860 - 1;
}
