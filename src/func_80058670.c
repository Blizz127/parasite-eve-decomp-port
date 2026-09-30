extern short *D_8009D048;
extern int D_8009D050;
extern unsigned char *D_8009D058;
extern int D_8009D064;
extern int D_8009D04C;
extern int D_8009D054;
extern unsigned char D_8009D05C[];
extern unsigned char D_800C0E48[];
extern unsigned char D_800C0EAC[];
extern unsigned char D_800BEEAC[];
extern unsigned char D_8009DE64[];
extern unsigned char *func_8005DB44(int);
extern int func_80052F70(void);

typedef struct { unsigned char b[32]; } Rec;

void func_80058670(void)
{
    int unused[2];
    int i;
    int v;
    int w;
    int k;
    unsigned char *res;
    unsigned char *p;
    int idx;
    unsigned char *dst;
    unsigned int u;
    unsigned char *g;
    unsigned char *lo;
    unsigned char *hi;
    unsigned char *end;

    D_8009D04C = 0;
    D_8009D054 = 0;
    g = D_800C0E48;
    D_8009D048 = (short *)g;
    D_8009D050 = func_80052F70();
    D_8009D058 = D_8009D05C;
    D_8009D064 = 2;
    i = 0;
    if (D_8009D050 > 0) {
        hi = g + 0x1064;
        lo = g + 0x64;
        do {
            if (i >= 0 && i < D_8009D050) {
                v = D_8009D048[i];
                w = v;
                if ((unsigned int)(v - 0x100) < 0x80) {
                    res = (unsigned char *)((v << 5) + (int)D_800BEEAC);
                    goto d1;
                }
                if ((unsigned int)(v - 1) < 0xFF) {
                    res = func_8005DB44(v - 1);
                    goto d1;
                }
                if ((unsigned int)(w - 0x200) < 9) {
                    res = (unsigned char *)((w << 5) + (int)D_8009DE64);
                    goto d1;
                }
            }
            res = 0;
        d1:
            if (res != 0) {
                k = res[6];
            } else {
                k = 0;
            }
            if ((unsigned int)(k - 1) < 9) {
                u = *(unsigned short *)&D_8009D048[i];
                if ((unsigned int)(u - 0x100) >= 0x80) {
                    p = D_800C0EAC;
                    end = p + 0x1000;
                    if (p < hi) {
                        do {
                            if (*p == 0) {
                                break;
                            }
                            p += 0x20;
                        } while (p < end);
                        if (p < hi) {
                            idx = (p - lo) >> 5;
                            goto d2;
                        }
                    }
                    idx = -1;
                d2:
                    if (idx >= 0) {
                        dst = (unsigned char *)((idx << 5) + (int)lo);
                        *(Rec *)dst = *(Rec *)func_8005DB44(D_8009D048[i] - 1);
                        D_8009D048[i] = idx + 0x100;
                    }
                }
            }
            i++;
        } while (i < D_8009D050);
    }
}
