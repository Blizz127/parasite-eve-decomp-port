/* func_80090A20 - VRAM 0x80090A20, file 0x81220, size 0x44.
 * Publish two cursor bytes through *D_8009D2C8 at +0x60/+0x5C and clear
 * +0x62/+0x5E; the final store fills the jr delay slot. */
extern unsigned int D_8009D2C8;

void func_80090A20(unsigned char *a0) {
    unsigned char *c = *(unsigned char **)a0;
    unsigned char *r = (unsigned char *)D_8009D2C8;
    unsigned int b;
    *(unsigned int *)a0 = (unsigned int)(c + 1);
    *(unsigned short *)(r + 0x60) = *c;
    c = *(unsigned char **)a0;
    *(unsigned int *)a0 = (unsigned int)(c + 1);
    b = *c;
    *(unsigned short *)(r + 0x62) = 0;
    *(unsigned short *)(r + 0x5E) = 0;
    *(unsigned short *)(r + 0x5C) = (unsigned short)b;
}
