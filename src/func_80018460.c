extern unsigned int *D_8009D2F0;

int func_80018460(int **a0) {
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
            if (p[5] == **a0) {
                f = p[4];
                v = q[1];
                p[4] = f & 0xFFBF;
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
