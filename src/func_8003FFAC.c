/* Phase 5EE: era $at absolute-sw pilot.
 * VRAM 0x8003FFAC / file 0x307AC / size 0x10.
 * The matching getter func_8003FFBC already declares this global as int.
 */
extern int D_800A1704;

void func_8003FFAC(int value) {
    D_800A1704 = value;
}
