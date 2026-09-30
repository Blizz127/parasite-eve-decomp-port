unsigned int func_80089FE0(unsigned char *a0, unsigned int a1)
{
    unsigned int i;
    unsigned int mask;
    unsigned int v;

    i = 0;
    mask = 0;
    do {
        if (a1 & (1 << i)) {
            v = *(unsigned int *)(a0 + 0xF0);
            if (v < 0x18) {
                mask |= 1 << v;
            }
        }
        i++;
        a0 += 0x11C;
    } while (i < 0x18);
    return mask;
}
