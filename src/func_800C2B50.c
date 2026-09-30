/* Phase 5BW: seventy-second matching C leaf (zero-prefix carve of B3350.s).
 * VRAM 0x800C2B50 / file 0xB3350 / size 0x18.
 * Original: lui/lw/nop/lw 0x70/jr/nop on D_800E2248.
 * Pointer field getter at +0x70; plain -O1 matches.
 */
extern int *D_800E2248;

int func_800C2B50(void) {
    return D_800E2248[0x1C];
}
