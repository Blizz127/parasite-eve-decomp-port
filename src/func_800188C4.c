extern unsigned int *D_8009D2F0;
extern int func_8001CAB0(int a0, int a1, int *a2, int a3);

int func_800188C4(int **a0) {
    int buf[8];
    int *d;
    int **s;
    unsigned int i;

    i = 0;
    d = buf;
    s = a0;
    do {
        d[0] = *s[0];
        i++;
        d[1] = *s[1];
        s += 2;
        d += 2;
    } while (i < 4);
    *a0[8] = func_8001CAB0(D_8009D2F0[0xA], D_8009D2F0[0xC], buf, 4);
    return 1;
}
