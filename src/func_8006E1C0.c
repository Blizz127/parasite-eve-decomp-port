/*
 * func_8006E1C0 — packed colour/vertex emit (retail 0x8006E1C0).
 *
 * VRAM 0x8006E1C0 / file 0x5E9C0 / size 0x110 (68 words). Carved out of the
 * [0x5E7A8, asm] run.  era -O2 -G0.  The packed word at +0x0C overlaps the
 * byte at +0x0F, so the record is addressed by explicit offsets.
 */
extern void func_8007506C(short *, unsigned int);

int func_8006E1C0(char *a0, unsigned int a1) {
    short buf[4];
    unsigned char v0;
    short v1;

    buf[0] = (*(unsigned int *)(a0 + 8) >> 10) & 0x7FF;
    buf[1] = *(unsigned int *)(a0 + 8) >> 21;
    buf[2] = *(unsigned int *)(a0 + 8) & 0x3FF;
    v0 = *(unsigned char *)(a0 + 7);
    v1 = 0x100;
    if (v0 != 0) {
        v1 = v0 & 0xFF;
    }
    buf[3] = v1;
    func_8007506C(buf, a1 + (*(unsigned int *)(a0 + 4) & 0xFFFFFF));
    if (*(unsigned int *)(a0 + 0xC) & 0xFFFFFF) {
        buf[0] = (*(unsigned int *)(a0 + 0x10) >> 10) & 0x7FF;
        buf[1] = *(unsigned int *)(a0 + 0x10) >> 21;
        buf[2] = *(unsigned int *)(a0 + 0x10) & 0x3FF;
        buf[3] = *(unsigned char *)(a0 + 0xF);
        func_8007506C(buf, a1 + (*(unsigned int *)(a0 + 4) & 0xFFFFFF)
                               + (*(unsigned int *)(a0 + 0xC) & 0xFFFFFF));
    }
    return 0;
}
