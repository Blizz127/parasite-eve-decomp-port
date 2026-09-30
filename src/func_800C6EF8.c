/* Phase 5GB: matching C leaf.
 * VRAM 0x800C6EF8 / file 0xB76F8 / size 0x54 (21 words).
 *
 * Copies a word array out of a record: the destination is the fixed buffer
 * D_800E2370, the source is `record + *(u16 *)(record + 8)` (a byte offset held
 * in the record's +8 halfword), and the count is `*(u16 *)(record + 0xA)` — read
 * again on every iteration, which is why retail carries the `lhu 0xA(a0)` inside
 * the loop rather than in a register.
 *
 * The counter increment is written *inside* the body, before the copy.  As a
 * `for (i = 0; i < count; i++)` header the increment is scheduled after the load
 * and three words differ; the explicit `i++;` first makes GCC order the body
 * exactly as retail does.
 */
extern int D_800E2370[];

void func_800C6EF8(unsigned char *record) {
    int i = 0;
    int *dst = D_800E2370;
    int *src = (int *)(record + *(unsigned short *)(record + 8));

    while (i < *(unsigned short *)(record + 0xA)) {
        i++;
        *dst++ = *src++;
    }
}
