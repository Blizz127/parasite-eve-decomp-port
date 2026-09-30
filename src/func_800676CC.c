/*
 * func_800676CC — VRAM 0x800676CC, size 0x64, file 0x57ECC-0x57F30.
 *
 * Sets/clears bit 2 (0x04) of the record byte, then writes arg2>>8 and arg3>>8
 * to the halfwords at +0x1C/+0x1E. Returns 0.
 *
 * era -O2 -G0; sibling of func_80067678.
 */
typedef struct {
    char pad[0x14];
    int field_14;
} Header;

extern Header *volatile D_800B1624;

int func_800676CC(int arg0, int arg1, unsigned int arg2, unsigned int arg3) {
    unsigned char *p = (unsigned char *)D_800B1624 + D_800B1624->field_14 + arg0 * 56;
    if (arg1 != 0) {
        *p |= 4;
    } else {
        *p &= ~4;
    }
    *(unsigned short *)(p + 0x1C) = arg2 >> 8;
    *(unsigned short *)(p + 0x1E) = arg3 >> 8;
    return 0;
}
