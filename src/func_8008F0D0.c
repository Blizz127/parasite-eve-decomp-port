void func_8008F0D0(unsigned char *a0, unsigned char *a1, int a2) {
    *(int *)(a0 + 0xF8) = a2;
    *(int *)(a0 + 0xFC) = *(int *)(a1 + 4);
    *(unsigned short *)(a0 + 0x10E) = a1[8];
    *(unsigned short *)(a0 + 0x110) = a1[9];
    *(unsigned short *)(a0 + 0x112) = a1[10];
    *(unsigned short *)(a0 + 0x114) = a1[11];
    *(unsigned int *)(a0 + 0x100) = a1[13];
    *(unsigned int *)(a0 + 0x104) = a1[14];
    if (*(unsigned int *)(a0 + 0x38) & 0x200) {
        *(unsigned int *)(a0 + 0xF4) |= 0x1BB80;
    } else {
        *(unsigned short *)(a0 + 0x116) = a1[12];
        *(unsigned int *)(a0 + 0x108) = a1[15];
        *(unsigned int *)(a0 + 0xF4) |= 0x1FF80;
    }
}
