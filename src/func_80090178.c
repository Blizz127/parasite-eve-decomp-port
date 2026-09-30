void func_80090178(unsigned char *a0) {
    *(unsigned int *)(a0 + 0x38) &= ~4;
    *(unsigned int *)(a0 + 0xF4) |= 3;
    *(unsigned short *)(a0 + 0xEC) = 0;
}
