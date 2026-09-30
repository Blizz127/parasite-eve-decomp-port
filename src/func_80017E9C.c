/* Phase 5AK: thirty-third matching C leaf (mid-2A0C carve).
 * VRAM 0x80017E9C / file 0x869C / size 0x8.
 * Original: jr $ra; addiu $v0,$zero,1 — return-1 stub (li in delay slot).
 * Second of the seven byte-identical return-1 twins (2026-07-09 triage);
 * rodata dispatch-table target (.word func_80017E9C at file 0x819D0).
 * Scratch probe (Phase 5AJ, same bytes): GCC 14.2 Phase 4J flags exact
 * 0x8-byte match (0800E003 01000224).
 */
int func_80017E9C(void) {
    return 1;
}
