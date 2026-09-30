extern short *D_8009D048;
extern int D_8009D050;
extern unsigned int *D_8009D058;
extern int D_8009D064;
extern int D_8009D040;
extern short D_800A1D9C[];
extern unsigned char D_800C0E48[];
extern unsigned int D_8009D05C[];
extern unsigned char D_800BEEAC[];
extern unsigned char D_8009DE64[];
extern unsigned char *func_8005DB44(int);
extern int func_80052F70(void);
extern void func_800542A0(int);

static inline int get_sel(int i)
{
    int r;

    if (i >= 0 && i < D_8009D040) {
        r = D_800A1D9C[i];
    } else {
        r = 0;
    }
    return r;
}

static inline void clear_bits(void)
{
    int k;

    for (k = 0; k < D_8009D064; k++) {
        D_8009D058[k] = 0;
    }
}

int func_8005CAEC(void)
{
    unsigned char *p;
    unsigned char *res;
    int n;
    int i;
    int idx;
    int v;
    int w;

    D_8009D048 = (short *)D_800C0E48;
    D_8009D050 = func_80052F70();
    D_8009D058 = D_8009D05C;
    D_8009D064 = 2;
    func_800542A0(0x3FE);
    clear_bits();
    n = 0;
    i = 0;
    while (1) {
        if (i >= D_8009D040) {
            break;
        }
        idx = get_sel(i);
        if (idx >= 0 && idx < D_8009D050) {
            v = D_8009D048[idx];
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
            p = res;
        } else {
            p = 0;
        }
        if (p[0x14] < p[1]) {
            n++;
            D_8009D058[i >> 5] |= 1 << (i & 0x1F);
        }
        i++;
    }
    return n;
}
