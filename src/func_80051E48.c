/* Phase 5AQ: fortieth matching C leaf (mid-41520 carve).
 * VRAM 0x80051E48 / file 0x42648 / size 0x10.
 * Original: lui $v0,%hi(D_800A1B30); addiu $v0,$v0,%lo(D_800A1B30); jr $ra; nop
 * Pure address-return leaf (returns &D_800A1B30).
 *
 * The ROM leaves the jr delay slot as nop (addiu scheduled *before* jr).
 * GCC 14.2 at -O1 otherwise fills the slot (lui; jr; addiu), so this single
 * unit is compiled with -fno-delayed-branch (per-file flag in build_us.sh;
 * the un-filled schedule was documented in the Phase 4J codegen probe).
 * Scratch probe with that flag emits the exact 4 words (0800E003 in word 3,
 * nop in word 4).
 */
extern int D_800A1B30;

int *func_80051E48(void) {
    return &D_800A1B30;
}
