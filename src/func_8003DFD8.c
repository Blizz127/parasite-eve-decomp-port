/* Copy `count` 32-byte records: halfwords +0..+0x10 and words +0x14/+0x18/+0x1C.
 * The manual i++ sits before the three word copies so it fills the -0xC
 * halfword's load delay; the for-header increment schedules src+=32 there
 * instead. -O2 -G0. Symbol size 0xCC (section is 16-byte aligned).
 */
void func_8003DFD8(unsigned char *src, unsigned char *dst, short count) {
    int i;

    for (i = 0; i < count;) {
        *(unsigned short *)(dst + 0x00) = *(unsigned short *)(src + 0x00);
        *(unsigned short *)(dst + 0x02) = *(unsigned short *)(src + 0x02);
        *(unsigned short *)(dst + 0x04) = *(unsigned short *)(src + 0x04);
        *(unsigned short *)(dst + 0x06) = *(unsigned short *)(src + 0x06);
        *(unsigned short *)(dst + 0x08) = *(unsigned short *)(src + 0x08);
        *(unsigned short *)(dst + 0x0A) = *(unsigned short *)(src + 0x0A);
        *(unsigned short *)(dst + 0x0C) = *(unsigned short *)(src + 0x0C);
        *(unsigned short *)(dst + 0x0E) = *(unsigned short *)(src + 0x0E);
        *(unsigned short *)(dst + 0x10) = *(unsigned short *)(src + 0x10);
        i++;
        *(int *)(dst + 0x14) = *(int *)(src + 0x14);
        *(int *)(dst + 0x18) = *(int *)(src + 0x18);
        *(int *)(dst + 0x1C) = *(int *)(src + 0x1C);
        src += 0x20;
        dst += 0x20;
    }
}
