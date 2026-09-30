extern unsigned char D_800B2900[];

void func_8008FB54(unsigned char *a0) {
    unsigned char *r;
    unsigned int t;

    r = &D_800B2900[*(unsigned short *)(a0 + 0x5A) * 0x40];
    *(unsigned short *)(a0 + 0x10E) = r[8];
    *(unsigned short *)(a0 + 0x110) = r[9];
    *(unsigned short *)(a0 + 0x112) = r[10];
    *(unsigned short *)(a0 + 0x114) = r[11];
    *(unsigned short *)(a0 + 0x116) = r[12];
    *(unsigned int *)(a0 + 0x100) = r[13];
    *(unsigned int *)(a0 + 0x104) = r[14];
    t = r[15];
    *(unsigned int *)(a0 + 0xF4) |= 0xFF00;
    *(unsigned int *)(a0 + 0x108) = t;
}
