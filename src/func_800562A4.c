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

#define GROUP(q) ((((q)[6] != 0) && ((q)[6] < 8)) ? ((((q)[6] - 4) > 0) ? ((q)[6] - 4) : 1) : (((q)[6] < 0x13) ? 0 : ((q)[6] - 0x12)))

static inline void clear_bits(void)
{
    int k;

    for (k = 0; k < D_8009D064; k++) {
        ((int *)D_8009D058)[k] = 0;
    }
}

static inline int count_bits(void)
{
    int k;
    int n;
    int one;

    n = 0;
    for (k = 0; k < D_8009D050; k++) {
        int *b = (int *)D_8009D058;
        one = 1;
        n += (b[k >> 5] & (one << (k & 0x1F))) != 0;
    }
    return n;
}

static inline int npass2(void)
{
    return D_8009D04C ? 2 : 1;
}

#define GROUPC(c, q) ((((c) != 0) && ((c) < 8)) ? ((((c) - 4) > 0) ? ((c) - 4) : 1) : (((q)[6] < 0x13) ? 0 : ((q)[6] - 0x12)))
int func_800562A4(int idx)
{
    unsigned char *p;
    unsigned char *q;
    int t;
    int g;
    int j;
    int n;
    int was;
    int one;
    int one2;

    q = get(idx);
    t = q[6];
    g = GROUP(q);
    clear_bits();
    if ((unsigned int)(t - 0x13) < 3) {
        for (j = 0; j < D_8009D050; j++) {
            one = 1;
            if (j == idx) {
                continue;
            }
            q = get(j);
            if (q != 0) {
                int *w = (int *)((j >> 5) * 4 + D_8009D058);
                int old = *w;
                int sh = j & 0x1F;
                int v;

                if (g == GROUP(q)) {
                    v = old | (one << sh);
                } else {
                    v = old;
                }
                asm("" : : "r"(sh), "r"(one));
                *w = v;
            }
        }
    } else {
        for (j = 0; j < D_8009D050; j++) {
            if (j == idx) {
                continue;
            }
            q = get(j);
            if (q != 0) {
                int bit = 0;
                int *w = (int *)((j >> 5) * 4 + D_8009D058);
                int *w2 = w;
                int tt = q[6];

                if ((unsigned int)(tt - 0x13) < 3 && g == GROUPC((unsigned char)tt, q)) {
                    bit = 1;
                }
                *w = *w2 | (bit << (j & 0x1F));
            }
        }
    }
    n = count_bits();
    was = D_8009D048 != (int)&D_800C0E48;
    if (!was && npass2() != 2) {
        return n;
    }
    asm("" : : "r"(was));
    sel(!was);
    clear_bits();
    if ((unsigned int)(t - 0x13) < 3) {
        for (j = 0; j < D_8009D050; j++) {
            one2 = 1;
            q = get(j);
            if (q != 0) {
                int *w = (int *)((j >> 5) * 4 + D_8009D058);
                int old = *w;
                int sh = j & 0x1F;
                int v;

                if (g == GROUP(q)) {
                    v = old | (one2 << sh);
                } else {
                    v = old;
                }
                asm("" : : "r"(sh), "r"(one2));
                *w = v;
            }
        }
    } else {
        for (j = 0; j < D_8009D050; j++) {
            q = get(j);
            if (q != 0) {
                int bit = 0;
                int *w = (int *)((j >> 5) * 4 + D_8009D058);
                int *w2 = w;
                int tt = q[6];

                if ((unsigned int)(tt - 0x13) < 3 && g == GROUPC((unsigned char)tt, q)) {
                    bit = 1;
                }
                *w = *w2 | (bit << (j & 0x1F));
            }
        }
    }
    n += count_bits();
    sel(was);
    return n;
}
