/* Phase 5FW: matching C leaf (42E34 cluster, part 3 of 3).
 * VRAM 0x800526C4 / file 0x42EC4 / size 0x48.
 *
 * Twin of func_800525EC with sound id 0x44F.  See func_80052634.c for why the
 * address is taken once into a volatile pointer.
 */
extern volatile int D_800B0E08;

void func_8006DF50(int a0, int a1, int a2, int a3, int a4);

void func_800526C4(void) {
    volatile int *p = &D_800B0E08;
    if (*p != 0) {
        func_8006DF50(*p, 0x44F, 0x100, 0x80, 0x7F);
    }
}
