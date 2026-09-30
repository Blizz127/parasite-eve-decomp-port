extern unsigned int D_800A8030;

unsigned char *func_8005DADC(int a0) {
    unsigned int *address = &D_800A8030;
    unsigned int offset = *address;
    unsigned char *base = (unsigned char *)address - 8;

    return base + offset + (a0 * 8);
}
