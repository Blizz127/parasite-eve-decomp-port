extern unsigned int *D_8009D2F0;

int func_80019B78(int **a0) {
    unsigned short *s;
    short *q;
    int t;
    register int t2 asm("$3");

    t2 = **a0;
    s = (unsigned short *)D_8009D2F0;
    t = s[0x1D] + t2;
    s[0x1D] = (unsigned short)t;
    if ((short)t > 0x1000) {
        s[0x1D] = (unsigned short)(t - 0x1000);
    }
    q = (short *)D_8009D2F0;
    if (q[0x1D] < 0) {
        q[0x1D] = (short)(q[0x1D] + 0x1000);
    }
    return 1;
}
