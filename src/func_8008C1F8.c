extern int D_8009D2D0;
extern short D_8009D220;
extern int D_8009D214;

void func_8008C1F8(unsigned char *a0) {
    int t;
    int n;
    int d;
    int val;

    t = *(int *)(a0 + 8);
    if (t == 0) {
        n = 1;
    } else {
        n = *(int *)(a0 + 4);
    }
    d = (t << 24) >> 8;
    D_8009D2D0 = d;
    val = ((*(signed char *)(a0 + 0xC) << 16) - d) / n;
    D_8009D220 = n;
    D_8009D214 = val;
}
