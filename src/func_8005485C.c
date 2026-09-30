extern short *D_8009D048;
extern int D_8009D050;
extern int D_8009D038;
extern int D_8009D040;
extern int D_8009D068;
extern unsigned char D_800C0E48[];
extern signed char D_800C0E20[];
extern signed char D_800C0E22[];
extern unsigned char D_800BEEAC[];
extern unsigned char D_8009DE64[];
extern short D_800A1D9C[];
extern unsigned char D_800A1B90[];
extern unsigned char D_800A1B70[];
extern unsigned char D_800A1D79[];
extern unsigned char *func_8005DB44(int);

int func_8005485C(void)
{
    int unused[2];
    short *cur;
    int i;
    int v;
    int w;
    unsigned char *res;
    int skip;
    int cnt;
    int n;
    register unsigned char *dst asm("$5");
    register int k asm("$4");

    cur = D_800A1D9C;
    i = 0;
    n = D_8009D050;
    if (n > 0) {
        do {
            if (i >= 0 && i < n) {
                v = D_8009D048[i];
                w = v;
                if ((unsigned int)(v - 0x100) < 0x80) {
                    res = (unsigned char *)((v << 5) + (int)D_800BEEAC);
                    goto d;
                }
                if ((unsigned int)(v - 1) < 0xFF) {
                    res = func_8005DB44(v - 1);
                    goto d;
                }
                if ((unsigned int)(w - 0x200) < 9) {
                    res = (unsigned char *)((w << 5) + (int)D_8009DE64);
                    goto d;
                }
            }
            res = 0;
        d:
            if (res != 0 && (res[5] & 0x80) != 0) {
                skip = 0;
                if (D_8009D048 == (short *)D_800C0E48) {
                    if (D_800C0E20[0] == i) {
                        skip = 1;
                    } else if (D_800C0E22[0] == i) {
                        skip = 1;
                    }
                }
                if (skip == 0) {
                    *cur = i;
                    cur++;
                }
            }
            i++;
            n = D_8009D050;
        } while (i < n);
    }
    cnt = cur - D_800A1D9C;
    D_8009D068 = 0;
    D_8009D040 = cnt;
    if (cnt == 0) {
        return 0;
    }
    D_8009D038 = D_8009D038 + 1;
    k = 0;
    if (D_8009D038 >= 0x209) {
        dst = D_800A1B90;
        do {
            *dst = *dst ^ D_800A1D79[k];
            k++;
            dst++;
        } while (k < 0x20);
        k = 0x20;
        dst = D_800A1B90 + 0x20;
        do {
            *dst = *dst ^ D_800A1B70[k];
            k++;
            dst++;
        } while (k < 0x209);
        D_8009D038 = 0;
    }
    return D_8009D048[D_800A1D9C[(D_8009D040 * D_800A1B90[D_8009D038]) >> 8]];
}
