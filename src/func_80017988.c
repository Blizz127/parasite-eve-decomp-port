extern unsigned int *D_8009D2F0;
extern unsigned int *D_8009D300;

int func_80017988(void) {
    unsigned int *base;
    unsigned short *q;
    unsigned short *p;
    unsigned char i;

    i = 0;
    base = D_8009D2F0;
    q = (unsigned short *)D_8009D300;
    do {
        p = (unsigned short *)base[i + 0x28];
        while (p != 0) {
            if (p != q) {
                p[4] |= 0x10;
            }
            p = *(unsigned short **)((unsigned char *)p + 0x24);
        }
        i++;
    } while (i < 3);
    return 1;
}
