void func_80087744(unsigned int a0) {
    volatile unsigned short *base = (volatile unsigned short *)0x1F800000;
    base[0xECC] = (unsigned short)a0;
    base[0xECD] = (unsigned short)(a0 >> 16);
}
