/* func_800909C0 - VRAM 0x800909C0, file 0x811C0, size 0x4C.
 * Read a sign-extended 16-bit big-endian pair from the cursor, publish it at
 * +0x14 with the +0x6A halfword and the +0x38 bit-3 set. */
void func_800909C0(unsigned char *a0) {
    unsigned char *p = *(unsigned char **)a0;
    int h = *(short *)(a0 + 0x46);
    unsigned int v1;
    unsigned char *a2;
    *(unsigned int *)a0 = (unsigned int)(p + 1);
    v1 = *(unsigned char *)p;
    a2 = p + 2;
    *(unsigned int *)a0 = (unsigned int)a2;
    v1 |= (unsigned int)(*(unsigned char *)(p + 1)) << 8;
    *(unsigned short *)(a0 + 0x6A) = (unsigned short)h;
    *(unsigned int *)(a0 + 0x38) |= 8u;
    *(unsigned int *)(a0 + 0x14) = (unsigned int)(a2 + (short)v1);
}
