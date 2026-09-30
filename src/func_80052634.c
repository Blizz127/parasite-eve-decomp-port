/* Phase 5FW: matching C leaf (42E34 cluster, part 1 of 3).
 * VRAM 0x80052634 / file 0x42E34 / size 0x48.
 *
 * Twin of func_800525EC with sound id 0x44D: when the sound package pointer
 * D_800B0E08 is non-zero it calls func_8006DF50(package, 0x44D, 0x100, 0x80, 0x7F).
 * Same codegen shape as func_800525EC — retail materialises &D_800B0E08 once into
 * $a0 and reads through it twice, so the address is taken once into a pointer and
 * the pointee is volatile to keep both reads.
 */
extern volatile int D_800B0E08;

void func_8006DF50(int a0, int a1, int a2, int a3, int a4);

void func_80052634(void) {
    volatile int *p = &D_800B0E08;
    if (*p != 0) {
        func_8006DF50(*p, 0x44D, 0x100, 0x80, 0x7F);
    }
}
