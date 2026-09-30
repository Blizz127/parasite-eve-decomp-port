void func_8008777C(unsigned int a0) {
    volatile unsigned short *base = (volatile unsigned short *)0x1F800000;
    base[0xEC8] = (unsigned short)a0;
    base[0xEC9] = (unsigned short)(a0 >> 16);
}
