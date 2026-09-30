void func_80087760(unsigned int a0) {
    volatile unsigned short *base = (volatile unsigned short *)0x1F800000;
    base[0xECA] = (unsigned short)a0;
    base[0xECB] = (unsigned short)(a0 >> 16);
}
