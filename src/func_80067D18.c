extern unsigned int D_800BCF88;
extern unsigned char D_800BCFFC;
extern unsigned char *volatile D_800B1624;
extern int D_8009CDDC;

int func_80067D18(void)
{
    char pad[16];
    register unsigned int f asm("$2");
    unsigned int *fp;
    register unsigned int t asm("$3");
    register unsigned int f1 asm("$3");
    register unsigned int r asm("$2");
    unsigned char *p0;
    register unsigned char *e asm("$4");
    register unsigned char *q asm("$3");
    register int i asm("$8");
    register int n asm("$9");
    register unsigned int k asm("$5");
    register unsigned int c asm("$6");
    register int col asm("$5");
    register int cc asm("$7");

    f1 = D_800BCF88;
    if ((f1 & 0x1000) != 0) {
        col = 0x80;
        r = f1 & 0x2000;
        if (r != 0) {
            col = D_800BCFFC;
        }
        i = 0;
        p0 = D_800B1624;
        e = D_800B1624;
        n = *(unsigned short *)(p0 + 6);
        e = e + *(unsigned int *)(p0 + 0x14);
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
        fp = &D_800BCF88;
        f = *fp;
        t = f & 0xFFFF3FFF;
        *fp = t;
        switch (f & 0xC000) {
        case 0x4000:
            r = t | 0x8000;
            *fp = r;
            break;
        case 0x8000:
            r = t & -0x1001;
            *fp = r;
            break;
        }
    }
    return 0;
}
