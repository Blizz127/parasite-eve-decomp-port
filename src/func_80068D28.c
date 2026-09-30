extern int D_800BCF88;
extern unsigned short D_800BCFE8;
extern unsigned short D_800BCFEA;
extern unsigned short D_800BCFEC;
extern unsigned char D_800BCFEE;
extern unsigned char D_800BCFEF;
extern short D_800BCFF0;
extern short D_800BCFF2;
extern short D_800BCFF4;
extern short D_800BCFF6;
extern short D_800BCFF8;

int func_80068D28(void) {
    char *base;
    int i;
    unsigned char v0;
    unsigned short v1;

    base = (char *)&D_800BCF88;

    D_800BCFEC = 0xFF;
    D_800BCFEA = 0xFF;
    D_800BCFE8 = 0xFF;
    D_800BCFEE = 1;
    D_800BCFF4 = 0;
    D_800BCFF2 = 0;
    D_800BCFF0 = 0;
    D_800BCFEF = 2;

    for (i = 0; i < 2; i++) {
        base[i * 0x10 + 0x33] = 3;
        base[i * 0x10 + 0x37] = 0x60;
        base[i * 0x10 + 0x34] = *(unsigned short *)(base + 0x60);
        base[i * 0x10 + 0x35] = *(unsigned short *)(base + 0x62);
        v1 = *(unsigned short *)(base + 0x64);
        v0 = base[i * 0x10 + 0x37];
        *(short *)(base + i * 0x10 + 0x38) = 0;
        *(short *)(base + i * 0x10 + 0x3A) = 0;
        *(short *)(base + i * 0x10 + 0x3C) = 0x140;
        *(short *)(base + i * 0x10 + 0x3E) = 0xE0;
        base[i * 0x10 + 0x37] = v0 | 2;
        base[i * 0x10 + 0x36] = v1;
        base[i * 8 + 0x53] = 1;
        *(int *)(base + i * 8 + 0x54) =
            0xE1000400 | ((*(unsigned char *)(base + 0x67) & 3) << 5);
    }

    *(short *)(base + 0x6E) = 0;
    *(short *)(base + 0x70) = 0;
    return 0;
}
