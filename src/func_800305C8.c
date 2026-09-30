/*
 * func_800305C8 — angle-wrap helper.
 *
 * VRAM 0x800305C8 / file 0x20DC8 / size 0x78 (30 words). Non-leaf:
 * frame -0x18, $s0 at 0x10, and $ra at 0x14. The intermediate
 * 2048-minus-result is truncated to signed i16 before the record's
 * signed halfword at +0x3A is added; the final wrapped value is also
 * truncated to signed i16.
 *
 * era -O2 -G0 + maspsx 2.21.
 */
extern int func_80079FB4(int x, int z);

int func_800305C8(unsigned char *a0, unsigned char *a1) {
    register int v0 asm("$2");
    register int v1 asm("$3");
    int x = *(int *)(a0 + 0x28) - *(int *)(a1 + 0x28);
    int z = *(int *)(a0 + 0x30) - *(int *)(a1 + 0x30);
    int r = func_80079FB4(x, z);
    v1 = (int)(short)(2048 - r);
    v0 = *(short *)(a1 + 0x3A);
    v1 += v0;
    if (v1 < 0)
        v0 = v1 + 0xFFF;
    else
        v0 = v1;
    v0 = v0 >> 12;
    v0 = v0 << 12;
    v0 = v1 - v0;
    return (int)(short)v0;
}
