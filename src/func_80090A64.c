/* func_80090A64 - VRAM 0x80090A64, file 0x81264, size 0x48.
 * Read a cursor byte into the *D_8009D2C8 +0x64 halfword, then OR the next
 * cursor byte shifted left 8 into it; the final store fills the jr delay slot. */
extern unsigned int D_8009D2C8;

void func_80090A64(unsigned char *a0) {
    unsigned char *c = *(unsigned char **)a0;
    unsigned char *r = (unsigned char *)D_8009D2C8;
    *(unsigned int *)a0 = (unsigned int)(c + 1);
    *(unsigned short *)(r + 0x64) = *c;
    c = *(unsigned char **)a0;
    *(unsigned int *)a0 = (unsigned int)(c + 1);
    *(unsigned short *)(r + 0x64) = (unsigned short)(*(unsigned short *)(r + 0x64) | ((unsigned int)*c << 8));
}
