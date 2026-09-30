extern unsigned int *D_8009D2F0;
extern unsigned int *D_8009D300;

int func_80018300(void) {
    unsigned int *base;
    unsigned short *q;
    unsigned short *p;
    unsigned int i;

    i = 0;
    q = (unsigned short *)D_8009D300;
    base = D_8009D2F0;
    do {
        p = (unsigned short *)base[0x28];
        while (p != 0) {
            if (p != q) {
                p[4] |= 0x40;
            }
            p = *(unsigned short **)((unsigned char *)p + 0x24);
        }
        i++;
        base++;
    } while (i < 3);
    return 1;
}
