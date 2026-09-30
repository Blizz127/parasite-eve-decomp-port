/*
 * func_80067730 — VRAM 0x80067730, size 0x70, file 0x57F30-0x57FA0.
 *
 * Sets/clears bit 3 (0x08) of the record byte, then writes (0x10000-arg2)>>8
 * and (0x10000-arg3)>>8 to the halfwords at +0x1C/+0x1E. Returns 0.
 *
 * era -O2 -G0; sibling of func_80067678/CC.
 */
typedef struct {
    char pad[0x14];
    int field_14;
} Header;

extern Header *volatile D_800B1624;

int func_80067730(int arg0, int arg1, unsigned int arg2, unsigned int arg3) {
    unsigned char *p = (unsigned char *)D_800B1624 + D_800B1624->field_14 + arg0 * 56;
    if (arg1 != 0) {
        *p |= 8;
    } else {
        *p &= ~8;
    }
    *(unsigned short *)(p + 0x1C) = (0x10000 - arg2) >> 8;
    *(unsigned short *)(p + 0x1E) = (0x10000 - arg3) >> 8;
    return 0;
}
