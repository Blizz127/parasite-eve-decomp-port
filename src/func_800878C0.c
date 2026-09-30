void func_800878C0(int voice, unsigned int low, unsigned int mode) {
    unsigned short *reg =
        (unsigned short *)(0x1F801C0A + (voice << 4));

    *reg = (unsigned short)((*reg & 0xFFC0) |
                            (((mode >> 2) << 5) | low));
}
