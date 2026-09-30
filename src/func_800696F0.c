extern unsigned int D_8009D1A0;
extern unsigned char D_800B0CD8;
extern void **volatile D_800942E0;
extern int D_800E10BC;

int func_800696F0(void)
{
    char pad[8];
    unsigned char *g;
    unsigned char *hdr;
    unsigned char *tbl;
    unsigned char *e;
    unsigned char *p;
    void **t;
    void (*f)();
    unsigned int w;
    unsigned int nn;
    unsigned int mask;
    register unsigned int off asm("$2");
    register unsigned int id asm("$4");
    int i;
    int *q;
    register void **r asm("$3");

    g = &D_800B0CD8;
    if ((D_8009D1A0 & 0x80) != 0) {
        for (i = 0; i < 8; i++) {
            t = (void **)D_800942E0[i];
            if (t != 0) {
                f = (void (*)())t[6];
                if (f != 0) {
                    f();
                }
            }
        }
        t = (void **)D_800942E0[0x55];
        if (t != 0) {
            f = (void (*)())t[6];
            if (f != 0) {
                f();
            }
        }
        hdr = *(unsigned char **)(g + 0x18C);
        mask = 0x3FFFFF;
        tbl = hdr + *(unsigned int *)(hdr + 4);
        w = *(unsigned int *)(tbl + 4);
        i = 0;
        e = hdr + (w & mask);
        if ((int)(w >> 22) > 0) {
            p = e;
            __asm__ __volatile__("" : "=r"(p) : "0"(p));
            do {
                id = p[7];
                __asm__ __volatile__("" : "=r"(id) : "0"(id));
                if ((id - 8) < 0x4D) {
                    off = id * 4;
                    __asm__ __volatile__("" : "=r"(off) : "0"(off));
                    t = *(void ***)(off + (unsigned int)D_800942E0);
                    if (t != 0) {
                        f = (void (*)())t[6];
                        if (f != 0) {
                            f();
                        }
                    }
                }
                nn = *(unsigned int *)(tbl + 4);
                __asm__ __volatile__("" : "=r"(nn) : "0"(nn));
                i++;
                p += 0xC;
            } while (i < (int)(nn >> 22));
        }
        D_8009D1A0 = D_8009D1A0 & ~0x80;
    }
    for (i = 8; i < 0x55; i++) {
        r = (void **)(i * 4 + (unsigned int)D_800942E0);
        if (*r != 0) {
            *r = 0;
        }
    }
    i = 0x1E;
    q = &D_800E10BC;
    for (; (unsigned int)i < 0x68; i++) {
        if (*q != 0) {
            *q = 0;
        }
        q++;
    }
    return 0;
}
