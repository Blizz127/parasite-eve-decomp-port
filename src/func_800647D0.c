extern void func_80064E90(int *a0);
extern void func_80064D08(void);

void func_800647D0(int *a, int n)
{
    int w;
    int f;
    int oldtop;
    register int vis asm("$5");
    register int r asm("$3");
    register int d asm("$3");
    register int lim asm("$2");
    int *p;
    int *h;

    w = a[13];
    a[22] = (n + w - 1) / w;
    w = a[13];
    f = 0;
    if (w == 2) {
        f = n & 1;
    }
    a[26] = f;
    if (a[18] >= a[22]) {
        a[18] = a[22] - 1;
    }
    if (a[26] != 0 && a[17] == 1 && a[18] == a[22] - 1) {
        a[17] = 0;
    }
    vis = a[27];
    r = a[22];
    oldtop = a[14];
    if (r < vis) {
        vis = r;
        __asm__ __volatile__("" ::: "memory");
        r = a[22];
    }
    lim = a[23];
    d = r - vis;
    a[14] = vis;
    if (d < lim) {
        a[23] = d;
    }
    p = (int *)a[1];
    if (p != 0 && p[8] == 1) {
        p[14] = p[14] + a[16] * (a[14] - oldtop);
    }
    h = (int *)a[32];
    if (h != 0) {
        r = a[22];
        lim = a[27];
        if (!(lim < r)) {
            func_80064E90(h);
        }
    } else {
        r = a[22];
        lim = a[27];
        if (lim < r) {
            func_80064D08();
        }
    }
}
