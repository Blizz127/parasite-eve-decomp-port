extern unsigned int *D_8009D2F0;

int func_800183E8(int **a0) {
    unsigned int *base;
    unsigned short *p;
    unsigned int i;

    i = 0;
    base = D_8009D2F0;
    do {
        p = (unsigned short *)base[0x28];
        while (p != 0) {
            if (p[5] == **a0) {
                p[4] |= 0x40;
                return 1;
            }
            p = *(unsigned short **)((unsigned char *)p + 0x24);
        }
        i++;
        base++;
    } while (i < 3);
    return 1;
}
