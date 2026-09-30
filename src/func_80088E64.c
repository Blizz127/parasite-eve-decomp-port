extern unsigned char D_800B8AC0[];
extern void func_800878F0();

void func_80088E64(unsigned char *a0, int a1)
{
    unsigned char *q;
    int r;
    int l;

    r = 0x7F - (*(short *)(a0 + 0xD8) >> 8);
    q = D_800B8AC0 + a1 * 0x11C;
    l = *(short *)(a0 + 0x118);
    *(short *)(a0 + 0x118) = (unsigned int)(l * r) >> 8;
    *(short *)(q + 0x118) = (l * *(short *)(a0 + 0xD8)) >> 16;
    l = *(short *)(a0 + 0x11A);
    *(short *)(a0 + 0x11A) = (unsigned int)(l * r) >> 8;
    *(short *)(q + 0x11A) = (l * *(short *)(a0 + 0xD8)) >> 16;
    *(unsigned short *)(q + 0x10C) = *(unsigned short *)(a0 + 0x10C);
    *(unsigned int *)(q + 0xF4) |= *(unsigned int *)(a0 + 0xF4);
    func_800878F0(*(int *)(a0 + 0xF0), a0 + 0xF0, *(int *)(a0 + 0x38));
    func_800878F0(a1, q + 0xF0, *(int *)(a0 + 0x38));
}
