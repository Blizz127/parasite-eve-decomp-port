extern unsigned int D_800BCF88;
extern unsigned char D_800BCFFA;
extern unsigned char D_800BCFFB;
extern unsigned char *volatile D_800B1624;
extern int D_8009CDDC;

int func_80067B74(void)
{
    char pad[16];
    register unsigned char *e asm("$4");
    register unsigned char *q asm("$3");
    register int i asm("$8");
    register int n asm("$9");
    register unsigned int k asm("$5");
    register unsigned int c asm("$6");
    register int col asm("$2");
    register int cc asm("$7");
    unsigned char *p0;
    unsigned char *p;
    unsigned char *h;
    register int na asm("$3");
    register int nb asm("$2");
    int d;
    unsigned int t;

    if ((D_800BCF88 & 0x400) != 0) {
        na = D_800BCFFB;
        nb = D_800BCFFA;
        na = na << 7;
        nb = nb - 1;
        d = na / nb;
        i = 0;
        p0 = D_800B1624;
        e = D_800B1624;
        n = *(unsigned short *)(p0 + 6);
        e = e + *(unsigned int *)(p0 + 0x14);
        col = 0x80 - d;
        if (n != 0) {
            cc = col;
            do {
                q = *(unsigned char **)(e + 0x30);
                c = *(unsigned short *)(e + 0x26);
                if (D_8009CDDC != 0) {
                    q = q + (c << 4);
                }
                k = 0;
                if (c != 0) {
                    do {
                        q[6] = cc;
                        q[5] = cc;
                        q[4] = cc;
                        __asm__ __volatile__("" ::: "memory");
                        k++;
                        q += 0x10;
                    } while (k < c);
                }
                i++;
                e += 0x38;
            } while (i < n);
        }
        p = &D_800BCFFB;
        t = *p + 1;
        *p = t;
        if ((t & 0xFF) >= D_800BCFFA) {
            h = D_800B1624;
            D_800BCF88 = (D_800BCF88 & -0xC01) | 0x800;
            *(short *)(h + 0x26) = 0x1FF0;
        }
    }
    return 0;
}
