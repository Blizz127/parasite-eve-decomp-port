/* Phase 5AN: thirty-sixth matching C leaf (mid-9860 carve).
 * VRAM 0x800190AC / file 0x98AC / size 0x8.
 * Original: jr $ra; addiu $v0,$zero,1 — return-1 stub (li in delay slot).
 * Fifth of the seven byte-identical return-1 twins (2026-07-09 triage);
 * back-to-back with func_800190B4 (file 0x98B4).
 * Scratch probe (Phase 5AJ, same bytes): GCC 14.2 Phase 4J flags exact
 * 0x8-byte match (0800E003 01000224).
 */
int func_800190AC(void) {
    return 1;
}
