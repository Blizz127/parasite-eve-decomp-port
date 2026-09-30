/* Phase 5FW: matching C leaf (42E34 cluster, part 2 of 3).
 * VRAM 0x8005267C / file 0x42E7C / size 0x48.
 *
 * Twin of func_800525EC with sound id 0x44E.  See func_80052634.c for why the
 * address is taken once into a volatile pointer.
 */
extern volatile int D_800B0E08;

void func_8006DF50(int a0, int a1, int a2, int a3, int a4);

void func_8005267C(void) {
    volatile int *p = &D_800B0E08;
    if (*p != 0) {
        func_8006DF50(*p, 0x44E, 0x100, 0x80, 0x7F);
    }
}
