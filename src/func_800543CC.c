extern short *D_8009D048;
extern int D_8009D050;
extern int D_8009D040;
extern int D_8009D068;
extern short D_800A1D9C[];
extern unsigned char D_800BEEAC[];
extern unsigned char D_8009DE64[];
extern unsigned char *func_8005DB44(int);

void func_800543CC(int mask, int skip)
{
    short *out;
    int i;
    int v;
    int w;
    unsigned char *res;
    unsigned char *p;

    out = D_800A1D9C;
    for (i = 0; i < D_8009D050; i++) {
        if (i >= 0 && i < D_8009D050) {
            v = D_8009D048[i];
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
        if (p != 0 && ((mask >> p[6]) & 1) && p[0x14] != 0 && i != skip) {
            *out++ = i;
        }
    }
    D_8009D068 = 0;
    D_8009D040 = out - D_800A1D9C;
}
