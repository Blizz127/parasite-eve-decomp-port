void func_800877D4(int a0, unsigned int a1) {
    *(volatile unsigned short *)(0x1F801C06 + (a0 << 4)) =
        (unsigned short)(a1 >> 3);
}
