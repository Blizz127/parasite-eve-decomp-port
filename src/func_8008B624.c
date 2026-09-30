extern int D_8009D2B4;
extern short D_8009D2A2;
extern int D_8009D284;

void func_8008B624(unsigned char *a0)
{
    int t;
    int d;
    int hi;
    int s;
    int r;

    t = *(int *)(a0 + 4);
    d = 1;
    if (t != 0) {
        d = t;
    }
    hi = *(unsigned short *)(a0 + 0xC) << 16;
    s = *(unsigned short *)(a0 + 8) << 16;
    r = (hi - s) / d;
    D_8009D2A2 = d;
    D_8009D2B4 = s;
    D_8009D284 = r;
}
