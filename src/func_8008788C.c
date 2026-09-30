void func_8008788C(int voice, unsigned int field, unsigned int mode) {
    unsigned short *reg =
        (unsigned short *)(0x1F801C0A + (voice << 4));

    *reg = (unsigned short)((*reg & 0x003F) |
                            (((mode >> 1) << 14) | (field << 6)));
}
