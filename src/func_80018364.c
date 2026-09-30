extern unsigned int *D_8009D2F0;

int func_80018364(void) {
    unsigned int *base;
    unsigned int *q;
    unsigned short *p;
    unsigned int i;
    unsigned int one;
    unsigned int v;
    unsigned short f;

    i = 0;
    one = 1;
    base = D_8009D2F0;
    do {
        q = (unsigned int *)base[0x28];
        while (q != 0) {
            p = (unsigned short *)q;
            f = p[4];
            if (f & 0x40) {
                p[4] = f & 0xFFBF;
                v = q[1];
                if (v != 0) {
                    q[0] = v;
                    q[4] = one;
                    p[4] &= 0xFFDF;
                }
            }
            q = (unsigned int *)q[9];
        }
        i++;
        base++;
    } while (i < 3);
    return 1;
}
