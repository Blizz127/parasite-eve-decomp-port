extern unsigned char *D_800E2368;
extern unsigned char *D_800F33E0;

int func_800D401C(int a0) {
    unsigned char *base;
    unsigned char *p;
    unsigned char *cur;
    unsigned int sz;
    int i;
    register unsigned int adv asm("$2");
    int r;
    register unsigned int sentinel asm("$3");
    register unsigned int szc asm("$3");

    i = 0;
    base = D_800E2368;
    sentinel = 0xFFFF;
    __asm__ __volatile__("" : "=r"(sentinel) : "0"(sentinel));
    p = base + 0x20;
scan:
    if (*(unsigned short *)p == sentinel) {
        goto found;
    }
    i++;
    p += 0xC;
    if (i < 8) {
        goto scan;
    }
found:
    if (i == 8) {
        return -1;
    }
    base[0xC] = base[0xC] + 1;
    *(unsigned short *)p = a0;
    *(unsigned short *)(p + 2) = 0;
    { unsigned char *tab = *(unsigned char **)(base + 0x80);
      sz = *(unsigned short *)(tab + a0 * 2 + 0x20); }
    {
        unsigned int t = *(unsigned short *)(base + 0x10) + sz;
        *(unsigned short *)(base + 0x10) = t;
        cur = *(unsigned char **)(base + 4);
        szc = sz;
        if ((t & 0xFFFF) >= 0x97D) {
            cur = base + 0x84;
            *(unsigned short *)(base + 0x10) = szc;
            *(unsigned char **)(base + 4) = cur;
        }
    }
    *(unsigned char **)(p + 8) = cur + sz;
    *(unsigned char **)(p + 4) = cur;
    {
        int (*f)();
        unsigned int idx = *(unsigned short *)p;
        unsigned char *tb = *(unsigned char **)(base + 0x80);
        unsigned char *arg2 = *(unsigned char **)(base + 8);
        f = *(int (**)())(tb + idx * 4);
        D_800F33E0 = p;
        r = f(0, cur, arg2);
    }
    adv = sz + r;
    if (r == 0) {
        *(unsigned char **)(p + 8) = 0;
    }
    {
        register unsigned char *c4 asm("$3");
        register unsigned int c10 asm("$4");
        c4 = *(unsigned char **)(base + 4);
        c10 = *(unsigned short *)(base + 0x10);
        c4 = c4 + adv;
        c10 = c10 + r;
        *(unsigned char **)(base + 4) = c4;
        *(unsigned short *)(base + 0x10) = c10;
    }
}
