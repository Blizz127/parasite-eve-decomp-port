void func_8008FED8(unsigned char *a0) {
    *(unsigned int *)(a0 + 0x38) &= ~1u;
    *(unsigned short *)(a0 + 0xE8) = 0;
    *(unsigned int *)(a0 + 0xF4) |= 0x10u;
}
