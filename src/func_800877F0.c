void func_800877F0(int a0, unsigned int a1) {
    *(volatile unsigned short *)(0x1F801C0E + (a0 << 4)) =
        (unsigned short)(a1 >> 3);
}
