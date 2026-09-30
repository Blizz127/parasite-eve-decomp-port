extern int *D_8009D154;
extern int *D_8009D158;
extern void func_800527C0(int a0);
extern int *func_8005DA8C(void);
extern int *func_800631DC(void);

int *func_80062D2C(int a0, int a1, int *a2, int a3)
{
    int *cfg;
    int *s;
    int *nx;
    int *pv;
    int *q;
    register int *p asm("$3");
    register int i asm("$4");
    register int *w asm("$2");
    register int x0 asm("$20");
    register int x1 asm("$19");
    register int *x2 asm("$18");
    register int x3 asm("$21");
    register int v asm("$2");
    register int *r asm("$5");
    register int *t asm("$2");
    register int n1 asm("$3");
    register int n2 asm("$4");
    int h;

    x0 = a0;
    x1 = a1;
    x2 = a2;
    x3 = a3;
    cfg = func_8005DA8C();
    if (cfg == 0) {
        func_800527C0(0xC);
    }
    s = D_8009D158;
    if (s == 0) {
        func_800527C0(0xA);
    }
    nx = (int *)s[0];
    pv = D_8009D154;
    D_8009D154 = s;
    s[1] = x1;
    s[11] = 0;
    s[12] = 0;
    D_8009D158 = nx;
    s[0] = (int)pv;
    i = 3;
    q = s + 3;
    do {
        q[2] = 0;
        i--;
        q--;
    } while (i >= 0);
    s[7] = 0;
    s[6] = 0;
    s[9] = 0;
    s[8] = 0;
    s[10] = 0;
    if (x2 != 0) {
        i = 0;
        p = x2;
        __asm__ __volatile__("" : "=r"(p) : "0"(p));
        for (; i < 4; i++) {
            if (p[2] == 0) {
                break;
            }
            p++;
        }
        if (i < 4) {
            w = (int *)(i * 4 + (unsigned int)x2);
            w[2] = (int)s;
        } else {
            func_800527C0(0xB);
        }
    }
    if (s == 0) {
        func_800527C0(0xD);
    }
    v = 1;
    if (x3 == 0) {
        r = func_800631DC();
        if (r == 0) {
            v = 1;
        } else {
            t = D_8009D154;
            n1 = r[0];
            n2 = t[0];
            t[0] = n1;
            r[0] = (int)t;
            D_8009D154 = (int *)n2;
            __asm__ __volatile__("" ::: "memory");
            v = 1;
        }
    }
    s[8] = v;
    s[9] = x0;
    if (cfg[0] != 0) {
        s[6] = cfg[0];
        s[7] = cfg[1];
    } else {
        n1 = cfg[2];
        v = 0xA0;
        n1 = n1 >> 1;
        v = v - n1;
        s[6] = v;
        v = cfg[3];
        n1 = cfg[1];
        n2 = v >> 1;
        v = 0x78;
        if (n1 != 0) {
            v = 0x50;
        }
        v = v - n2;
        s[7] = v;
    }
    s[13] = cfg[2];
    n1 = cfg[3];
    w = s;
    w[15] = 0;
    w[16] = 0;
    w[17] = x3;
    w[18] = 0;
    w[19] = 0;
    w[14] = n1;
    return s;
}
