void func_80090054(unsigned char *a0) {
    *(unsigned int *)(a0 + 0x38) &= ~2;
    *(unsigned int *)(a0 + 0xF4) |= 3;
    *(unsigned short *)(a0 + 0xEA) = 0;
}
