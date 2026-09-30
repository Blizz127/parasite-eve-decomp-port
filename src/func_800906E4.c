extern unsigned char D_800B290C[];
void func_800906E4(unsigned char *a0) {
    unsigned int v = *(unsigned int *)(a0 + 0x38);
    unsigned char byte;
    unsigned int f;
    v &= ~0x200u;
    *(unsigned int *)(a0 + 0x38) = v;
    byte = D_800B290C[*(unsigned short *)(a0 + 0x5A) << 6];
    f = *(unsigned int *)(a0 + 0xF4) | 0x4400;
    *(unsigned int *)(a0 + 0xF4) = f;
    *(unsigned short *)(a0 + 0x116) = byte;
}
