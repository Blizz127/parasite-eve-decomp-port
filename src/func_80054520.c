extern short *D_8009D048;
extern int D_8009D050;
extern unsigned char *D_8009D058;
extern int D_8009D064;
extern short *D_8009D04C;
extern int D_8009D054;
extern unsigned char D_8009D05C[];
extern unsigned char D_800C0E48[];
extern unsigned char D_800A1F84[];
extern unsigned char D_800BEEAC[];
extern unsigned char D_8009DE64[];
extern unsigned char *func_8005DB44(int);
extern int func_80052F70(void);

int func_80054520(int mask)
{
    int unused[4];
    int i;
    int cnt;
    int s3;
    int add;
    int v;
    register int w asm("$5");
    register int m asm("$3");
    int bnd;
    int t;
    int n2;
    unsigned char *p;
    unsigned char *res;
    short *d;

    i = 0;
    cnt = 0;
    s3 = (D_8009D048 != (short *)D_800C0E48);
    D_8009D048 = (short *)D_800C0E48;
    D_8009D050 = func_80052F70();
    D_8009D058 = D_8009D05C;
    D_8009D064 = 2;
    if (D_8009D050 > 0) {
        do {
            if (i >= 0 && i < D_8009D050) {
                v = D_8009D048[i];
                w = v;
                if ((unsigned int)(v - 0x100) < 0x80) {
                    res = (unsigned char *)((v << 5) + (int)D_800BEEAC);
                    goto d1a;
                }
                if ((unsigned int)(v - 1) < 0xFF) {
                    res = func_8005DB44(v - 1);
                    goto d1a;
                }
                if ((unsigned int)(w - 0x200) < 9) {
                    m = w << 5;
                    res = (unsigned char *)(m + (int)D_8009DE64);
                    goto d1a;
                }
                res = 0;
            d1a:
                p = res;
            } else {
                p = 0;
            }
            add = 0;
            if (p != 0) {
                if (((mask >> p[6]) & 1) != 0) {
                    add = (p[0x14] != 0);
                }
            }
            bnd = D_8009D050;
            __asm__ __volatile__("" : "=r"(bnd) : "0"(bnd));
            i++;
            cnt += add;
        } while (i < bnd);
    }
    d = D_8009D04C;
    t = (d != 0) ? 2 : 1;
    if (t >= 2 && (mask & 0x100) == 0) {
        if (d != 0) {
            D_8009D048 = d;
            D_8009D058 = D_800A1F84;
            D_8009D064 = 4;
            D_8009D050 = D_8009D054;
        } else {
            D_8009D048 = (short *)D_800C0E48;
            D_8009D050 = func_80052F70();
            D_8009D058 = D_8009D05C;
            D_8009D064 = 2;
        }
        n2 = D_8009D050;
        i = 0;
        if (n2 > 0) {
            do {
                if (i >= 0 && i < n2) {
                    v = D_8009D048[i];
                    w = v;
                    if ((unsigned int)(v - 0x100) < 0x80) {
                        res = (unsigned char *)((v << 5) + (int)D_800BEEAC);
                        goto d2a;
                    }
                    if ((unsigned int)(v - 1) < 0xFF) {
                        res = func_8005DB44(v - 1);
                        goto d2a;
                    }
                    if ((unsigned int)(w - 0x200) < 9) {
                        m = w << 5;
                    res = (unsigned char *)(m + (int)D_8009DE64);
                        goto d2a;
                    }
                    res = 0;
                d2a:
                    p = res;
                } else {
                    p = 0;
                }
                add = 0;
                if (p != 0) {
                    if (((mask >> p[6]) & 1) != 0) {
                        add = (p[0x14] != 0);
                    }
                }
                n2 = D_8009D050;
                __asm__ __volatile__("" : "=r"(n2) : "0"(n2));
                i++;
                cnt += add;
            } while (i < n2);
        }
    }
    if (s3 != 0 && D_8009D04C != 0) {
        D_8009D048 = D_8009D04C;
        D_8009D058 = D_800A1F84;
        D_8009D064 = 4;
        D_8009D050 = D_8009D054;
    } else {
        D_8009D048 = (short *)D_800C0E48;
        D_8009D050 = func_80052F70();
        D_8009D058 = D_8009D05C;
        D_8009D064 = 2;
    }
    return !(cnt < 2);
}
