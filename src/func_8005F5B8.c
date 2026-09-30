extern int *D_8009D12C;
extern int D_8009D124;
extern int D_8009D128;
extern int D_800A22B0[];
extern int D_800A2270[];
extern unsigned char *func_8005DC4C(int);
extern void func_800527C0(int);
extern void func_8005EED4(int);

void func_8005F5B8(int a0)
{
    unsigned char *s;
    unsigned char c;
    register int *t asm("$3");
    register int *r asm("$5");
    int t0;
    int t1;

    s = func_8005DC4C(a0);
    if (s == 0) {
        return;
    }
    r = D_8009D12C;
    if (r < D_800A22B0) {
        t0 = D_8009D124;
        t1 = D_8009D128;
        t = r + 2;
        D_8009D12C = t;
        r[0] = t0;
        r[1] = t1;
    } else {
        func_800527C0(2);
    }
    c = *s;
    while (c != 0xFF) {
        func_8005EED4(c);
        s++;
        c = *s;
    }
    t = D_8009D12C;
    if (D_800A2270 < t) {
        t0 = t[-2];
        t1 = t[-1];
        D_8009D12C = t - 2;
        D_8009D124 = t0;
        D_8009D128 = t1;
    } else {
        func_800527C0(3);
    }
}
