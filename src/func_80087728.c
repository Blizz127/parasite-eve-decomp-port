/* Writes the low and high halfwords of a command word to the GPU port. */
void func_80087728(unsigned int a0) {
    volatile unsigned short *base = (volatile unsigned short *)0x1F800000;
    base[0xEC6] = (unsigned short)a0;
    base[0xEC7] = (unsigned short)(a0 >> 16);
}
