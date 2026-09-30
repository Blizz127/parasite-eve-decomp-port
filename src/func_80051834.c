/* Phase 5AV: forty-fifth matching C leaf (mid-41520 carve).
 * VRAM 0x80051834 / file 0x42034 / size 0x18.
 * Original: lui/lw/nop/srlv/jr/andi 1 on D_800C0E24.
 * Unsigned global bit test; plain Phase 4J -O1 emits the exact six words.
 */
extern unsigned int D_800C0E24;

int func_80051834(int a0) {
    return (D_800C0E24 >> a0) & 1;
}
