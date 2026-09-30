extern int D_8009D03C;
extern int D_8009D050;
extern unsigned short *D_8009D048;
extern unsigned short D_800A1E6E[];
extern unsigned char D_800BEEB0[];

int func_80053E6C(int a0)
{
    int n;
    int count;
    unsigned short *p;
    int u;
    int v;

    count = 0;
    n = D_8009D03C;
    if (a0 >= n && a0 < n + 3) {
        count = *(unsigned short *)((char *)D_800A1E6E + ((a0 - n) << 5));
    } else {
        p = D_8009D048;
        while (p < D_8009D048 + D_8009D050) {
            u = *p;
            if ((unsigned int)(u - 0x100) < 0x80) {
                v = D_800BEEB0[(short)u << 5];
            } else {
                v = (short)u;
            }
            count += ((v ^ a0) == 0);
            p++;
        }
    }
    return count;
}
