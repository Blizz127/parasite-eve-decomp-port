extern unsigned char *volatile D_800B1624;
extern unsigned int D_800BCF88;

int func_800677A0(int a0, short a1, short a2)
{
    unsigned char *p = D_800B1624;
    unsigned char *q = D_800B1624;
    unsigned int *s;

    q += *(unsigned int *)(p + 0x14);
    q += a0 * 56;
    *(short *)(q + 0xC) = a1;
    *(short *)(q + 0x8) = a1;
    *(short *)(q + 0xE) = a2;
    *(short *)(q + 0xA) = a2;
    s = &D_800BCF88;
    *s = *s | 0x80;
    return 0;
}
