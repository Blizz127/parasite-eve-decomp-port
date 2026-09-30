extern int D_8009D2CC;
extern short D_8009D21E;
extern int D_8009D210;

void func_8008C290(unsigned char *a0) {
    int t;
    int n;
    int val;

    t = *(int *)(a0 + 4);
    n = 1;
    if (t != 0) {
        n = t;
    }
    val = ((*(signed char *)(a0 + 8) << 16) - D_8009D2CC) / n;
    D_8009D21E = n;
    D_8009D210 = val;
}
