extern int D_8009D048;
extern int D_8009D04C;
extern int D_8009D050;
extern int D_8009D054;
extern int D_8009D058;
extern int D_8009D064;
extern int D_8009D028;
extern int D_800A1F84;
extern int D_800C0E48;
extern int D_8009D05C;
extern unsigned short D_800C0E06[];
extern unsigned char D_800BEEAC[];
extern unsigned char D_8009DE64[];
extern int func_80052F70();
extern unsigned char *func_8005DB44(int);
extern int func_8004E970(void);

static inline void sel(int a0)
{
    if (a0 != 0 && D_8009D04C != 0) {
        int v = D_8009D054;
        D_8009D048 = D_8009D04C;
        D_8009D058 = (int)&D_800A1F84;
        D_8009D064 = 4;
        D_8009D050 = v;
    } else {
        D_8009D048 = (int)&D_800C0E48;
        D_8009D050 = func_80052F70();
        D_8009D058 = (int)&D_8009D05C;
        D_8009D064 = 2;
    }
}

static inline unsigned char *get(int i)
{
    int v;
    int w;
    unsigned char *res;

    if (i >= 0 && i < D_8009D050) {
        v = ((short *)D_8009D048)[i];
        w = v;
        if ((unsigned int)(v - 0x100) < 0x80) {
            res = (unsigned char *)((v << 5) + (int)D_800BEEAC);
            goto done;
        }
        if ((unsigned int)(v - 1) < 0xFF) {
            res = func_8005DB44(v - 1);
            goto done;
        }
        if ((unsigned int)(w - 0x200) < 9) {
            res = (unsigned char *)((w << 5) + (int)D_8009DE64);
            goto done;
        }
        res = 0;
    done:
        return res;
    }
    return 0;
}

static inline int a6(int j)
{
    unsigned char *p = get(j);

    return p ? p[6] : 0;
}

void func_80055760(void)
{
    int i;
    int j;
    int k;
    int f1;
    int f2;
    int was;
    unsigned char *p;
    int b;
    int n;
    unsigned short *h;
    int x;

    f1 = 0;
    f2 = 0;
    was = D_8009D048 != (int)&D_800C0E48;
    i = 0;
loop1:
    n = 1;
    if (D_8009D04C != 0) {
        n = 2;
    }
    if (i < n) {
        sel(i);
        for (j = 0; j < D_8009D050; j++) {
            if ((0xFE >> a6(j)) & 1) {
                break;
            }
        }
        f1 |= j < D_8009D050;
        for (j = 0; j < D_8009D050; j++) {
            if (a6(j) == 9) {
                break;
            }
        }
        f2 |= j < D_8009D050;
        i++;
        goto loop1;
    }
    i = 0;
    h = &D_800C0E06[1];
loop2:
    n = 1;
    if (D_8009D04C != 0) {
        n = 2;
    }
    if (i < n) {
        sel(i);
        for (p = 0; (int)p < D_8009D064; p++) {
            ((int *)D_8009D058)[(int)p] = 0;
        }
        if (func_8004E970() != 0) {
            for (j = 0; j < D_8009D050; j++) {
                p = get(j);
                if (p != 0) {
                    x = p[5];
                    if (D_8009D028 != 0) {
                        b = (x >> 1) & 1;
                    } else {
                        b = x & 1;
                    }
                    if (p[6] == 10 && p[0xE] == 2) {
                        b = 0;
                    }
                    if ((unsigned int)(p[4] - 6) < 5 && !(h[0] < h[-1])) {
                        b = 0;
                    }
                    ((int *)D_8009D058)[j >> 5] |= b << (j & 0x1F);
                }
            }
        } else {
            for (j = 0; j < D_8009D050; j++) {
                p = get(j);
                if (p != 0) {
                    x = p[5];
                    if (D_8009D028 != 0) {
                        b = (x >> 1) & 1;
                    } else {
                        b = x & 1;
                    }
                    if (p[6] == 10) {
                        if ((unsigned int)(p[0xE] - 4) < 3) {
                            b &= f1;
                        } else if ((unsigned int)(p[0xE] - 0xC) < 3) {
                            b &= f2;
                        }
                    }
                    if ((unsigned int)(p[4] - 6) < 5 && !(h[0] < h[-1])) {
                        b = 0;
                    }
                    ((int *)D_8009D058)[j >> 5] |= b << (j & 0x1F);
                }
            }
        }
        i++;
        goto loop2;
    }
    asm volatile("" : : : "memory");
    sel(was);
}
