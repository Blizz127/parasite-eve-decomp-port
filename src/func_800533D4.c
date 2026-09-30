extern int D_8009D050;
extern short *D_8009D048;
extern unsigned char D_800BEEAC[];
extern unsigned char D_8009DE64[];
extern unsigned char *func_8005DB44(int);

int func_800533D4(unsigned char *target)
{
    int unused[2];
    int i;
    int n;
    int v;
    int w;
    unsigned char *p;
    int r;

    i = 0;
    n = D_8009D050;
    if (n > 0) {
        do {
            if (i >= 0 && i < n) {
                v = D_8009D048[i];
                w = v;
                if ((unsigned int)(v - 0x100) < 0x80) {
                    p = (unsigned char *)((v << 5) + (int)D_800BEEAC);
                    goto got;
                }
                if ((unsigned int)(v - 1) < 0xFF) {
                    p = func_8005DB44(v - 1);
                    goto got;
                }
                if ((unsigned int)(w - 0x200) < 9) {
                    p = (unsigned char *)((w << 5) + (int)D_8009DE64);
                    goto got;
                }
            }
            p = 0;
        got:
            if (p == target) {
                break;
            }
            n = D_8009D050;
            i++;
        } while (i < n);
    }
    r = -1;
    if (i < D_8009D050) {
        r = i;
    }
    return r;
}
