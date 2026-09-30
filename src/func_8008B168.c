extern void func_8008A92C(unsigned char *a0, unsigned int a1, unsigned int a2);

void func_8008B168(unsigned char *a0)
{
    unsigned char *base;
    unsigned int p1;
    unsigned int p2;
    unsigned int o;

    base = *(unsigned char **)(a0 + 4);
    p1 = *(unsigned short *)base;
    if (p1 != 0xFFFF) {
        p1 = p1 + (unsigned int)base + 4;
    } else {
        p1 = 0;
    }
    o = ((unsigned short *)base)[1];
    p2 = 0;
    if (o != 0xFFFF) {
        p2 = o + *(unsigned int *)(a0 + 4) + 4;
    }
    *(unsigned char **)(a0 + 4) = *(unsigned char **)(a0 + 0x14);
    func_8008A92C(a0, p1, p2);
}
