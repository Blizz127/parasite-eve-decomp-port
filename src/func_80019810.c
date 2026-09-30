extern unsigned char *D_8009D2F0;
extern int func_80077CF4(int a0);
extern int func_80077DC4(int a0);
extern int func_8003708C(int a0, int a1);

int func_80019810(int **a0) {
    register unsigned char *st asm("$2");
    int *p;
    int idx;
    int neg;
    int t;
    int r;

    st = D_8009D2F0;
    asm volatile("" : "=r"(st) : "0"(st));
    p = a0[0];
    asm volatile("" : "=r"(p) : "0"(p));
    idx = *(short *)(st + 0x3A);
    asm volatile("" : "=r"(idx) : "0"(idx));
    neg = -*p;
    t = func_80077CF4(idx);
    r = func_8003708C(neg, t << 4);
    *a0[1] = *(int *)(D_8009D2F0 + 0x28) + r;
    t = func_80077DC4(*(short *)(D_8009D2F0 + 0x3A));
    r = func_8003708C(neg, t << 4);
    *a0[2] = *(int *)(D_8009D2F0 + 0x30) + r;
    return 1;
}
