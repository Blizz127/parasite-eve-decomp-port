extern int D_8009D2B4;
extern short D_8009D2A2;
extern int D_8009D284;

void func_8008B5B8(unsigned char *a0)
{
    int t;
    int d;
    int r;

    t = *(int *)(a0 + 4);
    d = 1;
    if (t != 0) {
        d = t;
    }
    r = ((*(unsigned short *)(a0 + 8) << 16) - D_8009D2B4) / d;
    D_8009D2A2 = d;
    D_8009D284 = r;
}
