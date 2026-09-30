extern int D_8009D2D0;
extern short D_8009D220;
extern int D_8009D214;

void func_8008C18C(unsigned char *a0) {
    int t;
    int n;
    int val;

    t = *(int *)(a0 + 4);
    n = 1;
    if (t != 0) {
        n = t;
    }
    val = ((*(signed char *)(a0 + 8) << 16) - D_8009D2D0) / n;
    D_8009D220 = n;
    D_8009D214 = val;
}
